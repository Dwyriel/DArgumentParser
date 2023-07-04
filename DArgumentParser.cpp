#include "DArgumentParser.h"

#include <utility>

/* ------ DArgumentOption ------ */
DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, const std::unordered_set<char> &_commandsShort, const std::unordered_set<std::string> &_commandsLong, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), commandsShort(_commandsShort), commandsLong(_commandsLong), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, const std::unordered_set<char> &_commandsShort, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), commandsShort(_commandsShort), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, const std::unordered_set<std::string> &_commandsLong, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), commandsLong(_commandsLong), description(std::move(_description)) {}

DArgumentOption::DArgumentOption(bool _isOptional, bool _takesParameter, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), description(std::move(_description)) {}

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
