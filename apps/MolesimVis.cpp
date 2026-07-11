#include "Simulation.hpp"
#include "SDL.h"
#include "SDL_syswm.h"

namespace Molesim
{

struct CameraState
{
    bool flipVertically = false;
    float nearDistance = 0.1f;
    float farDistance = 1000.0f;
    float radiusScale = 1.0f;;

    Matrix4x4 projectionMatrix;
    Matrix4x4 inverseProjectionMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 inverseViewMatrix;
};

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
        printf("MolesimVis version 0.1\n");
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

        // Load the molecules
        simulation = std::make_shared<Simulation> ();
        initializeAtomColorConventions();
        for(auto &fileName : moleculeFileNames)
        {
            auto molecule = loadMolecule(fileName);
            if(!molecule)
                return 1;
            molecule->prepareForSimulation();
            simulation->molecules.push_back(molecule);
        }

        // Create the test datasets
        if(moleculeFileNames.empty())
        {
            {
                auto firstMolecule = std::make_shared<Molecule> ();
                firstMolecule->createFirstTestMolecule();
                simulation->molecules.push_back(firstMolecule);
            }

            {
                auto secondMolecule = std::make_shared<Molecule> ();
                secondMolecule->createSecondTestMolecule();
                simulation->molecules.push_back(secondMolecule);
            }
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
        cameraState.flipVertically = device->hasTopLeftNdcOrigin() == device->hasBottomLeftTextureCoordinates();

        // Create the main render pass
        {
            agpu_renderpass_color_attachment_description colorAttachment = {};
            colorAttachment.format = swapChainColorBufferFormat;
            colorAttachment.begin_action = AGPU_ATTACHMENT_CLEAR;
            colorAttachment.end_action = AGPU_ATTACHMENT_KEEP;
            colorAttachment.clear_value.r = 0.1f;
            colorAttachment.clear_value.g = 0.1f;
            colorAttachment.clear_value.b = 0.1f;
            colorAttachment.sample_count = 1;

            // Depth stencil
            agpu_renderpass_depth_stencil_description depthStencil = {};
            depthStencil.format = depthBufferFormat;
            depthStencil.begin_action = AGPU_ATTACHMENT_CLEAR;
            depthStencil.end_action = AGPU_ATTACHMENT_KEEP;
            depthStencil.clear_value.depth = 0.0;
            depthStencil.sample_count = 1;

            agpu_renderpass_description description = {};
            description.color_attachment_count = 1;
            description.color_attachments = &colorAttachment;
            description.depth_stencil_attachment = &depthStencil;

            mainRenderPass = device->createRenderPass(&description);
        }

        // Shader signature
        {
            auto shaderSignatureBuilder = device->createShaderSignatureBuilder();

            shaderSignatureBuilder->beginBindingBank(1);
            shaderSignatureBuilder->addBindingBankElement(AGPU_SHADER_BINDING_TYPE_UNIFORM_BUFFER, 1);

            shaderSignatureBuilder->beginBindingBank(1024);
            shaderSignatureBuilder->addBindingBankElement(AGPU_SHADER_BINDING_TYPE_UNIFORM_BUFFER, 1);
            shaderSignatureBuilder->addBindingBankElement(AGPU_SHADER_BINDING_TYPE_STORAGE_BUFFER, 1);

            shaderSignature = shaderSignatureBuilder->build();
            if (!shaderSignature)
                return 1;
        }

        // Camera uniform buffer
        {
            agpu_buffer_description desc;
            desc.size = agpu_uint(sizeof(CameraState));
            desc.heap_type = AGPU_MEMORY_HEAP_TYPE_HOST_TO_DEVICE;
            desc.usage_modes = desc.main_usage_mode = AGPU_UNIFORM_BUFFER;
            desc.mapping_flags = AGPU_MAP_DYNAMIC_STORAGE_BIT | AGPU_MAP_WRITE_BIT;
            desc.stride = 0;
            cameraStateBuffer = device->createBuffer(&desc, nullptr);
            if(!cameraStateBuffer)
                return 1;

            cameraStateBinding = shaderSignature->createShaderResourceBinding(0);
            cameraStateBinding->bindUniformBuffer(0, cameraStateBuffer);
        }

        // Atom pipeline state
        {
            auto vertexShader = compileShaderFromFile("assets/shaders/renderingShaderCommon.glsl", "assets/shaders/atomVertex.glsl", AGPU_VERTEX_SHADER);
            auto fragmentShader = compileShaderFromFile("assets/shaders/renderingShaderCommon.glsl", "assets/shaders/atomFragment.glsl", AGPU_FRAGMENT_SHADER);
            if (!vertexShader || !fragmentShader)
                return false;

            // Create the pipeline builder
            auto pipelineBuilder = device->createPipelineBuilder();
            pipelineBuilder->setRenderTargetFormat(0, swapChainColorBufferFormat);
            pipelineBuilder->setDepthStencilFormat(depthBufferFormat);
            pipelineBuilder->setShaderSignature(shaderSignature);
            pipelineBuilder->attachShader(vertexShader);
            pipelineBuilder->attachShader(fragmentShader);
            pipelineBuilder->setPrimitiveType(AGPU_TRIANGLE_STRIP);
            pipelineBuilder->setDepthState(true, true, AGPU_GREATER_EQUAL);

            // Build the pipeline
            atomRenderingPipeline = pipelineBuilder->build();
            if (!atomRenderingPipeline)
                return false;
        }

        // Grid rendering pipeline
        {
            auto vertexShader = compileShaderFromFile("assets/shaders/renderingShaderCommon.glsl", "assets/shaders/floorGridVertex.glsl", AGPU_VERTEX_SHADER);
            auto fragmentShader = compileShaderFromFile("assets/shaders/renderingShaderCommon.glsl", "assets/shaders/floorGridFragment.glsl", AGPU_FRAGMENT_SHADER);
            if (!vertexShader || !fragmentShader)
                return false;

            // Create the pipeline builder
            auto pipelineBuilder = device->createPipelineBuilder();
            pipelineBuilder->setRenderTargetFormat(0, swapChainColorBufferFormat);
            pipelineBuilder->setDepthStencilFormat(depthBufferFormat);
            pipelineBuilder->setShaderSignature(shaderSignature);
            pipelineBuilder->attachShader(vertexShader);
            pipelineBuilder->attachShader(fragmentShader);
            pipelineBuilder->setPrimitiveType(AGPU_TRIANGLE_STRIP);
            pipelineBuilder->setDepthState(true, false, AGPU_GREATER_EQUAL);
            pipelineBuilder->setCullMode(AGPU_CULL_MODE_NONE);
            pipelineBuilder->setBlendState(-1, true);
            pipelineBuilder->setBlendFunction(-1, AGPU_BLENDING_ONE, AGPU_BLENDING_INVERTED_SRC_ALPHA, AGPU_BLENDING_OPERATION_ADD,
            AGPU_BLENDING_ONE, AGPU_BLENDING_INVERTED_SRC_ALPHA, AGPU_BLENDING_OPERATION_ADD);

            // Build the pipeline
            gridRenderingPipeline = pipelineBuilder->build();
            if (!gridRenderingPipeline)
                return false;
        }

        // Command allocator and list
        commandAllocator = device->createCommandAllocator(AGPU_COMMAND_LIST_TYPE_DIRECT, commandQueue);
        commandList = device->createCommandList(AGPU_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr);
        commandList->close();

        // Create the molecule states.
        for(auto &molecule: simulation->molecules)
        {
            if(!createMoleculeRenderingStates(molecule))
                return 1;
        }

        // Enter the main loop
        while(!quitting)
        {
            processEvents();
            if(isSimulating)
            {
                const auto SimulationTimeStep = 1.0f / 60.0f;
                simulation->update(SimulationTimeStep);
            }
                
            render();
        }

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
    }

