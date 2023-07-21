#include "DArgumentParser.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

const char *argOptionTakesValueString = "<value> "; //size of 8
const char *helpAndVersionOptionsSectionOpeningString = "\nGetting help:\n";
const char *normalOptionSectionOpeningString = "\nOptions:\n";
const char minusSign = '-', equalSign = '=';
const int shortCommandStartPos = 1, longCommandStartPos = 2;

/* ------ DUnique ------ */
DUnique::DUnique() : objCounter(new int(1)) {}

DUnique::DUnique(const DUnique &dUnique) {
    objCounter = dUnique.objCounter;
    (*objCounter)++;
}

DUnique::DUnique(DUnique &&dUnique) noexcept {
    objCounter = dUnique.objCounter;
}

void DUnique::deleteObjectCounter() const {
    delete objCounter;
}

/* ------ DArgumentOption ------ */
std::unordered_set<std::string> DArgumentOption::ids;

std::string DArgumentOption::generateID() {
    const int strSize = 32;
    const char digits[] = "0123456789ABCDEF";
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<char> uniformIntDistribution(0, sizeof(digits) - 2);
    std::string id(strSize, '0');
    for (int i = 0; i < strSize; i++)
        id[i] = digits[uniformIntDistribution(rng)];
    if (DArgumentOption::ids.find(id) != DArgumentOption::ids.end())
        id = generateID();
    DArgumentOption::ids.insert(id);
    return id;
}

DArgumentOption::DArgumentOption() : id(generateID()), type(DArgumentOptionType::NormalOption) {}

DArgumentOption::DArgumentOption(std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description) : id(generateID()), type(DArgumentOptionType::NormalOption), shortCommands(_shortCommands), longCommands(_longCommands), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(DArgumentOptionType _type, std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description) : id(generateID()), type(_type), shortCommands(_shortCommands), longCommands(_longCommands), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(DArgumentOptionType _type, std::string _description) : id(generateID()), type(_type), description(std::move(_description)) {}

DArgumentOption::~DArgumentOption() {
    if (--(*objCounter) > 0)
        return;
    DArgumentOption::ids.erase(id);
    deleteObjectCounter();
}

bool DArgumentOption::AddShortCommand(char shortCommand) {
    if (shortCommand < 33 || shortCommand == minusSign || shortCommand == 127)
        return false;
    return shortCommands.insert(shortCommand).second;
}

bool DArgumentOption::AddShortCommand(std::set<char> &&_shortCommands) {
    for (auto shortCommand: _shortCommands)
        if (shortCommand < 33 || shortCommand == minusSign || shortCommand == 127)
            return false;
    shortCommands.merge(_shortCommands);
    return true;
}

const std::set<char> &DArgumentOption::ShortCommands() const {
    return shortCommands;
}

void DArgumentOption::ClearShortCommands() {
    shortCommands.clear();
}

bool DArgumentOption::AddLongCommand(const std::string &longCommand) {
    if (longCommand.front() == minusSign || longCommand.find(equalSign) != std::string::npos)
        return false;
    return longCommands.insert(longCommand).second;
}

bool DArgumentOption::AddLongCommand(std::set<std::string> &&_longCommands) {
    for (auto longCommand: _longCommands)
        if (longCommand.front() == minusSign || longCommand.find(equalSign) != std::string::npos)
            return false;
    longCommands.merge(_longCommands);
    return true;
}

const std::set<std::string> &DArgumentOption::LongCommands() const {
    return longCommands;
}

void DArgumentOption::ClearLongCommands() {
    longCommands.clear();
}

void DArgumentOption::AddDescription(const std::string &_description) {
    description = _description;
}

DArgumentOptionType DArgumentOption::GetType() const {
    return type;
}

void DArgumentOption::SetType(DArgumentOptionType _type) {
    type = _type;
}

int DArgumentOption::WasSet() const {
    return wasSet;
}

const std::string &DArgumentOption::GetValue() const {
    return value;
}

DArgumentParser::DArgumentParser(int argc, char **argv, std::string _appName, std::string _appVersion, std::string _appDescription) : argumentCount(argc), argumentValues(argv), executableName(getExecutableName(argv[0])), appName(std::move(_appName)), appVersion(std::move(_appVersion)), appDescription(std::move(_appDescription)) {}

std::string DArgumentParser::getExecutableName(char *execCall) {
    std::string execName(execCall);
    return execName.substr(execName.find_last_of('/') + 1);
}

