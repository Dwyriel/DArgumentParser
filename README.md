# DArgumentParser
 Parses command line arguments for easier handling.

## Usage

First create the parser and initialize it with both the argc and argv variables, optionally pass all the app information as well so the parser can generate a help and version text for you.
```c++
DArgumentParser parser(argc, argv);
//or
DArgumentParser parser(argc, argv, "MyAppName", "v1.2.5", "Brightens your day by smiling at you every minute.");
//or
DArgumentParser parser(argc, argv);
parser.SetAppInfo("MyAppName", "v1.2.5", "Brightens your day by smiling at you every minute.");
```
If your program has special options, create a DArgumentOption for each of them. Options are passed with either one or two minus signs, for either the shortened or extensive version. The parser will set the minuses itself and won't accept options that start with a minus sign.
```c++
DArgumentOption increaseFrequencyOption;
increaseFrequencyOption.AddShortCommand('f');
increaseFrequencyOption.AddLongCommand("frequency");
increaseFrequencyOption.AddDescription("Increases the smile frequency from 1SPM(Smile Per Minute) to 2SPM.");
//or all in one line
DArgumentOption increaseFrequencyOption({'f'}, {"frequency"}, "Increases the smile frequency from 1SPM(Smile Per Minute) to 2SPM.");

//the command below won't be added
increaseFrequencyOption.AddLongCommand("--freq");
//but commands with a minus sign in the middle are accepted
increaseFrequencyOption.AddLongCommand("smile-freq");
```
Options can also take a parameter, you just need to set the TakesParameter bool to true.<br>
Following the example below, the option will read a parameter if it was passed with a space ```-o outfile.txt``` ```--output outfile.txt``` or by using the equal sign with one of the extensive/long commands ```--output=outfile.txt```.
```c++
DArgumentOption outputFileOption;
outputFileOption.SetTakesParameter(true);
outputFileOption.AddShortCommand('o');
outputFileOption.AddLongCommand("output");
outputFileOption.AddDescription("If set, all the smiles will be writen in this file rather than being printed on the console.");
//or all in one line
DArgumentOption outputFileOption(true, {'o'}, {"output"}, "If set, all the smiles will be writen in this file rather than being printed on the console.");
```
After all option objects are created, they should be added to the parser in order for them to be checked when ```Parse()``` is called later on. They can be passed one by one or passed all at the same time using a list initializer. (needs to be passed as a pointer)
```c++
parser.AddArgumentOption(increaseFrequencyOption);
parser.AddArgumentOption(outputFileOption);
//or
parser.AddArgumentOption({&increaseFrequencyOption, &outputFileOption});
```
It's also possible to specify positional arguments, they will be used for creating the help text and aren't needed for the parsing itself.
```c++
parser.AddPositionalArgument("smile", "the custom smile to use");
//or, in case a custom syntax is desired (for more complex options)
parser.AddPositionalArgument("smile", "the custom smiles to use, in the order they were passed in", "[smiles...]");
```
When everything is set up ```Parse()``` should be called. For every option it finds it will add one to the wasSet attribute of the option, and all positional arguments will be added to the parser's positionalArgsValues attribute.<br>
Because wasSet is an integer, it's possible to know if the user set an option more than once, and it can still be used as a boolean, as 0 resolves to false, and any other value to true.
```c++
parser.Parse();
if (increaseFrequency.WasSet())
    std::cout << "Aren't we needy today? well now.. here's a smile for you :)\n";
std::string outputFileName;
if (outputFileOption.WasSet())
    outputFileName = outputFileOption.GetValue();
for (const auto &posArg: parser.GetPositionalArguments()) {
    //do something
}
``` 
Checking if option was set more than once.
```c++
if (increaseFrequency.WasSet() > 1 || outputFileOption.WasSet() > 1) {
    std::cout << "Error: Option was set more than once.";
    exit(EXIT_FAILURE);
}
```
Parsing can also fail, and will return an enumerator specifying what the problem was. It will also generate a ErrorText that can be printed directly to the console if handling the error isn't desired.
```c++
DParseResult parseResult = parser.Parse();
if (parseResult != DParseResult::ParseSuccessful) {
    std::cout << parser.ErrorText();
    exit(EXIT_FAILURE);
}
```
Using the generated help and version texts together with DArgumentOption to print them to the console when requested.
```c++
DArgumentOption helpOption({'h'}, {"help"}, "Prints out the help text.");
DArgumentOption versionOption({'v'}, {"version"}, "Prints out the version.");
parser.AddArgumentOption({&helpOption, &versionOption});
//...
parser.Parse();
if (helpOption.WasSet()) {
    std::cout << parser.HelpText();
    exit(EXIT_SUCCESS);
}
if (versionOption.WasSet()) {
    std::cout << parser.VersionText();
    exit(EXIT_SUCCESS);
}
```
Documentation specific to each function can be found in the DArgumentParser.h file.