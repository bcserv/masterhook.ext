// =======================================================================
// File: conv_cdecl.cpp
// Purpose: Defines the implementation of the CThiscall_Convention class.
//	This class will generate ASM code for saving registers and 
//	cleaning up the stack.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "conv_thiscall.h"

// =======================================================================
// Namespaces to use.
// =======================================================================
using namespace AsmJit;

// =======================================================================
// Stage 1. Called by the detour system to save registers.
// =======================================================================
void CThiscall_Convention::Stage_1( Assembler* pAssembler )
{
	// ------------------------------------
	// 1) Save the stack pointer
	// ------------------------------------
	pAssembler->mov( dword_ptr_abs(&m_Registers.r_esp), esp );

	// ------------------------------------
	// 2) Save the this pointer.
	// ------------------------------------
	pAssembler->mov( dword_ptr_abs(&m_Registers.r_ecx), ecx );

}

// =======================================================================
// Stage 2. Called by the detour system to restore registers.
// =======================================================================
void CThiscall_Convention::Stage_2( Assembler* pAssembler )
{
	// ------------------------------------
	// 1) Restore ESP.
	// ------------------------------------
	pAssembler->mov( esp, dword_ptr_abs(&m_Registers.r_esp) );

	// ------------------------------------
	// 1) Restore ECX.
	// ------------------------------------
	pAssembler->mov( ecx, dword_ptr_abs(&m_Registers.r_ecx) );
}

// =======================================================================
// Stage 2. Called by the detour system to restore registers.
// =======================================================================
void CThiscall_Convention::Stage_3( Assembler* pAssembler )
{
	// ------------------------------------
	// 1) Restore ECX.
	// ------------------------------------
	pAssembler->mov( ecx, dword_ptr_abs(&m_Registers.r_ecx) );

	// ------------------------------------
	// 2) Restore ESP.
	// ------------------------------------
	pAssembler->mov( esp, dword_ptr_abs(&m_Registers.r_esp) );
}