std::vector<int> DArgumentParser::calculateSizeOfOptionStrings(const std::vector<DArgumentOption *> &args) {
    std::vector<int> sizes(args.size());
    int index = 0;
    for (auto arg: args) {
        /** formula explanation:
         * 2 * arg->shortCommands.size() -> adding the minus sign to a characters always leads to 2 characters, so we just need to multiply the amount of chars by 2
         * arg->shortCommands.size() + arg->longCommands.size() -> the amount of spaces needed between commands, not reducing by 1 means we'll have one extra space total, but we can account for that when building the string and avoid branches.
         * (arg->type == DArgumentOptionType::InputOption) * 8) -> if arg.type takes a value, we'll print "<value> " (8 characters) after it, because false/true resolves to 0/1, we can safely multiply by this comparison to avoid branching.
         */
        sizes[index] = (int) ((2 * arg->shortCommands.size()) + arg->shortCommands.size() + arg->longCommands.size() + ((arg->type == DArgumentOptionType::InputOption) * 8));//strlen("<value> ") == 8
        auto iterator = arg->longCommands.begin(), end = arg->longCommands.end();
        while (iterator != end) {
            /** formula explanation:
             * 2 + (*iterator).size() -> size of string plus two minus signs characters
             */
            sizes[index] += (int) (2 + (*iterator).size());
            ++iterator;
        }
        index++;
    }
    return std::move(sizes);
}

std::vector<std::string> DArgumentParser::generateOptionStrings(const std::vector<DArgumentOption *> &args, std::vector<int> &sizes, int columnSize) {
    std::vector<std::string> optionStrings;
    optionStrings.reserve(args.size());
    int index = 0;
    for (auto arg: args) {
        std::ostringstream ostringstream;
        ostringstream << "   " << std::setw(columnSize) << std::left;
        std::string tempStr;
        tempStr.reserve(sizes[index]);
        for (auto c: arg->shortCommands) {
            tempStr += minusSign;
            tempStr += c;
            tempStr += ' ';
        }
        for (const auto &str: arg->longCommands) {
            tempStr += "--";
            tempStr += str;
            tempStr += ' ';
        }
        if (arg->type == DArgumentOptionType::InputOption)
            tempStr += argOptionTakesValueString;
        ostringstream << tempStr;
        if (!arg->description.empty())
            ostringstream << "  " << arg->description;
        ostringstream << '\n';
        optionStrings.emplace_back(std::move(ostringstream.str()));
        index++;
    }
    return std::move(optionStrings);
}

std::string DArgumentParser::generateOptionsSubSection(const std::vector<DArgumentOption *> &args, const char *openingString) {
    std::string sectionString = openingString;
    std::vector<int> otherSizes = calculateSizeOfOptionStrings(args);
    int optionCommandsColSize = 0;
    for (int i = 0; i < args.size(); i++) {
        if (otherSizes[i] > optionCommandsColSize)
            optionCommandsColSize = otherSizes[i];
    }
    auto orderedOptionStrings = generateOptionStrings(args, otherSizes, optionCommandsColSize);
    std::sort(orderedOptionStrings.begin(), orderedOptionStrings.end());
    size_t totalSize = sectionString.size();
    for (const auto &str: orderedOptionStrings)
        totalSize += str.size();
    sectionString.reserve(totalSize);
    for (const auto &str: orderedOptionStrings)
        sectionString += str;
    return std::move(sectionString);
}

bool DArgumentParser::isLongCommand(const std::string &argument) {
    return (argument.size() > longCommandStartPos && argument[0] == minusSign && argument[1] == minusSign);
}

bool DArgumentParser::isShortCommand(const std::string &argument) {
    return (argument.size() > shortCommandStartPos && argument[0] == minusSign && argument[1] != minusSign);
}

bool DArgumentParser::checkIfArgumentIsUnique(DArgumentOption *dArgumentOption) {
    if (argumentOptions.find(dArgumentOption) != argumentOptions.end())
        return false;
    if (dArgumentOption->shortCommands.empty() && dArgumentOption->longCommands.empty())
        return false;
    for (auto argument: argumentOptions) {
        auto shortEnd = argument->shortCommands.end();
        for (auto shortCommand: dArgumentOption->shortCommands)
            if (argument->shortCommands.find(shortCommand) != shortEnd)
                return false;
        auto longEnd = argument->longCommands.end();
        for (auto &longCommand: dArgumentOption->longCommands)
            if (argument->longCommands.find(longCommand) != longEnd)
                return false;
    }
    return true;
}

