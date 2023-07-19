#include "DArgumentParser.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

const char *valueString = "<value> "; //size of 8
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

DArgumentOption::DArgumentOption() : id(generateID()), takesParameter(false) {}

DArgumentOption::DArgumentOption(std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description) : id(generateID()), takesParameter(false), shortCommands(_shortCommands), longCommands(_longCommands), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _takesParameter, std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description) : id(generateID()), takesParameter(_takesParameter), shortCommands(_shortCommands), longCommands(_longCommands), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _takesParameter, std::string _description) : id(generateID()), takesParameter(_takesParameter), description(std::move(_description)) {}

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

bool DArgumentOption::GetTakesParameter() const {
    return takesParameter;
}

void DArgumentOption::SetTakesParameter(bool _takesParameter) {
    takesParameter = _takesParameter;
}

bool DArgumentOption::WasSet() const {
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

void DArgumentParser::calculateSizeOfOptionsString(std::vector<int> &sizes) {
    int index = 0;
    for (auto arg: argumentOptions) {
        sizes[index] = (int) ((2 * arg->shortCommands.size()) + arg->shortCommands.size() + arg->longCommands.size() + (arg->takesParameter * 8));//strlen("<value> ") == 8
        auto iterator = arg->longCommands.begin(), end = arg->longCommands.end();
        while (iterator != end) {
            sizes[index] += (int) (2 + (*iterator).size());
            ++iterator;
        }
        index++;
    }
}

std::vector<std::string> DArgumentParser::generateOptionStrings(std::vector<int> &sizes, int columnSize) {
    std::vector<std::string> optionStrings;
    optionStrings.reserve(argumentOptions.size());
    int index = 0;
    for (auto arg: argumentOptions) {
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
        if (arg->takesParameter)
            tempStr += valueString;
        ostringstream << tempStr;
        if (!arg->description.empty())
            ostringstream << "  " << arg->description;
        ostringstream << '\n';
        optionStrings.emplace_back(std::move(ostringstream.str()));
        index++;
    }
    return std::move(optionStrings);
}

std::string DArgumentParser::generateArgumentOptionsSection() {
    std::string argSection = "\nOptions:\n";
    std::vector<int> sizes(argumentOptions.size());
    calculateSizeOfOptionsString(sizes);
    int optionCommandsColSize = 0;
    for (int i = 0; i < argumentOptions.size(); i++) {
        if (sizes[i] > optionCommandsColSize)
            optionCommandsColSize = sizes[i];
    }
    auto orderedOptionStrings = generateOptionStrings(sizes, optionCommandsColSize);
    std::sort(orderedOptionStrings.begin(), orderedOptionStrings.end());
    size_t totalSize = argSection.size();
    for (const auto &str: orderedOptionStrings)
        totalSize += str.size();
    argSection.reserve(totalSize);
    for (const auto &str: orderedOptionStrings)
        argSection += str;
    return std::move(argSection);
}

void DArgumentParser::resetParsedValues() {
    positionalArgsValues.clear();
    errorText.clear();
    for (auto arg: argumentOptions) {
        arg->wasSet = false;
        arg->value.clear();
    }
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

std::vector<DArgumentOption *> DArgumentParser::getOptionsThatTakeValue() {
    std::vector<DArgumentOption *> args;
    for (auto arg: argumentOptions)
        if (arg->takesParameter)
            args.push_back(arg);
    return std::move(args);
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

bool DArgumentParser::WasSet(char command) {
    for (auto argument: argumentOptions)
        if (argument->shortCommands.find(command) != argument->shortCommands.end())
            return argument->wasSet;
    return false;
}

bool DArgumentParser::WasSet(const std::string &command) {
    for (auto argument: argumentOptions)
        if (argument->longCommands.find(command) != argument->longCommands.end())
            return argument->wasSet;
    return false;
}

std::vector<std::string> DArgumentParser::GetPositionalArguments() const {
    return positionalArgsValues;
}

std::string DArgumentParser::VersionText() {
    std::string versionText;
    versionText.reserve(appName.size() + appVersion.size() + 1);
    versionText.append(appName);
    versionText += ' ';
    versionText.append(appVersion);
    return versionText;
}

std::string DArgumentParser::HelpText() {
    std::string helpText;
    std::string usage = generateUsageSection();
    std::string description = generateDescriptionSection();
    std::string argsHelpText = argumentOptions.empty() ? std::string() : generateArgumentOptionsSection();
    helpText.reserve(usage.size() + description.size() + argsHelpText.size());
    helpText += usage;
    helpText += description;
    helpText += argsHelpText;
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

DParseResult DArgumentParser::parseLongCommand(const std::string &argument, int &currentIndex) {
    size_t posOfEqualSign = argument.find_first_of(equalSign, longCommandStartPos);
    std::string command = argument.substr(longCommandStartPos, posOfEqualSign - longCommandStartPos);
    bool commandFound = false;
    for (auto arg: argumentOptions) {
        auto iter = arg->longCommands.find(command);
        if (iter == arg->longCommands.end())
            continue;
        commandFound = true;
        if (!arg->takesParameter && posOfEqualSign != std::string::npos) {
            generateErrorText(DParseResult::ValuePassedToOptionThatDoesNotTakeValue, command);
            return DParseResult::ValuePassedToOptionThatDoesNotTakeValue;
        }
        if (arg->takesParameter) {
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
            } else
                arg->value = argument.substr(posOfEqualSign + 1);
        }
        arg->wasSet = true;
        break;
    }
    if (!commandFound) {
        generateErrorText(DParseResult::InvalidOption, command);
        return DParseResult::InvalidOption;
    }
    return DParseResult::ParseSuccessful;
}

DParseResult DArgumentParser::parseShortCommand(const std::string &argument, int &currentIndex) {
    return DParseResult::ParseSuccessful;
}
