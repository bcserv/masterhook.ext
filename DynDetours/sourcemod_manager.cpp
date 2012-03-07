// =======================================================================
// File: cpp_manager.cpp
// Purpose: Defines the CCPPManager callback manager. This class is
//	responsible for adding / creating callbacks for the C++ language.
// =======================================================================

// =======================================================================
// Includes
// =======================================================================
#include "sourcemod_manager.h"
#include "detour_class.h"
#include "detourman_class.h"
#include "natives.h"
#include "extension.h"
#include "rtti.h"



SourcemodManager* SMM_active;

// =======================================================================
// Adds a callback to the list.
// =======================================================================
void SourcemodManager::Add( IPluginFunction* pFuncObj, eCallConv conv, eHookType type )
{
	// ------------------------------------
	// Sanity checking.
	// ------------------------------------
	if( !pFuncObj ) {
		return;
	}

	// ------------------------------------
	// Add the callback to the proper list.
	// ------------------------------------
	switch( type ) 
	{
		case TYPE_PRE:
			m_vecPreCalls.push_back( pFuncObj );
			break;
			
		case TYPE_POST:
			m_vecPostCalls.push_back( pFuncObj );
			break;
	}
}

// =======================================================================
// Removes a callback from the list.
// =======================================================================
void SourcemodManager::Remove( IPluginFunction* pFuncObj, eHookType type )
{
	// ------------------------------------
	// Sanity checking.
	// ------------------------------------
	if( !pFuncObj ) {
		return;
	}

	// ------------------------------------
	// Find the proper callback list to
	// look in.
	// ------------------------------------
	std::vector<IPluginFunction*>* pList = NULL;
	switch( type )
	{
		case TYPE_PRE:
			pList = &m_vecPreCalls;
			break;

		case TYPE_POST:
			pList = &m_vecPostCalls;
			break;
	}

	// ------------------------------------
	// Using iterators, loop through the
	// list and delete the function.
	// ------------------------------------
	for( unsigned int i = 0; i < pList->size(); i++ )
	{
		// ------------------------------------
		// Compare the function item at this
		// index.
		// ------------------------------------
		if( (*pList)[i] == pFuncObj ) 
		{
			// ------------------------------------
			// If they match, remove it.
			// ------------------------------------
			pList->erase( pList->begin() + i );

			// ------------------------------------
			// Done.
			// ------------------------------------
			break;
		}
	}
}

// =======================================================================
// Removes a callback from the list.
// =======================================================================
void SourcemodManager::RemoveAllByContext(IPluginContext* plugincontext)
{

	if (!plugincontext) {
		return;
	}

	for(unsigned int i = 0; i < m_vecPreCalls.size(); i++) {

		if (m_vecPreCalls[i]->GetParentContext() == plugincontext)  {

			m_vecPreCalls.erase(m_vecPreCalls.begin() + i);

			break;
		}
	}
}

std::vector<IPluginFunction*>* SourcemodManager::GetPreCalls()
{
	return &this->m_vecPreCalls;
}

