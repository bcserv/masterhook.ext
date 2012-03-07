/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"
#include "compat_wrappers.h"
#include "natives.h"

#ifndef _WIN32
#include "symboltable.h"
#endif

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Masterhook g_Extension;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Extension);

CGlobalVars *g_pGlobals;
CUtlVector<CDetour*> g_HookList;

IBinTools *g_pBinTools = NULL;
void *g_pEntityFactoryDictAddr = NULL;
IEntityFactoryDictionary *g_pEntityFactoryDict = NULL;
IGameConfig *g_pGameConf = NULL;
IServerGameEnts *gameents = NULL;

/**
 * IEntityFactoryDictionary, IServerGameDLL & IVEngineServer Hooks
 */
SH_DECL_HOOK1(IEntityFactoryDictionary, Create, SH_NOATTRIB, 0, IServerNetworkable *, const char *);
SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, const char *, const char *, const char *, const char *, bool, bool);

HookRetBuf_t* callback_2( CDetour* pDet ) {

	CFuncObj* funcObj = pDet->GetFuncObj();
#ifdef DEBUG
	META_CONPRINTF("Debug1: %x\n", (unsigned int)funcObj->GetAddress());
#endif

	HookRetBuf_t* pBuf = new HookRetBuf_t;

	pBuf->eRes = HOOKRES_OVERRIDE;
	pBuf->pRetBuf = (void *)false;
	return pBuf;
}

class BaseAccessor : public IConCommandBaseAccessor {

public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase) {
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;


bool Masterhook::SDK_OnLoad(char *error, size_t maxlength, bool late) {

	sharesys->AddDependency(myself, "bintools.ext", true, true);
	sharesys->AddNatives(myself, g_Natives);

	playerhelpers->AddClientListener(&g_Extension);
	plsys->AddPluginsListener(&g_Extension);

#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
	ConVar_Register(0, &s_BaseAccessor);
#else
	ConCommandBaseMgr::OneTimeInit(&s_BaseAccessor);
#endif

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("masterhook.games", &g_pGameConf, conf_error, sizeof(conf_error))) {
		if(conf_error[0])
			snprintf(error, maxlength, "Could not read masterhook.games.txt: %s", conf_error);
		
		return false;
	}

#ifdef DEBUG
	META_CONPRINTF("[MASTERHOOK START]\n");
#endif

	if (!g_pGameConf->GetMemSig("IEntityFactoryDictionary", &g_pEntityFactoryDictAddr)) {
		snprintf(error, maxlength, "Failed to locate IEntityFactoryDictionary sig!");
		return false;
	}

	/*void *addr = NULL;
#ifndef _WIN32

	Dl_info info;
	// GNU only: returns 0 on error, inconsistent! >:[
	if (dladdr((const void*)g_SMAPI->GetServerFactory(false), &info) != 0) {
		void *handle = dlopen(info.dli_fname, RTLD_NOW);
		if (handle) {
			addr = ResolveSymbolPattern(handle, "CBaseTrigger::PassesTriggerFilters");
			dlclose(handle);
		} else {
			META_CONPRINTF("[SM] Unable to load library \"server\"\n");
		}
	}

	if (addr != NULL) {
		CPP_CreateCallback(addr, CONV_THISCALL, "v)v", &callback_2, TYPE_PRE);
	}
#endif*/

	SetupHooks();

	return true;
}

void Masterhook::SDK_OnAllLoaded()  {

	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);

	if (!g_pBinTools)
		return;

	SourceMod::PassInfo retData;
	retData.flags = PASSFLAG_BYVAL;
	retData.size = sizeof(void *);
	retData.type = PassType_Basic;

	ICallWrapper *pWrapper = g_pBinTools->CreateCall(g_pEntityFactoryDictAddr, CallConv_Cdecl, &retData, NULL, 0);
	void *returnData = NULL;
	pWrapper->Execute(NULL, &returnData);
	pWrapper->Destroy();
	if (!returnData)
	{
		g_pSM->LogError(myself, "Sig was loaded but NULL was returned...");
		return;
	}

	g_pEntityFactoryDict = (IEntityFactoryDictionary *)returnData;
	if (!g_pEntityFactoryDict)
	{
		g_pSM->LogError(myself, "couldn't recast IEntityFactoryDictionary...");
		return;
	}

	SH_ADD_HOOK_MEMFUNC(IEntityFactoryDictionary, Create, g_pEntityFactoryDict, &g_Extension, &Masterhook::Hook_Create, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, &g_Extension, &Masterhook::Hook_LevelInit, false);
}

