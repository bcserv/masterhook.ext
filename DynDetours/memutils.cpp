/**
* ======================================================
* DynDetours
* Copyright (C) 2009 Deniz Sezen
* All rights reserved.
* ======================================================
*
* This software is provided 'as-is', without any express or implied warranty.
* In no event will the authors be held liable for any damages arising from 
* the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose, 
* including commercial applications, and to alter it and redistribute it 
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not 
* claim that you wrote the original software. If you use this software in a 
* product, an acknowledgment in the product documentation would be 
* appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
*
* SetMemPatchable and ProtectMemory taken entirely from CSSDM by
* David "BAILOPAN" Anderson.
*/

// ==================================================================
// Includes
// ==================================================================
#include "memutils.h"
#include "asm.h"

// ==================================================================
// Removes/Adds protection from/to memory
// ==================================================================
void ProtectMemory(void *addr, int length, int prot)
{
#if defined __linux__
	void *addr2 = (void *)ALIGN(addr);
	mprotect(addr2, sysconf(_SC_PAGESIZE), prot);
#elif defined _WIN32
	DWORD old_prot;
	VirtualProtect(addr, length, prot, &old_prot);
#endif

}

// ==================================================================
// Wrapper for ProtectMemory.
// ==================================================================
void SetMemPatchable(void *address, size_t size)
{
	ProtectMemory(address, (int)size, PAGE_EXECUTE_READWRITE);
}

// ==================================================================
// Wrapper for ProtectMemory.
// ==================================================================
void WriteJMP(unsigned char* src, void* dest)
{
	// Make sure we can modify bytes at this address
	SetMemPatchable(src, 20);

	// Inject the jump
	inject_jmp((void *)src, dest);
}
