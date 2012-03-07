#ifndef _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
#define _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_

#include "smsdk_ext.h"
#include "callback_manager.h"
#include "hook_types.h"
#include "arg_types.h"
#include "func_types.h"

enum ValveLibrary {

	ValveLibrary_Server,
	ValveLibrary_Engine,
	ValveLibrary_Vphysics,
	ValveLibrary_LibSteam,
	ValveLibrary_LibSteamAPI
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

cell_t Native_Mh_HookFunction(IPluginContext *pContext, const cell_t *params);
cell_t Native_Mh_UnhookFunction(IPluginContext *pContext, const cell_t *params);
cell_t Native_Mh_SetReturnValue(IPluginContext *pContext, const cell_t *params);
cell_t Native_Mh_CallOriginal(IPluginContext *pContext, const cell_t *params);
cell_t Native_Mh_Call(IPluginContext *pContext, const cell_t *params);

const sp_nativeinfo_t g_Natives[] = {
	{"Mh_HookFunction",			Native_Mh_HookFunction},
	{"Mh_UnhookFunction",		Native_Mh_UnhookFunction},
	{"Mh_SetReturnValue",		Native_Mh_SetReturnValue},
	{"MH_CallOriginal",			Native_Mh_CallOriginal},
	{"Mh_Call",					Native_Mh_Call},
	{NULL,						NULL},
};

#endif // _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