bool DArgumentParser::checkIfAllArgumentsInListAreUnique(const std::unordered_set<DArgumentOption *> &_arguments) {
    for (auto upperIterator = _arguments.begin(), upperEnd = _arguments.end(); upperIterator != upperEnd; ++upperIterator) {
        if (!checkIfArgumentIsUnique(*upperIterator))
            return false;
        auto lowerIterator = upperIterator;
        if (++lowerIterator == upperEnd)
            break;
        for (; lowerIterator != upperEnd; ++lowerIterator) {
            auto shortEnd = (*lowerIterator)->shortCommands.end();
            for (auto shortCommand: (*upperIterator)->shortCommands)
                if ((*lowerIterator)->shortCommands.find(shortCommand) != shortEnd)
                    return false;
            auto longEnd = (*lowerIterator)->longCommands.end();
            for (auto &longCommand: (*upperIterator)->longCommands)
                if ((*lowerIterator)->longCommands.find(longCommand) != longEnd)
                    return false;
        }
    }
    return true;
}

std::string DArgumentParser::generateUsageSection() {
    std::string usageString = "Usage: ";
    std::string optionString = argumentOptions.empty() ? std::string() : std::string(" [options]");
    std::string posArgString;
    if (!positionalArgs.empty()) {
        std::string::size_type totalSize = 0;
        for (const auto &posArg: positionalArgs)
            totalSize += std::get<2>(posArg).empty() ? std::get<0>(posArg).size() + 3 : std::get<2>(posArg).size() + 1;
        posArgString.reserve(totalSize);
        for (const auto &posArg: positionalArgs) {
            if (std::get<2>(posArg).empty()) {
                posArgString += " [";
                posArgString += std::get<0>(posArg);
                posArgString += ']';
                continue;
            }
            posArgString += ' ';
            posArgString += std::get<2>(posArg);
        }
    }
    usageString.reserve(usageString.size() + executableName.size() + optionString.size() + posArgString.size() + 1);
    usageString += executableName;
    usageString += optionString;
    usageString += posArgString;
    usageString += '\n';
    return usageString;
}

std::string DArgumentParser::generateDescriptionSection() {
    if (appDescription.empty())
        return {};
    std::string descriptionString;
    descriptionString.reserve(2 + appDescription.size());
    descriptionString += '\n';
    descriptionString += appDescription;
    descriptionString += '\n';
    return descriptionString;
}

void DArgumentParser::calculateSizeOfArgumentsString(std::vector<int> &sizes) {
    int index = 0;
    for (auto arg: positionalArgs)
        /** formula explanation:
         * std::get<0>(arg).size() + 2 -> size of string plus size of 2 characters
         */
        sizes[index++] = (int) (std::get<2>(arg).empty() ? (std::get<0>(arg).size() + 2) : std::get<2>(arg).size());
}

std::string DArgumentParser::generatePositionalArgsSection() {
    std::string argSection = "\nArguments:\n";
    std::vector<int> sizes(positionalArgs.size());
    calculateSizeOfArgumentsString(sizes);
    int optionArgsColSize = 0;
    for (int i = 0; i < positionalArgs.size(); i++) {
        if (sizes[i] > optionArgsColSize)
            optionArgsColSize = sizes[i];
    }
    for (auto arg: positionalArgs) {
        std::ostringstream ostringstream;
        ostringstream << "   " << std::setw(optionArgsColSize) << std::left;
        bool customSyntax = !std::get<2>(arg).empty();
        if (customSyntax)
            ostringstream << std::get<2>(arg);
        else {
            std::string tempStr;
            tempStr.reserve(2 + std::get<0>(arg).size());
            tempStr += '[';
            tempStr += std::get<0>(arg);
            tempStr += ']';
            ostringstream << tempStr;
        }
        ostringstream << "   " << std::get<1>(arg) << '\n';
        argSection += ostringstream.str();
    }
    return std::move(argSection);
}

std::string DArgumentParser::generateOptionsSection() {
    std::vector<DArgumentOption *> helpAndVersionOptions, normalOptions;
    for (auto arg: argumentOptions)
        if (arg->type == DArgumentOptionType::HelpOption || arg->type == DArgumentOptionType::VersionOption)
            helpAndVersionOptions.push_back(arg);
        else
            normalOptions.push_back(arg);
    std::string sectionString;
    if (!helpAndVersionOptions.empty())
        sectionString += generateOptionsSubSection(helpAndVersionOptions, helpAndVersionOptionsSectionOpeningString);
    if (!normalOptions.empty())
        sectionString += generateOptionsSubSection(normalOptions, normalOptionSectionOpeningString);
    return std::move(sectionString);
}