// =======================================================================
// Processes pre-callbacks.
// =======================================================================
HookRetBuf_t* SourcemodManager::DoPreCalls( CDetour* pDet )
{

	// ------------------------------------
	// Sanity checks.
	// ------------------------------------
	if( !pDet ) {
		return NULL;
	}

	// ------------------------------------
	// The temporary return value buffer.
	// ------------------------------------
	HookRetBuf_t* pFinalBuf = new HookRetBuf_t;

	// ------------------------------------
	// Set some initial values.
	// ------------------------------------
	pFinalBuf->eRes	  = HOOKRES_NONE;
	pFinalBuf->pRetBuf = NULL;

	// ------------------------------------
	// Stores the return information from
	// the callback.
	// ------------------------------------
	HookRetBuf_t* pTempBuf = new HookRetBuf_t;

	// ------------------------------------
	// Set some initial values.
	// ------------------------------------
	pTempBuf->eRes	  = HOOKRES_NONE;
	pTempBuf->pRetBuf = NULL;

	// Get function state variables
	CFuncObj* pFuncObj = pDet->GetFuncObj();

	// Make sure both are valid
	if (!pFuncObj)
	{
		return pFinalBuf;
	}

	// ------------------------------------
	// The calling convention contains
	// the register states which the
	// assembly will pull information
	// from.
	// ------------------------------------
	ICallConvention* pConv = pDet->GetAsmBridge()->GetConv();

	// ------------------------------------
	// Get the size of the return value.
	// ------------------------------------
	int iRetSize = pFuncObj->GetRetType()->GetSize();

	CFuncStack* stack = pDet->GetFuncObj()->GetStack();

	unsigned int addOffset = 4;

	if (pFuncObj->GetConvention() == CONV_THISCALL) {
		addOffset += 4;
	}

	unsigned int esp_args = (unsigned int)pDet->GetAsmBridge()->GetConv()->GetRegisters()->r_esp + addOffset;

#ifdef DEBUG
	META_CONPRINTF("[MASTERHOOK] Num Args: %d Stack-Size: %d\n", stack->GetNumArgs(), stack->GetStackSize());
#endif

	SMM_active = this;

	// ------------------------------------
	// Start executing all of the functions.
	// ------------------------------------
	for( unsigned int i = 0; i < m_vecPreCalls.size(); i++ )
	{

		// ------------------------------------
		// Get the callback.
		// ------------------------------------
		IPluginFunction* theCallback = m_vecPreCalls[i];

		this->classType = SDKDataType_CBaseEntity;
#ifdef DEBUG
		META_CONPRINTF("[MASTERHOOK] Calling callback (classType = %d)\n", this->classType);
#endif

		cell_t thisEntity = -1;
		if (pFuncObj->GetConvention() == CONV_THISCALL) {
			void* thisPtr = *(void**)(esp_args-4);

			IType *pType = GetType(thisPtr);
			IBaseType *pBaseType = pType->GetBaseType();
			if (IsClassDerivedFrom(pBaseType, "CBaseEntity")) {
				CBaseEntity* pEntity = (CBaseEntity*)thisPtr;
				thisEntity = gamehelpers->EntityToBCompatRef(pEntity);
#ifdef DEBUG
				META_CONPRINTF("[MASTERHOOK] Value: %d\n", thisEntity);
#endif
				theCallback->PushCellByRef(&thisEntity);
			}
			pType->Destroy();
		}

		for (unsigned int x = 0; x < args.size(); x++) {

			void* argPtr = (void*)(esp_args + (unsigned int)stack->GetArgument(x)->m_nOffset);

			ArgumentDescription* argDesc = &args.at(x);
#ifdef DEBUG
			META_CONPRINTF("Pushing argument %d Type: %d\n", x, argDesc->type);
#endif

			switch (argDesc->type) {
				case SDKDataType_Int:
				case SDKDataType_Bool: {
#ifdef DEBUG
					META_CONPRINTF("[MASTERHOOK] Int(%d): %d\n", x, *(int*)argPtr);
#endif
					if (argDesc->isConst) {
						theCallback->PushCell(*(int*)argPtr);
					}
					else {
						theCallback->PushCellByRef((cell_t*)argPtr, SM_PARAM_COPYBACK);
					}
					break;
				}
				case SDKDataType_Float: {
					if (argDesc->isConst) {
						theCallback->PushFloat(*(float*)argPtr);
					}
					else {
						theCallback->PushFloatByRef((float*)argPtr, SM_PARAM_COPYBACK);
					}
					break;
				}	
				case SDKDataType_String: {
#ifdef DEBUG
					META_CONPRINTF("[MASTERHOOK] String(%d): %s\n", x, *(const char**)argPtr);
#endif
					if (argDesc->isConst) {
						theCallback->PushString(*(const char**)(argPtr));
					}
					else {
						theCallback->PushStringEx(*(char**)(argPtr), strlen(*(char**)(argPtr)), SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
					}
					break;
				}
				case SDKDataType_CBaseEntity: {
					cell_t ref = gamehelpers->EntityToBCompatRef(*(CBaseEntity**)argPtr);
					theCallback->PushCellByRef(&ref);
					break;
				}
				case SDKDataType_Edict: {
					int entity = IndexOfEdict(*(edict_t**)argPtr);
					theCallback->PushCellByRef(&entity);
					break;
				}
				case SDKDataType_Vector: {
					Vector* vec = *(Vector**)argPtr;
					cell_t cellVec[3] = { sp_ftoc(vec->x), sp_ftoc(vec->y), sp_ftoc(vec->z) };
					theCallback->PushArray(cellVec, 3, SM_PARAM_COPYBACK);
					break;
				}
				case SDKDataType_QAngle: {
					QAngle* cellQAngle = *(QAngle**)argPtr;
					cell_t cellVec2[3] = { sp_ftoc(cellQAngle->x), sp_ftoc(cellQAngle->y), sp_ftoc(cellQAngle->z) };
					theCallback->PushArray(cellVec2, 3, SM_PARAM_COPYBACK);
					break;
				}
				case SDKDataType_CTakeDamageInfo: {

					CTakeDamageInfoHack* info = *(CTakeDamageInfoHack**)argPtr;

					theCallback->PushCell(info->GetAttacker());
					theCallback->PushCell(info->GetInflictor());
					theCallback->PushFloat(info->GetDamage());
					theCallback->PushCell(info->GetDamageType());
					theCallback->PushCell(info->GetAmmoType());
					break;
				}
			}
		}

		// ------------------------------------
		// Call the function.
		// ------------------------------------
		cell_t result;
		theCallback->Execute(&result);

		bool stop=false;
		switch (result) {

			case Pl_Changed: {
				pTempBuf->eRes = HOOKRES_NEWPARAMS;
				break;
			}
			case Pl_Handled: {
				pTempBuf->eRes = HOOKRES_OVERRIDE;
				break;
			}
			case Pl_Stop: {
				stop = true;
				pTempBuf->eRes = HOOKRES_OVERRIDE;
				break;
			}
			default: {
				pTempBuf->eRes = HOOKRES_NONE;
			}
		}

#ifdef DEBUG
		META_CONPRINTF("[MASTERHOOk] Return Value: %d\n", this->retValue);
#endif
		pTempBuf->pRetBuf = this->retValue;
		this->SetReturnValue(NULL);

		if (stop) {
			break;
		}

		// ------------------------------------
		// Prioritize the actions.
		// ------------------------------------
		if ( pFinalBuf->eRes <= pTempBuf->eRes ) {

			// ------------------------------------		
			// Copy the action and return value.
			// ------------------------------------
			pFinalBuf->eRes = pTempBuf->eRes;
			pFinalBuf->pRetBuf = pTempBuf->pRetBuf;
		}

		// ------------------------------------
		// Free the temp buffer.
		// ------------------------------------
		delete pTempBuf;
	}

	// ------------------------------------
	// Return the highest priority action.
	// ------------------------------------
	return pFinalBuf;
}

// =======================================================================
// Processes pre-callbacks.
// =======================================================================
HookRetBuf_t* SourcemodManager::DoPostCalls( CDetour* pDet )
{
	// ------------------------------------
	// Sanity checks.
	// ------------------------------------
	if( !pDet ) {
		return NULL;
	}

	// ------------------------------------
	// The temporary return value buffer.
	// ------------------------------------
	HookRetBuf_t* pFinalBuf = new HookRetBuf_t;

	// ------------------------------------
	// Set some initial values.
	// ------------------------------------
	pFinalBuf->eRes	  = HOOKRES_NONE;
	pFinalBuf->pRetBuf = NULL;

	// ------------------------------------
	// Get the size of the return value.
	// ------------------------------------
	int iRetSize = pDet->GetFuncObj()->GetRetType()->GetSize();

	// ------------------------------------
	// Start executing all of the functions.
	// ------------------------------------
	for( unsigned int i = 0; i < m_vecPostCalls.size(); i++ )
	{
		// ------------------------------------
		// Stores the return information from
		// the callback.
		// ------------------------------------
		HookRetBuf_t* pTempBuf = NULL;

		// ------------------------------------
		// Get the callback.
		// ------------------------------------
		SourcemodCallBack theCallback = (SourcemodCallBack)m_vecPostCalls[i];

		// ------------------------------------

		// Call the function.
		// ------------------------------------
		pTempBuf = theCallback( pDet );

		// ------------------------------------
		// Prioritize the actions.
		// ------------------------------------
		if ( pFinalBuf->eRes <= pTempBuf->eRes ) {

			// ------------------------------------		
			// Copy the action
			// ------------------------------------
			pFinalBuf->eRes = pTempBuf->eRes;

			// ------------------------------------
			// Copy the return value contents if
			// the function has a return value.
			// ------------------------------------
			if( iRetSize ) {
				memcpy( pFinalBuf->pRetBuf, pTempBuf->pRetBuf, iRetSize );
			}
		}

		// ------------------------------------
		// Free the temp buffer.
		// ------------------------------------
		free( pTempBuf );
	}

	// ------------------------------------
	// Return the highest priority action.
	// ------------------------------------
	return pFinalBuf;
}
