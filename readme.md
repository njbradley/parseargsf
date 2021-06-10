# parseargsf - Make command line args as easy as printf

## General idea
The general idea of parseargsf was to make an argument parser that requires the least amount of code to get something up and running. There are lots of different argument parsing libraries, but most require lots of definitions, manipulating structs, and in general they are not as usable for smaller programs. This is where argparsef comes in. A some example calls to argparse are:

	parseargf(argc, argv, "--verbose|v inputfile:%50s outputfile:%50s", &verbose, infile, outfile);

	parseargf(argc, argv, "--version|V() date:%i/%i/%i", versionfunc, &month, &day, &year);

	parseargf(argc, argv,
		"--help|h() #Prints this message# ..", helpfunc,
		"--verbose|v #Prints extra debug information# ..", &verbose,
		"--num-iters|n:%i #The number of iterations that will be run# ..", &num_iters,
		"input-file:%s #The file read from (if not specified, stdin will be read from)#", filestr
	);

As you can see in the examples, there is very little code needed to get arguments parsed, if your program only needs a couple arguments and flags, then you can do all your parsing in one line. If your arguments are more complex, you can split up the format string and add more inputs and comments, like the last example.

## Format and Specs

The format of the params are described with the printf/scanf format codes, where the values are put into the pointers passed in after the format string. Each parameter follows a similar format:  
 - `[--]param-name[()][:format-str] [#comment#]`  
 - `[--]` A leading -- makes the parameter a flag, meaning it is not a positional argument and it is specified by using --name, with any arguments following.  
 - `param-name` The name used for the parameter in the help message and if it is used as a flag.  
 - `[()]` A () means that instead of a format string, you will pass in a void() function pointer that will be run if the parameter is included. 
 - `[:format-str]` The format string is a string with percent codes used in scanf/printf. The string will be matched to any argument passed in, and the variable arguments will be filled in with the argument.  
 - `[#comment#]` The comment is enclosed in hashtags, and it is displayed in the help message for each flag / parameter.  

## Todo

This library is still very much in development, and there are a couple features that are not in it. I just wanted to test out the idea, and see how useful it would be. If you think this is an idea that you would use, let me know by starring, and suggest any ideas or comments you have! Also, I am always open to pull requests if you want to add something.
