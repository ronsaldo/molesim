#include "Simulation.hpp"
#include "AGPU/agpu.hpp"
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
        bool vsyncDisabled = false;
        bool debugLayerEnabled = false;
        agpu_uint platformIndex = 0;
        agpu_uint gpuIndex = 0;

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
                else if(!strcmp(arg, "-no-vsync"))
                {
                    vsyncDisabled = true;
                }
                else if(!strcmp(arg, "-debug"))
                {
                    debugLayerEnabled = true;
                }
                else if(!strcmp(arg, "-platform"))
                {
                    platformIndex = atoi(argv[++argc]);
                }
                else if(!strcmp(arg, "-gpu"))
                {
                    gpuIndex = atoi(argv[++argc]);
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

        // Load the molecules
        simulation = std::make_shared<Simulation> ();
        for(auto &fileName : moleculeFileNames)
        {
            auto molecule = loadMolecule(fileName);
            if(!molecule)
                return 1;
            simulation->molecules.push_back(molecule);
        }

        // Get the platform.
        agpu_uint numPlatforms;
        agpuGetPlatforms(0, nullptr, &numPlatforms);
        if (numPlatforms == 0)
        {
            fprintf(stderr, "No agpu platforms are available.\n");
            return 1;
        }
        else if (platformIndex >= numPlatforms)
        {
            fprintf(stderr, "Selected platform index is not available.\n");
            return 1;
        }

        std::vector<agpu_platform*> platforms;
        platforms.resize(numPlatforms);
        agpuGetPlatforms(numPlatforms, &platforms[0], nullptr);
        auto platform = platforms[platformIndex];
        
        printf("Chosen platform: %s\n", agpuGetPlatformName(platform));

        // Initialize SDL2 and create the window.
        SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow("MolesimVis", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

        // Get the window info.
        SDL_SysWMinfo windowInfo;
        SDL_VERSION(&windowInfo.version);
        SDL_GetWindowWMInfo(window, &windowInfo);

        // Open the device
        agpu_device_open_info openInfo = {};
        openInfo.gpu_index = gpuIndex;
        openInfo.debug_layer = debugLayerEnabled;
        if(isVirtualReality)
            openInfo.open_flags = AGPU_DEVICE_OPEN_FLAG_ALLOW_VR;

        memset(&currentSwapChainCreateInfo, 0, sizeof(currentSwapChainCreateInfo));
        switch(windowInfo.subsystem)
        {
    #if defined(SDL_VIDEO_DRIVER_WINDOWS)
        case SDL_SYSWM_WINDOWS:
            currentSwapChainCreateInfo.window = (agpu_pointer)windowInfo.info.win.window;
            break;
    #endif
    #if defined(SDL_VIDEO_DRIVER_X11)
        case SDL_SYSWM_X11:
            openInfo.display = (agpu_pointer)windowInfo.info.x11.display;
            currentSwapChainCreateInfo.window = (agpu_pointer)(uintptr_t)windowInfo.info.x11.window;
            break;
    #endif
    #if defined(SDL_VIDEO_DRIVER_COCOA)
        case SDL_SYSWM_COCOA:
            currentSwapChainCreateInfo.window = (agpu_pointer)windowInfo.info.cocoa.window;
            break;
    #endif
        default:
            fprintf(stderr, "Unsupported window system\n");
            return -1;
        }

        currentSwapChainCreateInfo.colorbuffer_format = swapChainColorBufferFormat;
        currentSwapChainCreateInfo.depth_stencil_format = depthBufferFormat;
        currentSwapChainCreateInfo.width = screenWidth;
        currentSwapChainCreateInfo.height = screenHeight;
        currentSwapChainCreateInfo.buffer_count = 3;
        currentSwapChainCreateInfo.flags = AGPU_SWAP_CHAIN_FLAG_APPLY_SCALE_FACTOR_FOR_HI_DPI;
        if (vsyncDisabled || isVirtualReality)
        {
            currentSwapChainCreateInfo.presentation_mode = AGPU_SWAP_CHAIN_PRESENTATION_MODE_MAILBOX;
            currentSwapChainCreateInfo.fallback_presentation_mode = AGPU_SWAP_CHAIN_PRESENTATION_MODE_IMMEDIATE;
        }

        device = platform->openDevice(&openInfo);
        if(!device)
        {
            fprintf(stderr, "Failed to open the device\n");
            return false;
        }

        printf("Chosen device: %s\n", device->getName());

        // Get the default command queue
        commandQueue = device->getDefaultCommandQueue();

        // Create the swap chain.
        swapChain = device->createSwapChain(commandQueue, &currentSwapChainCreateInfo);
        if(!swapChain)
        {
            fprintf(stderr, "Failed to create the swap chain\n");
            return false;
        }

        displayWidth = swapChain->getWidth();
        displayHeight = swapChain->getHeight();

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
    int displayWidth = 60*16;
    int displayHeight = 60*9;
    bool isVirtualReality = false;

    agpu_texture_format swapChainColorBufferFormat = AGPU_TEXTURE_FORMAT_B8G8R8A8_UNORM_SRGB;
    agpu_texture_format depthBufferFormat = AGPU_TEXTURE_FORMAT_D32_FLOAT;

    agpu_device_ref device;
    agpu_command_queue_ref commandQueue;

    agpu_swap_chain_create_info currentSwapChainCreateInfo;
    agpu_swap_chain_ref swapChain;
    SimulationPtr simulation;
};

}

int main(int argc, const char **argv)
{
    Molesim::MolesimVis app;
    return app.main(argc, argv);
}