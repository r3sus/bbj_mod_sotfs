#include <windows.h>

#include "mmr.c"

#include "d8w.c"

void asm_nobbj() { asm ("mov r8, [rdi-0x24e8]; movss xmm0, [r8+0xe8]; sqrtps xmm1,xmm1; "); }

void attach_hook()
{

void *base = GetModuleHandle(NULL);

wp(base+0x37B4B4, asm_nobbj, 'a');

}
       
BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
	attach_hook();
        FreeLibrary(mod);
	break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
	break;
    }
    return TRUE;
}