void DArgumentParser::generateErrorText(DParseResult error, const std::string &command) {
    switch (error) {
        case DParseResult::InvalidOption:
            errorText = "Option --" + command + " is invalid";
            break;
        case DParseResult::ValuePassedToOptionThatDoesNotTakeValue:
            errorText = "Option --" + command + " received a value but it doesn't take any";
            break;
        case DParseResult::NoValueWasPassedToOption:
            errorText = "Option --" + command + " takes a value but none was passed.";
            break;
        default:
            return;
    }
}

void DArgumentParser::generateErrorText(DParseResult error, char command) {
    std::string str;
    str = command;
    switch (error) {
        case DParseResult::InvalidOption:
            errorText = "Option -" + str + " is invalid";
            break;
        case DParseResult::ValuePassedToOptionThatDoesNotTakeValue:
            errorText = "Option -" + str + " received a value but it doesn't take any";
            break;
        case DParseResult::NoValueWasPassedToOption:
            errorText = "Option -" + str + " takes a value but none was passed.";
            break;
        case DParseResult::OptionsThatTakesValueNeedsToBeSetSeparately:
            errorText = "Options that takes a value needs to be set separately. Error with option: -" + str;
            break;
        default:
            return;
    }
}

void DArgumentParser::resetParsedValues() {
    positionalArgsValues.clear();
    errorText.clear();
    for (auto arg: argumentOptions) {
        arg->wasSet = 0;
        arg->value.clear();
    }
}

DParseResult DArgumentParser::parseLongCommand(const std::string &argument, int &currentIndex) {
    size_t posOfEqualSign = argument.find_first_of(equalSign, longCommandStartPos);
    std::string command = argument.substr(longCommandStartPos, posOfEqualSign - longCommandStartPos);
    bool commandFound = false;
    for (auto arg: argumentOptions) {
        auto iterator = arg->longCommands.find(command);
        if (iterator == arg->longCommands.end())
            continue;
        commandFound = true;
        if (arg->type != DArgumentOptionType::InputOption && posOfEqualSign != std::string::npos) {
            generateErrorText(DParseResult::ValuePassedToOptionThatDoesNotTakeValue, command);
            return DParseResult::ValuePassedToOptionThatDoesNotTakeValue;
        }
        if (arg->type == DArgumentOptionType::InputOption) {
            if (posOfEqualSign == std::string::npos) {
                if (++currentIndex == argumentCount) {
                    generateErrorText(DParseResult::NoValueWasPassedToOption, command);
                    return DParseResult::NoValueWasPassedToOption;
                }
                arg->value = std::string(argumentValues[currentIndex]);
                if (isLongCommand(arg->value) || isShortCommand(arg->value)) {
                    generateErrorText(DParseResult::NoValueWasPassedToOption, command);
                    return DParseResult::NoValueWasPassedToOption;
                }
            } else {
                if (argument[posOfEqualSign] == argument[argument.size() - 1]) {
                    generateErrorText(DParseResult::NoValueWasPassedToOption, command);
                    return DParseResult::NoValueWasPassedToOption;
                }
                arg->value = argument.substr(posOfEqualSign + 1);
            }
        }
        arg->wasSet++;
        break;
    }
    if (!commandFound) {
        generateErrorText(DParseResult::InvalidOption, command);
        return DParseResult::InvalidOption;
    }
    return DParseResult::ParseSuccessful;
}

DParseResult DArgumentParser::parseShortCommand(const std::string &argument, int &currentIndex) {
    for (int i = 1; i < argument.size(); i++) {
        bool commandFound = false;
        for (auto arg: argumentOptions) {
            auto iterator = arg->shortCommands.find(argument[i]);
            if (iterator == arg->shortCommands.end())
                continue;
            commandFound = true;
            if (arg->type == DArgumentOptionType::InputOption && argument.size() > 2) {
                generateErrorText(DParseResult::OptionsThatTakesValueNeedsToBeSetSeparately, argument[i]);
                return DParseResult::OptionsThatTakesValueNeedsToBeSetSeparately;
            }
            if (arg->type == DArgumentOptionType::InputOption) {
                if (++currentIndex == argumentCount) {
                    generateErrorText(DParseResult::NoValueWasPassedToOption, argument[i]);
                    return DParseResult::NoValueWasPassedToOption;
                }
                arg->value = std::string(argumentValues[currentIndex]);
                if (isLongCommand(arg->value) || isShortCommand(arg->value)) {
                    generateErrorText(DParseResult::NoValueWasPassedToOption, argument[i]);
                    return DParseResult::NoValueWasPassedToOption;
                }
            }
            arg->wasSet++;
            break;
        }
        if (!commandFound) {
            generateErrorText(DParseResult::InvalidOption, argument[i]);
            return DParseResult::InvalidOption;
        }
    }
    return DParseResult::ParseSuccessful;
}

