#include "extension.h"
#include "natives.h"

#include "sourcemod_manager.h"
#include "detour_class.h"
#include "detourman_class.h"
#include "symboltable.h"
#include "typeconverter.h"
#include "arg_types.h"


eArgType TranslateMasterHookType(size_t type) {

	switch (type) {

		case MhDataType_Unknown: {
			return TYPE_UNKNOWN;
		}
		case MhDataType_Void: {
			return TYPE_VOID;
		}
		case MhDataType_Bool: {
			return TYPE_BOOL;
		}
		case MhDataType_Char: {
			return TYPE_CHAR;
		}
		case MhDataType_Char_Ptr: {
			return TYPE_CHAR_PTR;
		}
		case MhDataType_Float: {
			return TYPE_FLOAT;
		}
		case MhDataType_Float_Ptr: {
			return TYPE_FLOAT_PTR;
		}
		case MhDataType_Int8: {
			return TYPE_INT8;
		}
		case MhDataType_Int8_Ptr: {
			return TYPE_INT8_PTR;
		}
		case MhDataType_Int16: {
			return TYPE_INT16;
		}
		case MhDataType_Int16_Ptr: {
			return TYPE_INT16_PTR;
		}
		case MhDataType_Int32: {
			return TYPE_INT32;
		}
		case MhDataType_Int32_Ptr: {
			return TYPE_INT32_PTR;
		}
		case MhDataType_Int64: {
			return TYPE_INT64;
		}
		case MhDataType_Int64_Ptr: {
			return TYPE_INT64;
		}
	}

	return TYPE_UNKNOWN;
}

char **strtokenizer(const char *message, char *buf, unsigned int n, char delimiter) {
	char **parray = new char*[n];
	parray[0] = buf;

	unsigned int x = 1, i;
	for (i=0; message[i] != '\0'; ++i) {
		if (x < n && message[i] == delimiter) {
			buf[i] = '\0';
			parray[x++] = buf+i+1;
		}
		else {
			buf[i] = message[i];
		}
	}
	buf[i] = '\0';

	//preventing crashes
	for (i=x; i<n; ++i) {
		parray[i] = "\0";
	}

	return parray;
}

int ParseDemangledSymbolArguments(const char* args, CFuncObj* pFuncObj, std::vector<ArgumentDescription>* vecArgs) {

	// ------------------------------------
	// Return 0 when empty
	// ------------------------------------
	if(!args) {
		return 0;
	}

	int numArgs=0;

	char buf[1024];
	char argBuf[256];
	char** toks = strtokenizer(args, buf, 32, ',');
	char** words;

	for (int i=0; i<32; i++) {

		if (toks[i][0] == '\0') {
			break;
		}

		if (toks[i][0] == ' ') {
			toks[i]++;
		}

		char *dataType = NULL;
		int bPointer = false;
		int bUnsigned = false;
		words = strtokenizer(toks[i], argBuf, 4, ' ');

		eArgPassType tmpPass = PASS_BYVAL;
		eArgType tmpType;
		ArgumentDescription argDesc;
		argDesc.type = SDKDataType_Int;
		argDesc.isConst = false;
		argDesc.passType = ParameterType_Value;

		int x=0;
		for (; x<4; x++) {

			if (words[x][0] == '\0') {
				break;
			}

			if (strcmp(words[x], "unsigned") == 0) {
				// It can't be negative !
				bUnsigned = true;
			}
			else if (x == 0 || (bUnsigned && x==1)) {
				// I'm a racist
				dataType = words[x];
			}
			else if (strstr(words[x], "const") == words[x] && (words[x][5] == '\0' || words[x][5] == '*' || words[x][5] == '&')) {
				// It's a const !
				argDesc.isConst = true;
			}

			char lastChar = words[x][strlen(words[x])-1];
			if (lastChar == '*' || lastChar == '&') {

				if (lastChar == '*') {
					bPointer=true;
					argDesc.passType = ParameterType_Pointer;
				}
				else {
					tmpPass = PASS_BYREF;
					argDesc.passType = ParameterType_Reference;
				}

				words[x][strlen(words[x])-1] = '\0';
			}
		}

#ifdef DEBUG
		META_CONPRINTF("[MASTERHOOK] Args: bUnsigned: %d dataType: %s isConst: %d\n", bUnsigned, dataType, argDesc.isConst);
#endif

		if (x == 0) {
			META_CONPRINTF("[Masterhook] There was a problem parsing the argument list for a symbol");
			return 0;
		}

		if (dataType != NULL) {

			if (strcmp(dataType, "void") == 0) {
				tmpType = TYPE_VOID;
				argDesc.type = SDKDataType_Unknown;
			}
			else if (strcmp(dataType, "bool") == 0) {
				tmpType = TYPE_BOOL;
				argDesc.type = SDKDataType_Bool;
			}
			else if (strcmp(dataType, "char") == 0) {
				if (bPointer) {
					tmpType = TYPE_CHAR_PTR;
					argDesc.type = SDKDataType_String;
				}
				else {
					tmpType = TYPE_CHAR;
					argDesc.type = SDKDataType_Int;
				}
			}
			else if (strcmp(dataType, "float") == 0) {
				if (bPointer) {
					tmpType = TYPE_FLOAT_PTR;
				}
				else {
					tmpType = TYPE_FLOAT;
				}
				argDesc.type = SDKDataType_Float;
			}
			else if (strcmp(dataType, "byte") == 0) {
				if (bPointer) {
					tmpType = TYPE_INT8_PTR;
				}
				else {
					tmpType = TYPE_INT8;
				}
				argDesc.type = SDKDataType_Int;
			}
			else if (strcmp(dataType, "short") == 0) {
				if (bPointer) {
					tmpType = TYPE_INT16_PTR;
				}
				else {
					tmpType = TYPE_INT8;
				}
				argDesc.type = SDKDataType_Int;
			}
			else if (strcmp(dataType, "int") == 0) {
				if (bPointer) {
					tmpType = TYPE_INT32_PTR;
				}
				else {
					tmpType = TYPE_INT32;
				}
				argDesc.type = SDKDataType_Int;
			}
			else if (strcmp(dataType, "Vector") == 0) {
				argDesc.type = SDKDataType_Vector;
			}
			else if (strcmp(dataType, "QAngle") == 0) {
				argDesc.type = SDKDataType_QAngle;
			}
			else if (bPointer && (strcmp(dataType, "CBaseEntity") == 0 || strcmp(dataType, "CBasePlayer") == 0)) {
				argDesc.type = SDKDataType_CBaseEntity;
			}
			else if (bPointer && strcmp(dataType, "CTakeDamageInfo") == 0) {
				argDesc.type = SDKDataType_CTakeDamageInfo;
			}

			if (pFuncObj != NULL) {
				pFuncObj->AddArg( tmpType, tmpPass );
			}

			vecArgs->push_back(argDesc);
			numArgs++;
		}

		delete []words;
	}

	delete []toks;

	return numArgs;
}

