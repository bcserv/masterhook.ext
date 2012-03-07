// =======================================================================
// File: arg_types.h
// Purpose: This file defines a number of constants which represent
//  attributes of a function argument (be it type, or size).
// =======================================================================
#ifndef _ARG_TYPES_H
#define _ARG_TYPES_H

#ifndef _WIN32
#define __int64 long long
#endif

// =======================================================================
// Types of arguments we can have.
// =======================================================================
enum eArgType 
{
	TYPE_UNKNOWN,   // Type is not a default type.
	TYPE_VOID,		// void
	TYPE_BOOL,		// bool
	
	TYPE_CHAR,		// char
	TYPE_CHAR_PTR,	// char*
	
	TYPE_FLOAT,		// float
	TYPE_FLOAT_PTR, // float*
	
	TYPE_INT8,      // byte
	TYPE_INT8_PTR,  // byte*
	
	TYPE_INT16,     // short
	TYPE_INT16_PTR, // short*
	
	TYPE_INT32,     // int
	TYPE_INT32_PTR, // int*
	
	TYPE_INT64,		// int64
	TYPE_INT64_PTR	// int64*
};

// =======================================================================
// If we're passed by reference or not.
// =======================================================================
enum eArgPassType
{
	PASS_UNKNOWN,	// Unknown passing convention.
	PASS_BYREF,		// Argument passed by reference to function.
	PASS_BYVAL		// Argument passed by value to the function.
};

#endif // _ARG_TYPES_H
