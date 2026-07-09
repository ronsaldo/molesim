#include "Simulation.hpp"

namespace Molesim
{
class MolesimVis
{
public:

    void printHelp()
    {
        printf(
            "Molesim [Options] <molecules>\n"
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
        std::vector<std::string> moleculeFileNames;
        SimulationPtr simulation;
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
                else
                {
                    printHelp();
                    return 1;
                }
            }
            else
            {
                moleculeFileNames.push_back(arg);
            }

        }

        if (moleculeFileNames.empty())
        {
            printHelp();
            return 0;
        }

        simulation = std::make_shared<Simulation> ();
        for(auto &fileName : moleculeFileNames)
        {
            auto molecule = loadMolecule(fileName);
            if(!molecule)
                return 1;
            simulation->molecules.push_back(molecule);
        }

        return 0;
    }
};

}

int main(int argc, const char **argv)
{
    Molesim::MolesimVis app;
    return app.main(argc, argv);
}