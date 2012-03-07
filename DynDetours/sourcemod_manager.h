// =======================================================================
// File: cpp_manager.h
// Purpose: Declares the CCPPManager callback manager. This class is
//	responsible for adding / creating callbacks for the C++ language.
// =======================================================================
#ifndef _SOURCEMOD_MANAGER_H
#define _SOURCEMOD_MANAGER_H

// =======================================================================
// Includes
// =======================================================================
#include "callback_manager.h"
#include "hook_types.h"
#include "func_types.h"
#include <vector>

#include "smsdk_ext.h"
#include <IBinTools.h>
#include <eiface.h>
#include "compat_wrappers.h"



// =======================================================================
// Valve SDK: Common Data Types
// =======================================================================
enum SDKCommonDataType {
	SDKDataType_Unknown,			/**< It's unknown */
	SDKDataType_Int,				/**< Integer */
	SDKDataType_Bool,				/**< Boolean */
	SDKDataType_Float,				/**< Float */
	SDKDataType_String,				/**< String */
	SDKDataType_CBaseEntity,		/**< CBaseEntity */
	SDKDataType_Edict,				/**< Edict */
	SDKDataType_Vector,				/**< Vector */
	SDKDataType_QAngle,				/**< QAngle */
	SDKDataType_CTakeDamageInfo		/**< CTakeDamageInfo */
};

enum SDKPassType {
	ParameterType_Value,
	ParameterType_Reference,
	ParameterType_Pointer
};

struct ArgumentDescription {
	SDKCommonDataType type;
	bool isConst;
	SDKPassType passType;
};

// =======================================================================
// Forward declarations
// =======================================================================
class CDetour;

// =======================================================================
// This is the function prototype for a C++ callback.
// =======================================================================
typedef HookRetBuf_t* (*SourcemodCallBack)( CDetour* );

// =======================================================================
// The CCPPManager class.
// =======================================================================
class SourcemodManager : public ICallbackManager
{
	private:
		// ------------------------------------
		// Callback lists.
		// ------------------------------------
		std::vector<IPluginFunction*> m_vecPreCalls;
		std::vector<IPluginFunction*> m_vecPostCalls;

		// ------------------------------------
		// Demangled Symbol Name without Args
		// ------------------------------------
		char name[256];

		// ------------------------------------
		// Parameter Description List
		// ------------------------------------
		std::vector<ArgumentDescription> args;

		// ------------------------------------
		// Parameter Description List
		// ------------------------------------
		SDKCommonDataType retDesc;

		// ------------------------------------
		// Parameter Description List
		// ------------------------------------
		SDKCommonDataType classType;

		// ------------------------------------
		// Return Value
		// ------------------------------------
		void* retValue;

	public:
		// ------------------------------------
		// Language accessor.
		// ------------------------------------
		virtual const char* GetLang( void ) {
			return "Sourcemod";
		}

		// ------------------------------------
		// Callback adding / removal.
		// ------------------------------------
		virtual void Add( void* pFuncObj, eHookType type ) { };
		virtual void Remove( void* pFuncObj, eHookType type ) { };
		virtual void Add( IPluginFunction* pFuncObj, eCallConv conv, eHookType type );
		virtual void Remove( IPluginFunction* pFuncObj, eHookType type );

		// ------------------------------------
		// Remove All By Context
		// ------------------------------------
		void RemoveAllByContext(IPluginContext* plugincontext);

		std::vector<IPluginFunction*>* GetPreCalls();

		// ------------------------------------
		// Callback processing.
		// ------------------------------------
		virtual HookRetBuf_t* DoPreCalls( CDetour* pDet );
		virtual HookRetBuf_t* DoPostCalls( CDetour* pDet );

		void SetArgs(std::vector<ArgumentDescription> _args) {
			this->args = _args;
		}

		void SetReturnType(SDKCommonDataType type) {
			this->retDesc = type;
		}

		char* GetName() {
			return this->name;
		}

		SDKCommonDataType GetReturnType() {
			return this->retDesc;
		}

		void SetSymbolname(char *_name) {
			memmove(this->name, _name, 256);

			char classType[256];
			char* twoColons = strstr(_name, "::");
			if (twoColons != NULL) {
				unsigned int len = (unsigned int)(twoColons-_name);
				memmove(classType, _name, len);
				classType[len] = '\0';

				if (strcmp(classType, "CBaseEntity")) {
					this->classType = SDKDataType_CBaseEntity;
				}
			}

			this->classType = SDKDataType_Unknown;
		}

		void SetReturnValue(void* value) {
			this->retValue = value;
		}
};

extern SourcemodManager* SMM_active;

#endif // _SOURCEMOD_MANAGER