    bool createMoleculeRenderingStates(const MoleculePtr &molecule)
    {
        // Model state buffer
        {
            agpu_buffer_description desc;
            desc.size = agpu_uint(sizeof(ModelState));
            desc.heap_type = AGPU_MEMORY_HEAP_TYPE_HOST_TO_DEVICE;
            desc.usage_modes = desc.main_usage_mode = AGPU_UNIFORM_BUFFER;
            desc.mapping_flags = AGPU_MAP_DYNAMIC_STORAGE_BIT | AGPU_MAP_WRITE_BIT;
            desc.stride = 0;

            molecule->modelStateBuffer = device->createBuffer(&desc, nullptr);
            if(!molecule->modelStateBuffer)
                return false;
        }

        // Atom rendering state
        {
            agpu_buffer_description desc;
            desc.size = agpu_uint(sizeof(AtomRenderingState)* molecule->atomStates.size());
            desc.heap_type = AGPU_MEMORY_HEAP_TYPE_HOST_TO_DEVICE;
            desc.usage_modes = desc.main_usage_mode = AGPU_STORAGE_BUFFER;
            desc.mapping_flags = AGPU_MAP_DYNAMIC_STORAGE_BIT | AGPU_MAP_WRITE_BIT;
            desc.stride = 0;

            molecule->moleculeRenderingStateBuffer = device->createBuffer(&desc, molecule->atomStates.data());
            if(!molecule->moleculeRenderingStateBuffer)
                return false;

        }

        molecule->moleculeResourceBinding = shaderSignature->createShaderResourceBinding(1);
        molecule->moleculeResourceBinding->bindUniformBuffer(0, molecule->modelStateBuffer);
        molecule->moleculeResourceBinding->bindStorageBuffer(1, molecule->moleculeRenderingStateBuffer);
        return true;
    }

