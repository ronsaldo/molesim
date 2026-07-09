#include "Simulation.hpp"
#include "SDL.h"
#include "SDL_syswm.h"

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
        
        SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow("MolesimVis", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

        while(!quitting)
        {
            processEvents();
        }

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
    }

    void processEvents()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quitting = true;
                    break;
                }
            case SDL_QUIT:
                quitting = true;
                break;
            }
        }
    }

    SDL_Window *window;
    bool quitting = false;
    int screenWidth = 60*16;
    int screenHeight = 60*9;
};

}

int main(int argc, const char **argv)
{
    Molesim::MolesimVis app;
    return app.main(argc, argv);
}