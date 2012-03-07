// =======================================================================
// File: hook_handler.cpp
// Purpose: Main callback for handling hooks. Every single detour registered
//	using DynDetours will call this callback!
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "hook_types.h"
#include "conv_interface.h"
#include "detour_class.h"
#include <stdio.h>

// =======================================================================
// Handles pre call backs.
// =======================================================================
eHookRes Dyn_PreHandler( CDetour* pDetour )
{
	// ------------------------------------
	// This will handle the hook action.
	// ------------------------------------
	eHookRes tempAction;
	HookRetBuf_t* buf = NULL;

	// ------------------------------------
	// Get return value size.
	// ------------------------------------
	int iRetSize = 
		pDetour->GetFuncObj()->GetRetType()->GetSize();

	// ------------------------------------
	// Process the prehooks. This is the
	// highest priority return value after
	// all of the callbacks are executed.
	// ------------------------------------
	buf = pDetour->DoCallbacks( TYPE_PRE );

	// ------------------------------------
	// Store the action here because we
	// are going to delete the buffer
	// after this function returns.
	// ------------------------------------
	tempAction = buf->eRes;

	// ------------------------------------
	// The calling convention contains
	// the register states which the
	// assembly will pull information
	// from.
	// ------------------------------------
	ICallConvention* pConv = 
		pDetour->GetAsmBridge()->GetConv();

	// ------------------------------------
	// Set the return value of the register
	// states located in ICallConvention.
	//
	// TODO: If return value is larger than
	//	4 bytes, handle it elsewhere.
	// ------------------------------------
	pConv->GetRegisters()->r_eax = (unsigned long)buf->pRetBuf;

	// ------------------------------------
	// Free up the temporary buffer.
	// ------------------------------------
	delete buf;
	
	// ------------------------------------
	// Return the highest priority action.
	// ------------------------------------
	return tempAction;
}
