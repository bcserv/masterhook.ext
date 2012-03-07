#if defined _masterhook_included
 #endinput
#endif
#define _masterhook_included

enum ValveLibrary {
	ValveLibrary_Server,
	ValveLibrary_Engine,
	ValveLibrary_Vphysics,
	ValveLibrary_LibSteam,
	ValveLibrary_LibSteamAPI
};

enum MasterhookHookType {
	MhHookType_Pre,			// Hook the function BEFORE it's getting called (allows us to block it and more)
	MhHookType_Post			// Hook the function AFTER it got called
};

enum MasterhookDataType {
	MhDataType_Unknown,		// Type is not a default type.

	MhDataType_Void,		// void
	MhDataType_Bool,		// bool
	
	MhDataType_Char,		// char
	MhDataType_Char_Ptr,	// char*
	
	MhDataType_Float,		// float
	MhDataType_Float_Ptr,	// float*
	
	MhDataType_Int8,     	// byte
	MhDataType_Int8_Ptr,  	// byte*
	
	MhDataType_Int16,    	// short
	MhDataType_Int16_Ptr, 	// short*
	
	MhDataType_Int32,     	// int
	MhDataType_Int32_Ptr, 	// int*
	
	MhDataType_Int64,		// int64
	MhDataType_Int64_Ptr, 	// int64*
	
	MhDataType_CBaseEntity, // CBaseEntity* (entity) or CBasePlayer* (player)
	MhDataType_Edict,		// Edict (networked entity with index)
	MhDataType_CTakeDamageInfo,		// CTakeDamageInfo (Damage Info Class)
	MhDataType_Vector,		// Vector
	MhDataType_QAngle		// QAngle
};

enum MasterhookPassType {
	MhPassType_Unknown,	// Ignore the return value
	MhPassType_ByValue,
	MhPassType_ByRef,
};

/**
 * @brief	Hooks a function
 *
 * @param	symbol		Demangled symbol name
 * @param	lib			Library type (see: enum ValveLibrary)
 * @param	callback	Callback function, there is no function prototype checking, so be sure the prototype is right
 * @param	returntype	Return Type of the hooked function
 * @return	Returns true if hook was successful or false otherwise
 */
native bool:Mh_HookFunction(
	const String:symbol[],
	ValveLibrary:lib=ValveLibrary_Server,
	any:callback,
	MasterhookDataType:returnType,
	MasterhookHookType:hookType=MhHookType_Pre
);

/**
 * @brief Sets a return value inside a hook callback
 *
 * @param	value		return value
 * @noreturn
 */
native Mh_SetReturnValue(any:value);

/**
 * @brief	Call the original function inside a hook,
 * 			in case you want to call it somewhere in the middle of the callback,
 * 			otherwise just return 
 *
 * @param	entity		Entity index
 * @param	type		Type of function to unhook
 * @param	callback	Callback function to unhook
 * @noreturn
 */
native any:Mh_CallOriginal();

/**
 * @brief	Call a function by a symbol
 *
 * @param	symbol		Demangled symbol name
 * @param	type		Type of function to unhook
 * @return	The return value the called function returned if any
 */
native any:Mh_Call(
	const String:symbol[],
	ValveLibrary:lib=ValveLibrary_Server,
	MasterhookDataType:returnType,
	any:...
);




/** Do Not Edit Below This Line **/

public Extension:__ext_masterhook =
{
	name = "masterhook",
	file = "masterhook.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};