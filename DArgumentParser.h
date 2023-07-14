#ifndef KTARGUMENTPARSER_LIBRARY_H
#define KTARGUMENTPARSER_LIBRARY_H

#include <string>
#include <vector>
#include <set>
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
    friend class DArgumentParser;

    static std::unordered_set<std::string> ids;

    static std::string generateID();

    bool wasSet = false;
    std::string value;
    std::set<char> shortCommands;
    std::set<std::string> longCommands;
    std::string description;

public:
    const std::string id;
    const bool isOptional;
    const bool takesParameter;

    DArgumentOption() = delete;

    DArgumentOption(bool _isOptional, bool _takesParameter, std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description = std::string());

    DArgumentOption(bool _isOptional, bool _takesParameter, std::string _description = std::string());

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
    bool AddShortCommand(std::set<char> &&_shortCommands);

    [[nodiscard]] const std::set<char> &ShortCommands() const;

    void ClearShortCommands();

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
    bool AddLongCommand(std::set<std::string> &&_longCommands);

    [[nodiscard]] const std::set<std::string> &LongCommands() const;

    void ClearLongCommands();

    void AddDescription(const std::string &_description);

    [[nodiscard]] bool WasSet() const;

    [[nodiscard]] const std::string &GetValue() const;
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
    const std::string executableName;
    std::string appName;
    std::string appVersion;
    std::string appDescription;
    std::unordered_set<DArgumentOption *> argumentOptions;
    std::vector<std::tuple<std::string, std::string, std::string>> positionalArgs;
    std::vector<std::string> positionalArgsValues;
    std::string error;

    static std::string getExecutableName(char *execCall);

    bool checkIfArgumentIsUnique(DArgumentOption *dArgumentOption);

    bool checkIfAllArgumentsInListAreUnique(const std::unordered_set<DArgumentOption *> &_arguments);

    std::string generateUsageSection();

    void calculateSizeOfOptionsString(std::vector<int> &sizes);

    std::vector<std::string> generateOptionStrings(std::vector<int> &sizes, int columnSize);

    std::string generateArgumentOptionsSection();

public:
    DArgumentParser(int argc, char **argv, std::string _appName = std::string(), std::string _appVersion = std::string(), std::string _appDescription = std::string());

    void SetAppInfo(const std::string &name, const std::string &version, const std::string &description = std::string());

    void SetAppName(const std::string &name);

    void SetAppVersion(const std::string &version);

    void SetAppDescription(const std::string &description);

    /**
     * <br>if the argument is valid(1) then it will be added to the argument list.
     * @return true if argument was added, false if it wasn't (invalid argument).
     * @def valid(1) - At least 1 command, either long or short, is set and all of its commands are unique (when compared to other DArgumentOptions added before).
     * @details Do not use in-place constructors as you won't be able to remove it later and clearing all the argumentOptions will result in the memory being leaked.
     */
    bool AddArgumentOption(DArgumentOption *dArgumentOption);

    /**
     * <br>if the argument is valid(1) then it will be added to the argument list.
     * @return true if argument was added, false if it wasn't (invalid argument).
     * @def valid(1) - At least 1 command, either long or short, is set and all of its commands are unique (when compared to other DArgumentOptions added before).
     */
    bool AddArgumentOption(DArgumentOption &dArgumentOption) { return AddArgumentOption(&dArgumentOption); }

    /**
     * <br>if all argumentOptions are valid(1) then they will be added to the argument list.
     * @return true if the argumentOptions were added, false if they were not. (at least one argument was invalid).
     * @def valid(1) - At least 1 command, either long or short, is set for each argument and all commands are unique (when compared to other DArgumentOptions added before and to each other).
     * @details Do not use in-place constructors for DArgumentOption as you won't be able to remove it later and clearing all the argumentOptions will result in the memory being leaked.
     */
    bool AddArgumentOption(std::unordered_set<DArgumentOption *> &&args);

    /**
     * Removes the passed argument from the argument list.
     * @return true if it was removed, false if it wasn't (in case there was no such argument in the list).
     */
    bool RemoveArgumentOption(DArgumentOption *argument);

    /**
     * Removes the passed argument from the argument list.
     * @return true if it was removed, false if it wasn't (in case there was no such argument in the list).
     */
    bool RemoveArgumentOption(DArgumentOption &argument) { RemoveArgumentOption(&argument); }

    /**
     * Removes all argumentOptions previously added, clearing the list.
     */
    void ClearArgumentOptions();

    /**
     * <br>Used to generate the help string.
     * @param name Name of the command.
     * @param description The description to be shown.
     * @param syntax How the command should be used, useful for more complex applications. (Optional, will default to the passed name if not set).
     */
    void AddPositionalArgument(std::string name, std::string description, std::string syntax = std::string());

    /**
     *
     * @param command the command character to check.
     * @return Returns a boolean indicating if the option was set or not, always returns false if no option with specified command was found.
     */
    [[nodiscard]] bool WasSet(char command);

    /**
     *
     * @param command the command string to check.
     * @return Returns a boolean indicating if the option was set or not, always returns false if no option with specified command was found.
     */
    [[nodiscard]] bool WasSet(const std::string &command);

    [[nodiscard]] std::vector<std::string> GetPositionalArguments() const;

    [[nodiscard]] std::string VersionText();

    [[nodiscard]] std::string HelpText();

    [[nodiscard]] std::string ErrorText();

    /**
     * <br>Parses the argv passed on creation based on the positional arguments and option arguments added.
     * @return true if parse was successful, false if an error occurred (non-optional parameter not passed). Call "ErrorText" function to retrieve a printable string of the error.
     */
    bool Parse();
};

#endif //KTARGUMENTPARSER_LIBRARY_H
