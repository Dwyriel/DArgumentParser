#ifndef DARGUMENTPARSER_LIBRARY_H
#define DARGUMENTPARSER_LIBRARY_H

#include <string>
#include <vector>
#include <set>
#include <unordered_set>

enum class DParseResult : unsigned char {
    ParseSuccessful,
    InvalidOption,
    ValuePassedToOptionThatDoesNotTakeValue,
    NoValueWasPassedToOption,
    OptionsThatTakesValueNeedsToBeSetSeparately
};

enum class DArgumentOptionType : unsigned char {
    NormalOption,
    InputOption,
    HelpOption,
    VersionOption
};

class DArgumentOption {
    friend class DArgumentParser;

    DArgumentOptionType type = DArgumentOptionType::NormalOption;
    int wasSet = 0;
    std::string value;
    std::set<char> shortCommands;
    std::set<std::string> longCommands;
    std::string description;

public:

    /**
     * @details instantiate a DArgumentOption with default values (type = DArgumentOptionType::NormalOption)
     */
    explicit DArgumentOption();

    DArgumentOption(std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description = std::string());

    DArgumentOption(DArgumentOptionType _type, std::set<char> &&_shortCommands, std::set<std::string> &&_longCommands, std::string _description = std::string());

    DArgumentOption(DArgumentOptionType _type, std::string _description);

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
     * @def invalid(1) - if string starts with a minus(-) sign or has a equal(=) sign.
     */
    bool AddLongCommand(const std::string &longCommand);

    /**
     * Adds the passed strings to the command list for this option, if any of the strings were already added it won't be added again.
     * @return false if any of the commands were invalid(1), otherwise true.
     * @def invalid(1) - if string starts with a minus(-) sign or has a equal(=) sign.
     */
    bool AddLongCommand(std::set<std::string> &&_longCommands);

    [[nodiscard]] const std::set<std::string> &LongCommands() const;

    void ClearLongCommands();

    void AddDescription(const std::string &_description);

    void SetType(DArgumentOptionType _type);

    [[nodiscard]] DArgumentOptionType GetType() const;

    [[nodiscard]] int WasSet() const;

    [[nodiscard]] const std::string &GetValue() const;
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
    std::string errorText;

    static std::string getExecutableName(char *execCall);

    static std::vector<int> calculateSizeOfOptionStrings(const std::vector<DArgumentOption *> &args);

    static std::vector<std::string> generateOptionStrings(const std::vector<DArgumentOption *> &args, const std::vector<int> &sizes, int columnSize);

    static std::string generateOptionsSubSection(const std::vector<DArgumentOption *> &args, const char *openingString);

    static bool isLongCommand(const std::string &argument);

    static bool isShortCommand(const std::string &argument);

    bool checkIfArgumentIsUnique(DArgumentOption *dArgumentOption);

    bool checkIfAllArgumentsInListAreUnique(const std::unordered_set<DArgumentOption *> &_arguments);

    std::string generateUsageSection();

    std::string generateDescriptionSection();

    void calculateSizeOfArgumentsString(std::vector<int> &sizes);

    std::string generatePositionalArgsSection();

    std::string generateOptionsSection();

    void generateErrorText(DParseResult error, const std::string &command);

    void generateErrorText(DParseResult error, char command);

    void resetParsedValues();

    DParseResult parseLongCommand(const std::string &argument, int &currentIndex);

    DParseResult parseShortCommand(const std::string &argument, int &currentIndex);

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
    bool RemoveArgumentOption(DArgumentOption &argument) { return RemoveArgumentOption(&argument); }

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
     * Removes all positional arguments previously added, clearing the list.
     */
    void ClearPositionalArgumets();

    /**
     * @param command the command character to check.
     * @return Returns a boolean indicating if the option was set or not, always returns false if no option with specified command was found.
     */
    [[nodiscard]] int WasSet(char command);

    /**
     * @param command the command string to check.
     * @return Returns a boolean indicating if the option was set or not, always returns false if no option with specified command was found.
     */
    [[nodiscard]] int WasSet(const std::string &command);

    /**
     * Retrieves the value of every positional argument that was set during the parsing.
     * @return Returns a const reference to the positionalArgsValues list.
     */
    [[nodiscard]] const std::vector<std::string> &GetPositionalArguments() const;

    [[nodiscard]] std::string VersionText();

    [[nodiscard]] std::string HelpText();

    [[nodiscard]] std::string ErrorText() const;

    /**
     * <br>Parses the argv passed on creation based on the positional arguments and option arguments added.
     * @return true if parse was successful, false if an error occurred (non-optional parameter not passed). Call "ErrorText" function to retrieve a printable string of the error.
     */
    DParseResult Parse();
};

#endif //DARGUMENTPARSER_LIBRARY_H
