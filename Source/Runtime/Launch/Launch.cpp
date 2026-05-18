#include <iostream>
#include <SDL3/SDL.h>
#include <objbase.h> // For CoInitializeEx
#include "Runtime/Engine/RvelentEngine.h"

// Force the application to use the NVIDIA discrete GPU (not Intel integrated)
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// This is the entry point for the ENTIRE application (Editor OR Game)
int main(int argc, char* argv[]) {
    
    // 0. Initialize COM as Multithreaded (MTA) for Wicked Engine Audio
    // SDL might init as STA otherwise, causing RPC_E_CHANGED_MODE crash
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "[Launch] Warning: CoInitializeEx failed: " << std::hex << hr << std::endl;
    }

    // 1. Create the Engine on the Heap
    // We use "new" because the engine is huge and we want control over its lifetime
    Rvelent::RvelentEngine* engine = new Rvelent::RvelentEngine();

    // 2. Initialize
    if (engine->Init()) {
        // 3. Run the Loop
        engine->Run();
    }

    // 4. Shutdown & Cleanup
    engine->Shutdown();
    delete engine;

    return 0;
}