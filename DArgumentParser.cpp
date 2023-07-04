#include "DArgumentParser.h"

#include <utility>

/* ------ DOptionArgument ------ */
DOptionArgument::DOptionArgument(bool _isOptional, bool _takesParameter, const std::unordered_set<char> &_commandsShort, const std::unordered_set<std::string> &_commandsLong, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), commandsShort(_commandsShort), commandsLong(_commandsLong), description(std::move(_description)) {}

DOptionArgument::DOptionArgument(bool _isOptional, bool _takesParameter, const std::list<char> &_commandsShort, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), commandsShort(_commandsShort), description(std::move(_description)) {}

DOptionArgument::DOptionArgument(bool _isOptional, bool _takesParameter, const std::list<std::string> &_commandsLong, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), commandsLong(_commandsLong), description(std::move(_description)) {}

DOptionArgument::DOptionArgument(bool _isOptional, bool _takesParameter, std::string _description) : isOptional(_isOptional), takesParameter(_takesParameter), description(std::move(_description)) {}

bool DOptionArgument::AddShortCommand(char shortCommand) {
    if (shortCommand < 33 || shortCommand == '-' || shortCommand == 127)
        return false;
    return commandsShort.insert(shortCommand).second;
}
