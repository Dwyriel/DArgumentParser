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
    const char hex[] = "0123456789ABCDEF";
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<char> uniformIntDistribution(0, 15);
    std::string id(32, '0');
    for (int i = 0; i < 32; i++) {
        id[i] = hex[uniformIntDistribution(rng)];
    }
    if (DArgumentOption::ids.find(id) != DArgumentOption::ids.end())
        id = generateID();
    DArgumentOption::ids.insert(id);
    return id;
}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<char> &&_commandsShort, std::unordered_set<std::string> &&_commandsLong, std::string _description) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter), commandsShort(_commandsShort), commandsLong(_commandsLong), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<char> &&_commandsShort, std::string _description) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter), commandsShort(_commandsShort), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<std::string> &&_commandsLong, std::string _description) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter), commandsLong(_commandsLong), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::string _description) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter) : id(generateID()), isOptional(_isOptional), takesParameter(_takesParameter) {}

DArgumentOption::~DArgumentOption() {
    if (--(*objCounter) > 0)
        return;
    DArgumentOption::ids.erase(id);
    deleteObjectCounter();
}

bool DArgumentOption::AddShortCommand(char shortCommand) {
    if (shortCommand < 33 || shortCommand == '-' || shortCommand == 127)
        return false;
    return commandsShort.insert(shortCommand).second;
}

bool DArgumentOption::AddShortCommand(std::unordered_set<char> &&_commandsShort) {
    for (auto shortCommand: _commandsShort)
        if (shortCommand < 33 || shortCommand == '-' || shortCommand == 127)
            return false;
    commandsShort.merge(_commandsShort);
    return true;
}

bool DArgumentOption::AddLongCommand(const std::string &longCommand) {
    if (longCommand.front() == '-')
        return false;
    return commandsLong.insert(longCommand).second;
}

bool DArgumentOption::AddLongCommand(std::unordered_set<std::string> &&_commandsLong) {
    for (auto longCommand: _commandsLong)
        if (longCommand.front() == '-')
            return false;
    commandsLong.merge(_commandsLong);
    return true;
}

bool DArgumentOption::WasSet() const {
    return wasSet;
}

std::string DArgumentOption::GetValue() const {
    return value;
}

DArgumentParser::DArgumentParser(int argc, char **argv, std::string _appName, std::string _appVersion, std::string _appDescription) : argumentCount(argc), argumentValues(argv), appName(std::move(_appName)), appVersion(std::move(_appVersion)), appDescription(std::move(_appDescription)) {}

void DArgumentParser::SetAppName(const std::string &name) {
    appName = name;
}

void DArgumentParser::SetAppVersion(const std::string &version) {
    appVersion = version;
}

void DArgumentParser::SetAppDescription(const std::string &description) {
    appDescription = description;
}
