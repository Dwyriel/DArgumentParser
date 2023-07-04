#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef KTARGUMENTPARSER_LIBRARY_H
#define KTARGUMENTPARSER_LIBRARY_H

#include <string>
#include <list>

class DOptionArgument {
public:
    const bool isOptional;
    const bool takesParameter;
    bool wasSet = false;
    std::list<char> commandsShort;
    std::list<std::string> commandsLong;
    std::string description;
    std::string value;

    DOptionArgument() = delete;

    explicit DOptionArgument(bool _isOptional, bool _takesParameter, const std::list<char> &_commandsShort = std::list<char>(), const std::list<std::string> &_commandsLong = std::list<std::string>(), const std::string &_description = "");

    explicit DOptionArgument(bool _isOptional, bool _takesParameter, const std::list<char> &_commandsShort = std::list<char>(), const std::string &_description = "");

    explicit DOptionArgument(bool _isOptional, bool _takesParameter, const std::list<std::string> &_commandsLong = std::list<std::string>(), const std::string &_description = "");

    explicit DOptionArgument(bool _isOptional, bool _takesParameter, const std::string &_description = "");

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
    std::list<DOptionArgument> arguments;
    std::list<std::string> positionalArgs;
    std::string error;

public:
    DArgumentParser(int argc, char **argv);

    void SetAppName(const std::string &name);

    void SetAppVersion(const std::string &name);

    void SetAppDescription(const std::string &name);

    /**
     * <br>Parses the argv passed on creation based on the positional arguments and option arguments added.
     * @return true if parse was successful, false if an error occurred (non-optional parameter not passed). Call "ErrorText" function to retrieve a printable string of the error.
     */
    bool Parse();

    /**
      * <br>if the argument is valid(1) then it will be added to the argument list.
      * @return true if argument was added, false if it wasn't (invalid argument).
      * @def valid(1) - At least 1 command, either long or short, is set.
      */
    bool AddArgument(const DOptionArgument &arg);

    /**
      * <br>if all arguments are valid(1) then they will be added to the argument list.
      * @return true if the arguments were added, false if they were not. (at least one argument was invalid).
      * @def valid(1) - At least 1 command, either long or short, is set for each argument.
      */
    bool AddArgument(const std::list<DOptionArgument> &args);

    /**
     * <br>Mostly used to generate the help string.
     * @param name Name of the command.
     * @param description The description to be shown.
     * @param syntax How the command should be used, useful for more complex applications. (Optional, will be omitted if not set).
     */
    void AddPositionalArgument(const std::string &name, const std::string &description, const std::string &syntax = std::string());

    bool WasSet(const std::string &command);

    bool WasSet(const DOptionArgument &argument);

    [[nodiscard]] std::list<std::string> GetPositionalArguments() const;

    [[nodiscard]] std::string VersionText();

    [[nodiscard]] std::string HelpText();

    [[nodiscard]] std::string ErrorText();
};

#endif //KTARGUMENTPARSER_LIBRARY_H
