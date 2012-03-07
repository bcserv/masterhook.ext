// =======================================================================
// File: conv_main.cpp
// Purpose: Links in all calling convention types. This file will
//	disappear as soon as I write dynamic calling convention instantiation
//	code.
// 
// Ideas: Creating a linked list of convention names (stored as strings),
//	paired to a function to generate new instances of calling convention
//	classes.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "conv_main.h"
#include "conv_cdecl.h"
#include "conv_thiscall.h"
#include "conv_stdcall.h"

// =======================================================================
// Returns an ICallConvention interface based on the given calling 
// convention enum.
// =======================================================================
ICallConvention* EnumToConvention( eCallConv conv )
{
	switch( conv )
	{
		// ------------------------------------
		// Create the proper class instance for
		// its respective calling convention.
		// ------------------------------------
		case CONV_CDECL:
			return new CCdecl_Convention();

		case CONV_THISCALL:
			return new CThiscall_Convention();

		case CONV_STDCALL:
			return new CStdCall_Convention();

		default:
			return NULL;
	}

	// ------------------------------------
	// Shouldn't happen but good style.
	// ------------------------------------
	return NULL;
}