void *GetLibraryHandle(ValveLibrary libtype) {
	
	void *factory = NULL;

	switch (libtype) {
		case ValveLibrary_Server: {
			factory = (void*)g_SMAPI->GetServerFactory(false);
			break;
		}
		case ValveLibrary_Engine: {
			factory = (void*)g_SMAPI->GetEngineFactory(false);
			break;
		}
		case ValveLibrary_Vphysics: {
			factory = (void*)g_SMAPI->GetPhysicsFactory(false);
			break;
		}
	}

	if (!factory) {
		return NULL;
	}

#if defined PLATFORM_POSIX
	Dl_info info;
	/* GNU only: returns 0 on error, inconsistent! >:[ */
	if (dladdr((const void*)factory, &info) != 0) {
		void *handle = dlopen(info.dli_fname, RTLD_NOW);
		if (handle) {
			return handle;
		} else {
			META_CONPRINTF("[Masterhook] Unable to load library type %d\n", libtype);
		}
	}
#endif

	return NULL;
}

cell_t Native_Mh_HookFunction(IPluginContext *pContext, const cell_t *params) {

	void *handle	= NULL;
	void *target	= NULL;
	
	handle = GetLibraryHandle((ValveLibrary)params[2]);

	if (handle == NULL) {
		return pContext->ThrowNativeError("Can't get library handle");
	}

	char *symbol;
	pContext->LocalToString(params[1], &symbol);

	//char args[512];
	//unsigned char symType;
	eCallConv conv = CONV_CDECL;

#if defined PLATFORM_POSIX
	int symType = -1;
	char args[2048];
	target = ResolveDemangledSymbol(handle, symbol, symType, args, 2048);
	dlclose(handle);
#endif

	if (target == NULL) {
		return pContext->ThrowNativeError("Couldn't find symbol \"%s\" in symboltable", symbol);
	}

#if defined PLATFORM_POSIX
	if (symType != STT_FUNC) {
		return pContext->ThrowNativeError("Symbol \"%s\" doesn't point to a function", symbol);
	}
#endif

	if (strstr(symbol, "::")) {
		conv = CONV_THISCALL;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[3]);
	
	// =======================================================================
	// This will create a C++ function callback for a detour.
	// =======================================================================
	// ------------------------------------
	// Can't continue if any of these are
	// null.
	// ------------------------------------
	if (!callback) {
		return pContext->ThrowNativeError("Callback is not defined");
	}

#ifdef DEBUG
	META_CONPRINTF("[MASTERHOOk] Target: 0x%X Symbol: %s\n", target, symbol);
#endif

	// ------------------------------------
	// Check to see if a detour already
	// exists at this function address.
	// ------------------------------------
	CDetour* pDetour = g_DetourManager.Find_Detour(target);

	// ------------------------------------
	// If the detour is not valid,
	// we need to create it and add it
	// to the manager.
	// ------------------------------------
	std::vector<ArgumentDescription> vecArgs;
	if (!pDetour) {
		CFuncObj* pFuncObj = new CFuncObj(target, conv);
#if defined PLATFORM_POSIX
		ParseDemangledSymbolArguments(args, pFuncObj, &vecArgs);
		pFuncObj->SetRetType(TranslateMasterHookType(params[4]));
#endif
		pDetour = g_DetourManager.Add_Detour(target, pFuncObj, conv);

		g_HookList.AddToTail(pDetour);
	}

	// ------------------------------------
	// Check to see if the detour already
	// has a callback manager.
	// ------------------------------------
	SourcemodManager* pMan = (SourcemodManager*)pDetour->GetManager("Sourcemod", (eHookType)params[5]);
	
	// ------------------------------------
	// If not, we need to create one and
	// add it to the detour.
	// ------------------------------------
	if(!pMan) {
		pMan = new SourcemodManager();
		pDetour->AddManager(pMan, (eHookType)params[5]);

		// Let's set our symbol name and previous created argument vector
		pMan->SetSymbolname(symbol);
		pMan->SetArgs(vecArgs);
		pMan->SetReturnType((SDKCommonDataType)params[4]);
	}

	// ------------------------------------
	// Now add the callback.
	// ------------------------------------
#ifdef DEBUG
	META_CONPRINTF("[MASTERHOOK] pMan->Add(%X, %d, %d\n", callback, conv, params[5]);
#endif
	pMan->Add(callback, conv, (eHookType)params[5]);

	// ------------------------------------
	// Done.
	// ------------------------------------
	return 1;
}

