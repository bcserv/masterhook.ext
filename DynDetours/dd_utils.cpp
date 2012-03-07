// =======================================================================
// File: dd_utils.cpp
// Purpose: Useful functions used accross dyndetours.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "dd_utils.h"

// =======================================================================
// Converts a char to a type enum.
// =======================================================================
eArgType CharToTypeEnum( char c )
{
	// ------------------------------------
	// Figure out what type this char
	// represents.
	// ------------------------------------
	switch( c )
	{
		case 'c':
			return TYPE_CHAR;
		case 'i':
			return TYPE_INT32;
		case 'p':
			return TYPE_INT32_PTR;
		case 'f':
			return TYPE_FLOAT;
		case 'b':
			return TYPE_BOOL;
	}

	// ------------------------------------
	// No idea what type you want.
	// ------------------------------------
	return TYPE_UNKNOWN;
}
