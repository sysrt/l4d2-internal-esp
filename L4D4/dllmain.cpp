#include "framework.h"

#include "hook.cpp"

void __stdcall Entry() {
    if (!AllocConsole())
        return;

    FILE* dummy = nullptr;
    if (freopen_s(&dummy, "CONOUT$", "w", stdout) != 0)
        return;

    std::cout << R"(
                      gG88Pp                    
                     8888888b                   
                    d888::::8)                  
   G88888888888888  Y88888::8Y  88888888888888P 
    ~d8:::::::::88  `88888888'  88:::::::;88'~  
      ~d88';88::88    P8888P    88:;88:;88'~    
         ~d88888888,    YY    ,888888888'       
                  `p88888888889`                
            .d8888. `888888888'.d!!!!b          
            Y8888888.   YY   .d!!!!!!!b         
            ~d88888888. YY  y!!!!!!!!b'         
                  `8888.YYy!!!!!'               
                    `888Y!!!!!'                 
                     y!!!!!!!                   
                   y!!!!!!!888a                 
                 y!!!!!~YY~d888p                
                d!!!!!" YY  88888               
                !!!!!C  YY  9888p               
                 ~d!!!a YYy9888y'               
                   ~!!!!Y888889'                
                    .#8888889.                  
                  .8888888#!!!!.                
                .d88888'YY `!!!!!               
                Y88888' YY  `!!!!'              
                d88Y'   YY    `!!'
    )" << std::endl;

    std::cout << "getting modules\n";

    engine_dll = GetModuleHandle(L"engine.dll");

    server_dll = GetModuleHandle(L"server.dll");

    if (engine_dll && server_dll == nullptr) {
        std::cout << "error when loading modules\n";
    }

    std::cout << "engine_dll :: " << (DWORD*)engine_dll << std::endl;
    std::cout << "server_dll :: " << (DWORD*)server_dll << std::endl;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Entry), nullptr, 0, nullptr);
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Hkinit), nullptr, 0, nullptr);
    }
    return TRUE;
}