    void processKeyDownEvent(const SDL_KeyboardEvent &event)
    {
        switch(event.keysym.sym)
        {
        case SDLK_g:
            drawGrid = !drawGrid;
            break;
        case SDLK_ESCAPE:
            quitting = true;
            break;
        case SDLK_SPACE:
            isSimulating = !isSimulating;
            break;
        }
    }

    void processKeyUpEvent(const SDL_KeyboardEvent &event)
    {
        // Nothing is required yet
    }

    void processMouseMotionEvent(const SDL_MouseMotionEvent &event)
    {
        if(event.state & SDL_BUTTON_LMASK)
        {
            cameraAngle += Vector3(float(event.yrel), float(event.xrel), 0) * float(0.1f/M_PI);
            cameraAngle.x = std::min(std::max(cameraAngle.x, float(-M_PI*0.5)), float(M_PI*0.5));
        }
        else if(event.state & SDL_BUTTON_RMASK)
        {
            cameraTranslation += cameraMatrix.transposed() * Vector3(float(event.xrel), float(-event.yrel), 0) * 0.1f;
        }
    }

    void processsMouseWheelEvent(const SDL_MouseWheelEvent &event)
    {
        cameraTranslation += cameraMatrix.transposed() * Vector3(0, 0, -event.y);
    }

