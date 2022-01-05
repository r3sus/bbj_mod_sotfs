#include <windows.h>

typedef HRESULT (WINAPI *dinp8crt_t)(HINSTANCE, DWORD, REFIID,
				     LPVOID *, LPUNKNOWN);
dinp8crt_t oDirectInput8Create;

__attribute__ ((dllexport))
HRESULT WINAPI DirectInput8Create(HINSTANCE inst, DWORD ver, REFIID id,
				  LPVOID *pout, LPUNKNOWN outer)
{
    return oDirectInput8Create(inst, ver, id, pout, outer);
}

void setup_d8proxy(void)
{
    char syspath[320];
    GetSystemDirectoryA(syspath, 320);
    strcat(syspath, "\\dinput8.dll");
    HMODULE mod = LoadLibraryA(syspath);
    oDirectInput8Create = (dinp8crt_t)GetProcAddress(mod, "DirectInput8Create");
}

void assembly()
{
  asm
  ("mov  -0x24E8(%rdi),%r8   ;\
  movss 0xE8(%r8),%xmm0     ;\
  sqrtps %xmm1,%xmm1"
  );
}

void attach_hook(void)
{
void *base_addr = GetModuleHandle(NULL);

*(byte*)(base_addr + 0x1604DFA) = 0x01; // nologo patch

// nomsg
void *jne = base_addr + 0x4F7136;

DWORD op1;
VirtualProtect(jne, 1,PAGE_EXECUTE_READWRITE, &op1);

*(byte*)jne = 0x74;

VirtualProtect(jne, 1, op1, &op1);
// end

void *bbj = base_addr + 0x37B4B4;
int sz = 19;
 VirtualProtect(bbj, sz, PAGE_EXECUTE_READWRITE, &op1);
memcpy(bbj, assembly, sz);
 VirtualProtect(bbj, sz, op1, &op1);
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
	setup_d8proxy();
        attach_hook();
	break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
	break;
    }

    return TRUE;
}