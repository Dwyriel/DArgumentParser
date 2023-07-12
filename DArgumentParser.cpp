#include "DArgumentParser.h"

#include <random>
#include <iostream>

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

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<char> &&_shortCommands, std::unordered_set<std::string> &&_longCommands, std::string _description) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter), shortCommands(_shortCommands), longCommands(_longCommands), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::string _description) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter), description(std::move(_description)) {}

DArgumentOption::~DArgumentOption() {
    if (--(*objCounter) > 0)
        return;
    DArgumentOption::ids.erase(id);
    deleteObjectCounter();
}

bool DArgumentOption::AddShortCommand(char shortCommand) {
    if (shortCommand < 33 || shortCommand == '-' || shortCommand == 127)
        return false;
    return shortCommands.insert(shortCommand).second;
}

bool DArgumentOption::AddShortCommand(std::unordered_set<char> &&_shortCommands) {
    for (auto shortCommand: _shortCommands)
        if (shortCommand < 33 || shortCommand == '-' || shortCommand == 127)
            return false;
    shortCommands.merge(_shortCommands);
    return true;
}

const std::unordered_set<char> &DArgumentOption::ShortCommands() const {
    return shortCommands;
}

void DArgumentOption::ClearShortCommands() {
    shortCommands.clear();
}

bool DArgumentOption::AddLongCommand(const std::string &longCommand) {
    if (longCommand.front() == '-')
        return false;
    return longCommands.insert(longCommand).second;
}

bool DArgumentOption::AddLongCommand(std::unordered_set<std::string> &&_longCommands) {
    for (auto longCommand: _longCommands)
        if (longCommand.front() == '-')
            return false;
    longCommands.merge(_longCommands);
    return true;
}

const std::unordered_set<std::string> &DArgumentOption::LongCommands() const {
    return longCommands;
}

void DArgumentOption::ClearLongCommands() {
    longCommands.clear();
}

void DArgumentOption::AddDescription(const std::string &_description) {
    description = _description;
}

bool DArgumentOption::WasSet() const {
    return wasSet;
}

const std::string &DArgumentOption::GetValue() const {
    return value;
}

DArgumentParser::DArgumentParser(int argc, char **argv, std::string _appName, std::string _appVersion, std::string _appDescription) : argumentCount(argc), argumentValues(argv), appName(std::move(_appName)), appVersion(std::move(_appVersion)), appDescription(std::move(_appDescription)) {}

bool DArgumentParser::checkIfArgumentIsUnique(DArgumentOption *dArgumentOption) {
    if (arguments.find(dArgumentOption) != arguments.end())
        return false;
    if (dArgumentOption->shortCommands.empty() && dArgumentOption->longCommands.empty())
        return false;
    for (auto argument: arguments) {
        for (auto shortCommand: dArgumentOption->shortCommands)
            if (argument->shortCommands.find(shortCommand) != argument->shortCommands.end())
                return false;
        for (auto &longCommand: dArgumentOption->longCommands)
            if (argument->longCommands.find(longCommand) != argument->longCommands.end())
                return false;
    }
    return true;
}

bool DArgumentParser::checkIfAllArgumentsInListAreUnique(const std::unordered_set<DArgumentOption *> &_arguments) {
    int index = 1;
    for (auto upperIterator = _arguments.begin(); upperIterator != _arguments.end(); upperIterator++) {
        if (!checkIfArgumentIsUnique(*upperIterator))
            return false;
        auto lowerIterator = _arguments.begin();
        for (int i = 0; i < index; i++)
            lowerIterator++;
        if (lowerIterator == _arguments.end())
            continue;
        for (; lowerIterator != _arguments.end(); lowerIterator++) {
            for (auto shortCommand: (*upperIterator)->shortCommands)
                if ((*lowerIterator)->shortCommands.find(shortCommand) != (*lowerIterator)->shortCommands.end())
                    return false;
            for (auto &longCommand: (*upperIterator)->longCommands)
                if ((*lowerIterator)->longCommands.find(longCommand) != (*lowerIterator)->longCommands.end())
                    return false;
        }
        index++;
    }
    return true;
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

bool DArgumentParser::AddArgument(DArgumentOption *dArgumentOption) {
    if (!checkIfArgumentIsUnique(dArgumentOption))
        return false;
    return arguments.insert(dArgumentOption).second;
}

bool DArgumentParser::AddArgument(DArgumentOption &dArgumentOption) {
    return AddArgument(&dArgumentOption);
}

bool DArgumentParser::AddArgument(std::unordered_set<DArgumentOption *> &&args) {
    if (!checkIfAllArgumentsInListAreUnique(args))
        return false;
    arguments.merge(args);
    return true;
}

bool DArgumentParser::RemoveArgument(DArgumentOption *argument) {
    return arguments.erase(argument);
}

bool DArgumentParser::RemoveArgument(DArgumentOption &argument) {
    return arguments.erase(&argument);
}

void DArgumentParser::ClearArguments() {
    arguments.clear();
}

void DArgumentParser::AddPositionalArgument(std::string name, std::string description, std::string syntax) {
    positionalArgs.emplace_back(std::move(name), std::move(description), std::move(syntax));
}