    void processEvents()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                processKeyDownEvent(event.key);
                break;
            case SDL_KEYUP:
                processKeyUpEvent(event.key);
                break;
            case SDL_MOUSEMOTION:
                processMouseMotionEvent(event.motion);
                break;
            case SDL_MOUSEWHEEL:
                processsMouseWheelEvent(event.wheel);
                break;
            case SDL_QUIT:
                quitting = true;
                break;
            }
        }
    }

    void render()
    {
        // Build the command list
        commandAllocator->reset();
        commandList->reset(commandAllocator, nullptr);

        // Update and upload the camera state
        cameraMatrix = Matrix3x3::XRotation(cameraAngle.x) * Matrix3x3::YRotation(cameraAngle.y);

        auto cameraInverseMatrix = cameraMatrix;
        auto cameraInverseTranslation = cameraInverseMatrix * -cameraTranslation;
        float cameraFovY = 60.0;
        float cameraAspect = float(displayWidth)/float(displayHeight);

        cameraState.viewMatrix = Matrix4x4::WithMatrix3x3AndTranslation(cameraInverseMatrix, cameraInverseTranslation);
        cameraState.inverseViewMatrix = cameraState.viewMatrix.inverse();

        cameraState.projectionMatrix = Matrix4x4::ReverseDepthPerspective(cameraFovY, cameraAspect, cameraState.nearDistance, cameraState.farDistance);
        bool flipProjectionVertically = device->hasTopLeftNdcOrigin();
        if(flipProjectionVertically)
            cameraState.projectionMatrix = Matrix4x4::ProjectionInvertYMatrix() * cameraState.projectionMatrix;

        cameraState.inverseProjectionMatrix = cameraState.projectionMatrix.inverse();

        cameraStateBuffer->uploadBufferData(0, sizeof(cameraState), &cameraState);

        // Grab the back buffer
        auto backBuffer = swapChain->getCurrentBackBuffer();

        // Begin the render pass
        commandList->setShaderSignature(shaderSignature);
        commandList->beginRenderPass(mainRenderPass, backBuffer, false);
        commandList->setViewport(0, 0, screenWidth, screenHeight);
        commandList->setScissor(0, 0, screenWidth, screenHeight);

        // Use the pipeline state
        commandList->usePipelineState(atomRenderingPipeline);
        commandList->useShaderResources(cameraStateBinding);

        // Render the molecules
        for(auto &molecule : simulation->molecules)
        {
            ModelState modelState = {};
            modelState.modelMatrix = molecule->transform.asMatrix();
            modelState.inverseModelMatrix = modelState.modelMatrix.inverse();

            molecule->modelStateBuffer->uploadBufferData(0, sizeof(ModelState), &modelState);

            commandList->useShaderResources(molecule->moleculeResourceBinding);
            commandList->drawArrays(4, molecule->atomStates.size(), 0, 0);
        }

        // Render the grid.
        if(drawGrid)
        {
            commandList->usePipelineState(gridRenderingPipeline);
            commandList->useShaderResources(cameraStateBinding);
            
            commandList->drawArrays(4, 1, 0, 0);
        }

        // Finish the command list
        commandList->endRenderPass();
        commandList->close();

        // Queue the command list
        commandQueue->addCommandList(commandList);

        swapBuffers();
        commandQueue->finishExecution();
    }

    void swapBuffers()
    {
        try
        {
            swapChain->swapBuffers();
        }
        catch(agpu_exception &e)
        {
            auto errorCode = e.getErrorCode();
            if(errorCode == AGPU_OUT_OF_DATE)
            {
                // We must recreate the swap chain.
                recreateSwapChain();
            }
            else if(errorCode == AGPU_SUBOPTIMAL)
            {
                // Ignore this case.
            }
            else
            {
                throw e;
            }
        }
    }

    void recreateSwapChain()
    {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        device->finishExecution();
        auto newSwapChainCreateInfo = currentSwapChainCreateInfo;
        newSwapChainCreateInfo.width = w;
        newSwapChainCreateInfo.height = h;
        newSwapChainCreateInfo.old_swap_chain = swapChain.get();
        swapChain = device->createSwapChain(commandQueue, &newSwapChainCreateInfo);

        screenWidth = swapChain->getWidth();
        screenHeight = swapChain->getHeight();
    }

    std::string readWholeFile(const std::string &fileName)
    {
        FILE *file = fopen(fileName.c_str(), "rb");
        if(!file)
        {
            fprintf(stderr, "Failed to open file %s\n", fileName.c_str());
            return std::string();
        }

        // Allocate the data.
        std::vector<char> data;
        fseek(file, 0, SEEK_END);
        data.resize(ftell(file));
        fseek(file, 0, SEEK_SET);

        // Read the file
        if(fread(&data[0], data.size(), 1, file) != 1)
        {
            fprintf(stderr, "Failed to read file %s\n", fileName.c_str());
            fclose(file);
            return std::string();
        }

        fclose(file);
        return std::string(data.begin(), data.end());
    }

    agpu_shader_ref compileShaderFromFile(const char *commonFileName, const char *fileName, agpu_shader_type type)
    {
        auto source = readWholeFile(commonFileName) + readWholeFile(fileName);
        if(source.empty())
            return nullptr;

        // Create the shader compiler.
        agpu_offline_shader_compiler_ref shaderCompiler = device->createOfflineShaderCompiler();
        shaderCompiler->setShaderSource(AGPU_SHADER_LANGUAGE_VGLSL, type, source.c_str(), (agpu_string_length)source.size());
        try
        {
            shaderCompiler->compileShader(AGPU_SHADER_LANGUAGE_DEVICE_SHADER, nullptr);
        }
        catch(agpu_exception &e)
        {
            auto logLength = shaderCompiler->getCompilationLogLength();
            std::unique_ptr<char[]> logBuffer(new char[logLength+1]);
            shaderCompiler->getCompilationLog(logLength+1, logBuffer.get());
            fprintf(stderr, "Compilation error of '%s':%s\n", fileName, logBuffer.get());
            return nullptr;
        }

        // Create the shader and compile it.
        return shaderCompiler->getResultAsShader();
    }


    SDL_Window *window;
    bool quitting = false;
    bool isSimulating = false;
    int screenWidth = 60*16;
    int screenHeight = 60*9;
    int displayWidth = 60*16;
    int displayHeight = 60*9;
    bool isVirtualReality = false;
    bool drawGrid = true;

    agpu_texture_format swapChainColorBufferFormat = AGPU_TEXTURE_FORMAT_B8G8R8A8_UNORM_SRGB;
    agpu_texture_format depthBufferFormat = AGPU_TEXTURE_FORMAT_D32_FLOAT;

    agpu_device_ref device;
    agpu_command_queue_ref commandQueue;
    agpu_renderpass_ref mainRenderPass;
    agpu_shader_signature_ref shaderSignature;
    agpu_command_allocator_ref commandAllocator;
    agpu_command_list_ref commandList;
    agpu_pipeline_state_ref atomRenderingPipeline;
    agpu_pipeline_state_ref gridRenderingPipeline;

    CameraState cameraState;
    Matrix3x3 cameraMatrix = Matrix3x3::Identity();
    Vector3 cameraAngle = Vector3{0, 0, 0};
    Vector3 cameraTranslation = Vector3{0, 0.5, 2};

    agpu_buffer_ref cameraStateBuffer;
    agpu_shader_resource_binding_ref cameraStateBinding;

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