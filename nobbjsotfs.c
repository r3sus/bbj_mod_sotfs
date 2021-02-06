/*
 *    build cmd with mingw64:
 *        gcc -Wall -Wl,--out-implib,libmessage.a -Wl,--enable-stdcall-fixup
 *            exports.DEF nobbjsotfs.c -shared -o DINPUT8.dll
 *
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*
 *    This patching mechanism taken from:
 *        https://github.com/bladecoding/DarkSouls3RemoveIntroScreens/blob/master/SoulsSkipIntroScreen/dllmain.cpp
 *
 *
 */
struct patch {
    DWORD rel_addr;
    DWORD size;
    char patch[50];
    char orig[50];
};

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

void attach_hook(void)
{ 	
	// This whole function is untidy and the variable names are terrible, sorry!
	// One day I'll improve as a person and sort this out.
	
	// Here the inject/patch uses the scratch r10 register for dumping variables
		
    void *base_addr = GetModuleHandle(NULL);
	
	/*
		Add the "Baby Jump fix (only one can be active at a time)" code
	*/
	
	static char spd_bytes[] =
	{
		//orig:
		//0x84, 0xC0,										// test al,al                
		//0x74, 0x07,               						// je 07 
		//0x80, 0x8F, 0xE8, 0x00, 0x00, 0x00, 0x08,    	// or byte ptr [rdi+E8]
		
		//update jmp from dll (64bit)
		0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// [0] mov r10, tempaddr_xxxxxxxx
		0x49, 0x8b, 0x02, 												// mov rax, [r10] 
		
		// CE newmem:
		0x48, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// [13] mov qword ptr [BasePtr (xxxxxxxx)], rax
		
		//orig:
		0x48, 0x8B, 0x58, 0x38, 										// [23] mov rbx, [rax+38]   
		
		//return to orig (64bit jmp)
		0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// [27] mov r10, addr_xxxxxxxx
		0x41, 0xFF, 0xE2, 												// [37] jmp r10 
		
		// speedBse:xxxxxxxx
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};			

	void *spdhook = spd_bytes;
	void *BasePtr = &spd_bytes[40]; // (aka speedBse in Vanilla)
	
	struct patch spd_patch = 
	    {
			0x3EE3F9, 												// "Base"  
			14,
			// inject:
			{0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// mov r10, addr_xxxxxxxx
			 0x41, 0xFF, 0xE2,												// [10] jmp r10
			 0x90}, 														// [13] nop filler
			// orig bytes:
			{//0x84, 0xC0,                				// test al,al
			 //0x74, 0x07,                				// je DarkSoulsII.exe+3EE3F9 
			 //0x80, 0x8F, 0xE8, 0x00, 0x00, 0x00, 0x08,  // or byte ptr [rdi+e8]
			 0x48, 0x8B, 0x05, 0xD0, 0xD4, 0x21, 0x01,  // mov rax,[DarkSoulsII.exe+160B8D0]
			 0x48, 0x8B, 0x58, 0x38,    				// mov rbx,[rax+38]
			 0x48, 0x85, 0xDB 							// [11] test rbx,rbx
			 }, 			
	    };
		
	void *spdinjaddr = base_addr + spd_patch.rel_addr;
	void *spdinj_ret = spdinjaddr + spd_patch.size;
	
	DWORD temprel = 0x160B8D0;
	void *tmpaddr = base_addr + temprel;
	
	uint64_t addrToJumpTo64 = (uint64_t) spdhook;
    memcpy(&spd_patch.patch[2], &addrToJumpTo64, sizeof(addrToJumpTo64)); 		// jmp fw
	memcpy(&spd_bytes[29],&spdinj_ret,sizeof(spdinj_ret)); 						// return to spdinject (+offset)
	memcpy(&spd_bytes[2],&tmpaddr,sizeof(tmpaddr)); 							// fix mov rax,[DarkSoulsII.exe+160B8D0] 
	memcpy(&spd_bytes[15],&BasePtr,sizeof(BasePtr)); 							// fix mov [BasePtr],rax 
    
		/*
	char str[50];
	sprintf(str,"baseptrsize %lld",sizeof(BasePtr));
	MessageBox(0,str,"",0);
	*/
	
	DWORD op;
	VirtualProtect(spd_bytes, sizeof(spd_bytes),
		       PAGE_EXECUTE_READWRITE, &op);
	
	// Edit memory:
	DWORD size = spd_patch.size;
	if (memcmp(spdinjaddr, spd_patch.orig, size) == 0) {
		DWORD old;
		VirtualProtect(spdinjaddr, size, PAGE_EXECUTE_READWRITE, &old);
		memcpy(spdinjaddr, spd_patch.patch, size);
		VirtualProtect(spdinjaddr, size, old, &old);
	}
	
	
	/*	
		Add the "NoBabyJ (full jump length)" code
	*/
	static char jump_bytes[] =
	{
		// orig:
		0xF3, 0x0F, 0x58, 0xC8, 									// addss  xmm1,xmm0
		0x0F, 0xC6, 0xC9, 0x00,            							// shufps xmm1,xmm1,00
		0x0F, 0x51, 0xC1,     										// sqrtps xmm0,xmm1
		
		// newmem:
		0x50, 														// [11] push rax
		0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [12] mov rax,[BasePtr (xxxxxxxx)]
		0x48, 0x8B, 0x80, 0xD0, 0x00, 0x00, 0x00, 					// [22] mov rax,[rax+D0]
		0x48, 0x8B, 0x80, 0xF8, 0x00, 0x00, 0x00, 					// [29] mov rax,[rax+F8]
		0x48, 0x8B, 0x80, 0xF0, 0x00, 0x00, 0x00, 					// [36] mov rax,[rax+F0]
		0xF3, 0x0F, 0x10, 0x80, 0x98, 0x37, 0x00, 0x00, 			// [43] movss xmm0,[rax+3798]
		0x58, 														// [51] pop rax
		
		// orig:
		0x0F, 0x29, 0x44, 0x24, 0x20, 								// [52] movaps [rsp+20], xmm0
		
		//return to orig (64bit jmp)
		0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [57] mov r10, addr_xxxxxxxx
		0x41, 0xFF, 0xE2, 											// jmp r10 
	};

	void *jumphook = jump_bytes;
	
	struct patch jump_patch = 
	    {
			0x37B4BC, 												// "INJECT"  
			16,
			// inject:
			{0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// mov r10, addr_xxxxxxxx
			 0x41, 0xFF, 0xE2,												// [10] jmp r10
			 0x90, 0x90, 0x90}, 											// [13] nop filler
			// orig:
			{0xF3, 0x0F, 0x58, 0xC8, 				// addss xmm1,xmm0
			 0x0F, 0xC6, 0xC9, 0x00,            	// shufps xmm1,xmm1,00
			 0x0F, 0x51, 0xC1,     					// sqrtps xmm0,xmm1
			 0x0F, 0x29, 0x44, 0x24, 0x20},       	// movaps [rsp+20],xmm0
	    };
	
	void *jumpinjaddr = base_addr + jump_patch.rel_addr;
	void *jumpinj_ret = jumpinjaddr + jump_patch.size;
	
	//DWORD temprel = 0x160B8D0;
	//void *tmpaddr = base_addr + temprel;
	
	addrToJumpTo64 = (uint64_t) jumphook;
	memcpy(&jump_patch.patch[2], &addrToJumpTo64, sizeof(addrToJumpTo64)); 		// jmp fw
	memcpy(&jump_bytes[59],&jumpinj_ret,sizeof(jumpinj_ret)); 					// return to jumpinject (+offset)
	
	memcpy(&jump_bytes[14],&BasePtr,sizeof(BasePtr)); 							// fix mov rax,[BasePtr]
	
		
	// Edit memory:
	size = jump_patch.size;
	if (memcmp(jumpinjaddr, jump_patch.orig, size) == 0) {
		DWORD old;
		VirtualProtect(jumpinjaddr, size, PAGE_EXECUTE_READWRITE, &old);
		memcpy(jumpinjaddr, jump_patch.patch, size);
		VirtualProtect(jumpinjaddr, size, old, &old);
	}
	
	
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason,
		      LPVOID res)
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
