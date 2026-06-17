#include <stdio.h>
#include <string.h>
#include <string>

void printHelp()
{
    printf(
        "Molesim [Options]\n"
        "-ligand <file>    Ligand file.\n"
        "-h                Print help.\n"
        "-version          Print version.\n"
    );
}

void printVersion()
{
    printf("Molesim version 0.1\n");
}

int main(int argc, const char **argv)
{
    std::string receiverFileName;
    std::string ligandFileName;
    for(int i = 1; i < argc; ++i)
    {
        auto arg = argv[i];
        if(*arg == '-')
        {
            if(!strcmp(arg, "-help"))
            {
                printHelp();
                return 0;
            }
            else if(!strcmp(arg, "-version"))
            {
                printVersion();
                return 0;
            }
            else if(!strcmp(arg, "-receiver"))
            {
                receiverFileName = argv[++i];
                return 0;
            }
            else if(!strcmp(arg, "-ligand"))
            {
                ligandFileName = argv[++i];
                return 0;
            }
            else
            {
                printHelp();
                return 1;
            }
        }

    }

    if (receiverFileName.empty() && ligandFileName.empty())
    {
        printHelp();
        return 0;
    }

    printf("Hello World\n");
    return 0;
}