void Masterhook::SDK_OnUnload() {

	SH_REMOVE_HOOK_MEMFUNC(IEntityFactoryDictionary, Create, g_pEntityFactoryDict, &g_Extension, &Masterhook::Hook_Create, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, &g_Extension, &Masterhook::Hook_LevelInit, false);

	gameconfs->CloseGameConfigFile(g_pGameConf);

	playerhelpers->RemoveClientListener(&g_Extension);
	plsys->RemovePluginsListener(&g_Extension);
}

bool Masterhook::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late) {

	GET_V_IFACE_CURRENT(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);

	g_pGlobals = ismm->GetCGlobals();

	return true;
}

void Masterhook::OnPluginUnloaded(IPlugin *plugin) {

	IPluginContext *plugincontext = plugin->GetBaseContext();

	for(int i = g_HookList.Count() - 1; i >= 0; i--) {

		SourcemodManager* pMan = (SourcemodManager*)g_HookList[i]->GetManager("Sourcemod", TYPE_PRE);
		
		pMan->RemoveAllByContext(plugincontext);
	}
}

void Masterhook::OnClientDisconnecting(int client) {

}

void Masterhook::SetupHooks() {

	//CPP_CreateCallback( &test_2, CONV_STDCALL, "iii)v", &callback_2, TYPE_PRE );
	//META_CONPRINTF("%d\n", test_2( 1, 2, 3 ));
}

/**
 * IEntityFactoryDictionary, IServerGameDLL & IVEngineServer Hook Handlers
 */
IServerNetworkable *Masterhook::Hook_Create(const char *pClassName) {

	IServerNetworkable *pNet = META_RESULT_ORIG_RET(IServerNetworkable *);
	if(!pNet)
		RETURN_META_VALUE(MRES_IGNORED, NULL);

	// Get entity edict and CBaseEntity
	edict_t *pEdict = pNet->GetEdict();
	CBaseEntity *pEnt = gameents->EdictToBaseEntity(pEdict);

	/*if (bCWeaponPhysCannon_CanPickupObject) {
		SH_ADD_MANUALHOOK_MEMFUNC(CWeaponPhysCannon_CanPickupObject, pEnt, &g_Extension, &Masterhook::Hook_CWeaponPhysCannon_CanPickupObject, false);
	}*/

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

bool Masterhook::Hook_LevelInit(char const *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background) {

	RETURN_META_VALUE(MRES_IGNORED, true);
}

CON_COMMAND(mh_list, "Lists all current Masterhook-Hooks")
{
	int iHookCount = g_HookList.Count();
	META_CONPRINT("[Masterhook] Active Hooks\n");
	META_CONPRINT("---------------------\n");
	if (iHookCount == 0)
	{
		META_CONPRINT("(No active hooks)\n");
		return;
	}

	IPlugin *pPlugin;

	for (int i = 0; i < iHookCount; i++)
	{
		SourcemodManager* pMan = (SourcemodManager*)g_HookList[i]->GetManager("Sourcemod", TYPE_PRE);
		std::vector<IPluginFunction*> *preCalls = pMan->GetPreCalls();

		META_CONPRINTF("[%d] Function: %s\n", i+1, pMan->GetName());

		for (unsigned int i = 0; i < preCalls->size(); i++) {
			(*preCalls)[i]->GetParentRuntime()->GetDefaultContext()->GetKey(2, (void **)&pPlugin);

			META_CONPRINTF("\t%s\n", pPlugin->GetFilename());
		}
	}
}
