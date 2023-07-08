#ifndef KTARGUMENTPARSER_LIBRARY_H
#define KTARGUMENTPARSER_LIBRARY_H

#include <string>
#include <vector>
#include <unordered_set>

class DUnique {
protected:
    int *objCounter;

    void deleteObjectCounter() const;

public:
    DUnique();

    DUnique(const DUnique &dUnique);

    DUnique(DUnique &&dUnique) noexcept;
};

class DArgumentOption : public DUnique {
    static std::unordered_set<std::string> ids;

    static std::string generateID();

    bool wasSet = false;

    std::string value;

public:
    const std::string id;
    const bool isOptional;
    const bool takesParameter;
    std::unordered_set<char> commandsShort;
    std::unordered_set<std::string> commandsLong;
    std::string description;

    DArgumentOption() = delete;

    DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<char> &&_commandsShort, std::unordered_set<std::string> &&_commandsLong, std::string _description = std::string());

    DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<char> &&_commandsShort, std::string _description = std::string());

    DArgumentOption(bool _isOptional, bool _takesParameter, std::unordered_set<std::string> &&_commandsLong, std::string _description = std::string());

    DArgumentOption(bool _isOptional, bool _takesParameter, std::string _description);

    DArgumentOption(bool _isOptional, bool _takesParameter);

    ~DArgumentOption();

    /**
     * Adds the passed character to the command list for this option, unless it was already included.
     * @return true if command was added, false if command was invalid(1) or not added.
     * @def invalid(1) - any character without a ascii representation, spaces or the character minus(-).
     */
    bool AddShortCommand(char shortCommand);

    /**
     * Adds the passed characters to the command list for this option, if any of the characters were already added it won't be added again.
     * @return false if any of the commands were invalid(1), otherwise true.
     * @def invalid(1) - any character without a ascii representation, spaces or the character minus(-).
     */
    bool AddShortCommand(std::unordered_set<char> &&_commandsShort);

    /**
     * Adds the passed string to the command list for this option, unless it was already included.
     * @return true if command was added, false if command was invalid(1) or not added.
     * @def invalid(1) - if string starting with a minus(-) sign.
     */
    bool AddLongCommand(const std::string &longCommand);

    /**
     * Adds the passed strings to the command list for this option, if any of the strings were already added it won't be added again.
     * @return false if any of the commands were invalid(1), otherwise true.
     * @def invalid(1) - if string starting with a minus(-) sign.
     */
    bool AddLongCommand(std::unordered_set<std::string> &&_commandsLong);

    bool WasSet() const;

    std::string GetValue() const;
};

template<>
struct std::hash<DArgumentOption> {
    std::size_t operator()(DArgumentOption const &dArgumentOption) const noexcept {
        return std::hash<std::string>{}(dArgumentOption.id);
    }
};

class DArgumentParser {
    int argumentCount;
    char **argumentValues;
    std::string appName;
    std::string appVersion;
    std::string appDescription;
    std::unordered_set<DArgumentOption *> arguments;
    std::vector<std::string> positionalArgs;
    std::string error;

public:
    DArgumentParser(int argc, char **argv, std::string _appName = std::string(), std::string _appVersion = std::string(), std::string _appDescription = std::string());

    void SetAppName(const std::string &name);

    void SetAppVersion(const std::string &version);

    void SetAppDescription(const std::string &description);

    /**
     * <br>if the argument is valid(1) then it will be added to the argument list.
     * @return true if argument was added, false if it wasn't (invalid argument).
     * @def valid(1) - At least 1 command, either long or short, is set.
     */
    bool AddArgument(DArgumentOption *arg);

    /**
     * <br>if all arguments are valid(1) then they will be added to the argument list.
     * @return true if the arguments were added, false if they were not. (at least one argument was invalid).
     * @def valid(1) - At least 1 command, either long or short, is set for each argument.
     */
    bool AddArgument(std::unordered_set<DArgumentOption *> &&args);

    /**
     * Removes the passed argument from the argument list.
     * @return true if it was removed, false if it wasn't (in case there was no such argument in the list).
     */
    bool RemoveArgument(DArgumentOption *arg);

    /**
     * Removes all arguments previously added, clearing the list.
     */
    void ClearArguments();

    /**
     * <br>Mostly used to generate the help string.
     * @param name Name of the command.
     * @param description The description to be shown.
     * @param syntax How the command should be used, useful for more complex applications. (Optional, will be omitted if not set).
     */
    void AddPositionalArgument(const std::string &name, const std::string &description, const std::string &syntax = std::string());

    /**
     * <br>Parses the argv passed on creation based on the positional arguments and option arguments added.
     * @return true if parse was successful, false if an error occurred (non-optional parameter not passed). Call "ErrorText" function to retrieve a printable string of the error.
     */
    bool Parse();

    bool WasSet(const std::string &command);

    bool WasSet(DArgumentOption *argument);

    [[nodiscard]] std::vector<std::string> GetPositionalArguments() const;

    [[nodiscard]] std::string VersionText();

    [[nodiscard]] std::string HelpText();

    [[nodiscard]] std::string ErrorText();
};

#endif //KTARGUMENTPARSER_LIBRARY_H