void DArgumentParser::SetAppInfo(const std::string &name, const std::string &version, const std::string &description) {
    appName = name;
    appVersion = version;
    appDescription = description;
}

void DArgumentParser::SetAppName(const std::string &name) {
    appName = name;
}

void DArgumentParser::SetAppVersion(const std::string &version) {
    appVersion = version;
}

void DArgumentParser::SetAppDescription(const std::string &description) {
    appDescription = description;
}

bool DArgumentParser::AddArgumentOption(DArgumentOption *dArgumentOption) {
    if (!checkIfArgumentIsUnique(dArgumentOption))
        return false;
    return argumentOptions.insert(dArgumentOption).second;
}

bool DArgumentParser::AddArgumentOption(std::unordered_set<DArgumentOption *> &&args) {
    if (!checkIfAllArgumentsInListAreUnique(args))
        return false;
    argumentOptions.merge(args);
    return true;
}

bool DArgumentParser::RemoveArgumentOption(DArgumentOption *argument) {
    return argumentOptions.erase(argument);
}

void DArgumentParser::ClearArgumentOptions() {
    argumentOptions.clear();
}

void DArgumentParser::AddPositionalArgument(std::string name, std::string description, std::string syntax) {
    positionalArgs.emplace_back(std::move(name), std::move(description), std::move(syntax));
}

void DArgumentParser::ClearPositionalArgumets() {
    positionalArgs.clear();
}

int DArgumentParser::WasSet(char command) {
    for (auto argument: argumentOptions)
        if (argument->shortCommands.find(command) != argument->shortCommands.end())
            return argument->wasSet;
    return 0;
}

int DArgumentParser::WasSet(const std::string &command) {
    for (auto argument: argumentOptions)
        if (argument->longCommands.find(command) != argument->longCommands.end())
            return argument->wasSet;
    return 0;
}

const std::vector<std::string> &DArgumentParser::GetPositionalArguments() const {
    return positionalArgsValues;
}

std::string DArgumentParser::VersionText() {
    std::string versionText;
    versionText.reserve(appName.size() + appVersion.size() + 2);
    versionText.append(appName);
    versionText += ' ';
    versionText.append(appVersion);
    versionText += '\n';
    return versionText;
}

std::string DArgumentParser::HelpText() {
    std::string helpText;
    std::string usageSection = generateUsageSection();
    std::string descriptionSection = generateDescriptionSection();
    std::string posArgsSection = generatePositionalArgsSection();
    std::string optionsSection = generateOptionsSection();
    helpText.reserve(usageSection.size() + descriptionSection.size() + posArgsSection.size() + optionsSection.size());
    helpText += usageSection;
    helpText += descriptionSection;
    helpText += posArgsSection;
    helpText += optionsSection;
    return helpText;
}

std::string DArgumentParser::ErrorText() const {
    return errorText;
}

DParseResult DArgumentParser::Parse() {
    resetParsedValues();
    if (argumentCount < 2)
        return DParseResult::ParseSuccessful;
    std::vector<std::string> arguments;
    DParseResult parseResult;
    for (int index = 1; index < argumentCount; index++) {
        std::string currArg(argumentValues[index]);
        if (isLongCommand(currArg)) {
            parseResult = parseLongCommand(currArg, index);
            if (parseResult != DParseResult::ParseSuccessful)
                return parseResult;
            continue;
        }
        if (isShortCommand(currArg)) {
            parseResult = parseShortCommand(currArg, index);
            if (parseResult != DParseResult::ParseSuccessful)
                return parseResult;
            continue;
        }
        positionalArgsValues.push_back(currArg);
    }
    //auto optionsThatTakeValue = getOptionsThatTakeValue();

    return DParseResult::ParseSuccessful;
}