cell_t Native_Mh_UnhookFunction(IPluginContext *pContext, const cell_t *params) {

	char *symbol;
	pContext->LocalToString(params[1], &symbol);
	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	return g_DetourManager.Remove_Detour((void*)callback);
}

cell_t Native_Mh_SetReturnValue(IPluginContext *pContext, const cell_t *params) {

	if (!SMM_active) {
		return pContext->ThrowNativeError("Error: not in callback");
	}

	cell_t returnValue = params[1];

	SourcemodManager* pMan = SMM_active;

	void *ret;

	switch (pMan->GetReturnType()) {
		case MhDataType_CBaseEntity: {
			ret = (void*)gamehelpers->ReferenceToEntity(returnValue);
			break;
		}
		case MhDataType_Edict: {
			ret = (void*)gameents->BaseEntityToEdict(gamehelpers->ReferenceToEntity(returnValue));
			break;
		}
		default: {
			ret = (void*)returnValue;
		}
	}

	pMan->SetReturnValue(ret);

	return 1;
}

cell_t Native_Mh_CallOriginal(IPluginContext *pContext, const cell_t *params) {
	cell_t number = params[1];
	return number * number;
}

cell_t Native_Mh_Call(IPluginContext *pContext, const cell_t *params) {

	void *handle	= NULL;
	void *target	= NULL;
	
	handle = GetLibraryHandle((ValveLibrary)params[2]);

	if (handle == NULL) {
		return pContext->ThrowNativeError("Can't get library handle");
	}

	char *symbol;
	pContext->LocalToString(params[1], &symbol);

	eCallConv conv = CONV_CDECL;

#if defined PLATFORM_POSIX
	int symType = -1;
	char args[2048];
	target = ResolveDemangledSymbol(handle, symbol, symType, args, 2048);
	dlclose(handle);
#endif

	if (target == NULL) {
		return pContext->ThrowNativeError("Couldn't find symbol \"%s\" in symboltable", symbol);
	}

#if defined PLATFORM_POSIX
	if (symType != STT_FUNC) {
		return pContext->ThrowNativeError("Symbol \"%s\" doesn't point to a function", symbol);
	}
#endif

	if (strstr(symbol, "::")) {
		conv = CONV_THISCALL;
	}

#ifdef DEBUG
	META_CONPRINTF("[MASTERHOOK] Target: 0x%X Symbol: %s\n", target, symbol);
#endif

	bool needs_extra;

	PassInfo retInfo;

	Masterhook_TypeConverter::ToBinParam(PASSFLAG_BYVAL, params[3], needs_extra);

	int numParams = params[0] - 3;

	PassInfo* paramInfo = new PassInfo[numParams];

	char args[2048];
	std::vector<ArgumentDescription> vecArgs;
	ParseDemangledSymbolArguments(args, NULL, &vecArgs);

	ICallWrapper *pWrapper = g_pBinTools->CreateCall(
					target,
					Masterhook_TypeConverter::ToSourcemodCallConvention(conv),
					&retInfo,
					&paramInfo,
					numParams
	);

	void *returnData;
	void *stack;
	pWrapper->Execute(stack, &returnData);
	pWrapper->Destroy();

	return 1;
}
