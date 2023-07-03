#ifndef KTARGUMENTPARSER_LIBRARY_H
#define KTARGUMENTPARSER_LIBRARY_H

#include <string>
#include <list>

class KtArgument {
public:
    const bool isOptional;
    const bool takesParameter;
    std::list<char> commandsShort;
    std::list<std::string> commandsLong;
    std::string description;
    bool wasSet = false;
    std::string value;

    KtArgument() = delete;

    explicit KtArgument(bool _isOptional, bool _takesParameter, const std::list<char> &_commandsShort = std::list<char>(), const std::list<std::string> &_commandsLong = std::list<std::string>(), const std::string &_description = "");

    explicit KtArgument(bool _isOptional, bool _takesParameter, const std::list<char> &_commandsShort = std::list<char>(), const std::string &_description = "");

    explicit KtArgument(bool _isOptional, bool _takesParameter, const std::list<std::string> &_commandsLong = std::list<std::string>(), const std::string &_description = "");

    explicit KtArgument(bool _isOptional, bool _takesParameter, const std::string &_description = "");

    void AddShortCommand(char shortCommand);

    void AddShortCommand(const std::list<char> &_commandsShort);

    void AddLongCommand(const std::string &longCommand);

    void AddLongCommand(const std::list<std::string> &_commandsShort);

    void SetDescription(const std::string &_description);
};

class DArgumentParser {

    std::string appName;
    std::string appVersion;
    std::string appDescription;
    std::list<KtArgument> arguments;

    DArgumentParser(int argc, char **argv);

    /**
     * if the argument is valid (at least 1 command, either long or short, is set) then it will be added to the argument list.
     * @return true if argument was added, false if it wasn't (invalid argument)
     * */
    bool AddArgument(const KtArgument &arg);

    /**
      * if all arguments are valid (at least 1 command, either long or short, is set for each argument) then they will be added to the argument list.
      * @return true if the arguments were added, false if they were not. (at least one argument was invalid)
      */
    bool AddArgument(const std::list<KtArgument> &args);
};

#endif //KTARGUMENTPARSER_LIBRARY_H
