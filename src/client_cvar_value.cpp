/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * ClientCvarValue
 * Written by Phoenix (˙·٠●Феникс●٠·˙) 2024.
 * ======================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 */

#include "client_cvar_value.h"
#include <networksystem/inetworkserializer.h>
#include <networksystem/inetworkmessages.h>
#include <inetchannel.h>
#include <igameeventsystem.h>
#include "utils.hpp"
#include <module.h>

constexpr int CLIENTLANGUAGEID = INT_MAX;
constexpr int CLIENTOPERATINGSYSTEMID = INT_MAX - 1;
constexpr int ProcessRespondCvarValueOffset = WIN_LINUX(39, 41);
constexpr int ClientSlotOffset = WIN_LINUX(64, 72);

ClientCvarValue g_ClientCvarValue;
PLUGIN_EXPOSE(ClientCvarValue, g_ClientCvarValue);

IGameEventSystem* g_pGameEventSystem = nullptr;

SH_DECL_MANUALHOOK1(OnProcessRespondCvarValue, ProcessRespondCvarValueOffset, 0, 0, bool, const CNetMessagePB<CCLCMsg_RespondCvarValue>&);
SH_DECL_HOOK6_void(ISource2GameClients, OnClientConnected, SH_NOATTRIB, 0, CPlayerSlot, char const*, uint64, const char*, const char*, bool);
SH_DECL_HOOK5_void(ISource2GameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, ENetworkDisconnectionReason, const char*, uint64, const char*);

bool ClientCvarValue::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION)
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2GameClients, ISource2GameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);

	void* pCServerSideClientVTable = DynLibUtils::CModule(g_pEngineServer).GetVirtualTableByName("CServerSideClient");
	m_iProcessRespondCvarValueID = SH_ADD_MANUALDVPHOOK(OnProcessRespondCvarValue, pCServerSideClientVTable, SH_MEMBER(this, &ClientCvarValue::OnProcessRespondCvarValue), true);
	SH_ADD_HOOK(ISource2GameClients, OnClientConnected, g_pSource2GameClients, SH_MEMBER(this, &ClientCvarValue::OnClientConnected), true);
	SH_ADD_HOOK(ISource2GameClients, ClientDisconnect, g_pSource2GameClients, SH_MEMBER(this, &ClientCvarValue::OnClientDisconnect), false);

	g_SMAPI->AddListener(this, this);

	return true;
}

bool ClientCvarValue::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(ISource2GameClients, ClientDisconnect, g_pSource2GameClients, SH_MEMBER(this, &ClientCvarValue::OnClientDisconnect), false);
	SH_REMOVE_HOOK(ISource2GameClients, OnClientConnected, g_pSource2GameClients, SH_MEMBER(this, &ClientCvarValue::OnClientConnected), true);
	SH_REMOVE_HOOK_ID(m_iProcessRespondCvarValueID);

	return true;
}

void* ClientCvarValue::OnMetamodQuery(const char* iface, int* ret)
{
	if (V_strcmp(iface, CLIENTCVARVALUE_INTERFACE) == 0)
	{
		if (ret)
			*ret = META_IFACE_OK;

		return static_cast<IClientCvarValue*>(this);
	}

	if (ret)
		*ret = META_IFACE_FAILED;

	return nullptr;
}

bool ClientCvarValue::OnProcessRespondCvarValue(const CNetMessagePB<CCLCMsg_RespondCvarValue>& msg)
{
	int nSlot = DynLibUtils::CMemory(META_IFACEPTR(void)).Offset(ClientSlotOffset).GetValue<int>();

	switch (msg.cookie())
	{
		case CLIENTLANGUAGEID:
		{
			m_ClientCvarData[nSlot].m_sLanguage = msg.value();

			break;
		}
		case CLIENTOPERATINGSYSTEMID:
		{
			m_ClientCvarData[nSlot].m_sOperatingSystem = msg.value();

			break;
		}

		default:
		{
			auto& queryCallback = m_ClientCvarData[nSlot].m_QueryCallback;
			auto it = queryCallback.find(msg.cookie());
			if (it != queryCallback.end())
			{
				it->second(nSlot, static_cast<ECvarValueStatus>(msg.status_code()), msg.name().c_str(), msg.value().c_str());
				queryCallback.erase(it);
			}

			break;
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}

void ClientCvarValue::OnClientConnected(CPlayerSlot nSlot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer)
{
	if (!bFakePlayer)
	{
		SendCvarValueQueryToClient(nSlot, "cl_language", CLIENTLANGUAGEID);
		SendCvarValueQueryToClient(nSlot, "engine_ostype", CLIENTOPERATINGSYSTEMID);
	}

	RETURN_META(MRES_IGNORED);
}

void ClientCvarValue::OnClientDisconnect(CPlayerSlot nSlot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID)
{
	m_ClientCvarData[nSlot.Get()].Reset();

	RETURN_META(MRES_IGNORED);
}

int ClientCvarValue::SendCvarValueQueryToClient(CPlayerSlot nSlot, const char* pszCvarName, int iQueryCvarCookieOverride)
{
	if (g_pEngineServer->GetPlayerNetInfo(nSlot))
	{
		static INetworkMessageInternal* pMsg = g_pNetworkMessages->FindNetworkMessagePartial("CSVCMsg_GetCvarValue");
		static int iQueryCvarCookieCounter = 0;
		int iQueryCvarCookie = iQueryCvarCookieOverride == -1 ? ++iQueryCvarCookieCounter : iQueryCvarCookieOverride;

		CNetMessagePB<CSVCMsg_GetCvarValue>* msg = pMsg->AllocateMessage()->ToPB<CSVCMsg_GetCvarValue>();
		msg->set_cookie(iQueryCvarCookie);
		msg->set_cvar_name(pszCvarName);

		uint64 clients = { 1llu << nSlot.Get() };
		g_pGameEventSystem->PostEventAbstract(-1, false, nSlot.Get() + 1, &clients, pMsg, msg, 0, BUF_RELIABLE);
		
		delete msg;

		return iQueryCvarCookie;
	}

	return -1;
}

bool ClientCvarValue::QueryCvarValue(CPlayerSlot nSlot, const char* pszCvarName, CvarValueCallback callback)
{
	if (pszCvarName)
	{
		int iQueryCvarCookie = SendCvarValueQueryToClient(nSlot, pszCvarName);
		if (iQueryCvarCookie != -1)
		{
			m_ClientCvarData[nSlot.Get()].m_QueryCallback[iQueryCvarCookie] = std::move(callback);

			return true;
		}
	}

	return false;
}

const char* ClientCvarValue::GetClientLanguage(CPlayerSlot nSlot)
{
	if (nSlot.Get() >= 0 && nSlot.Get() < m_ClientCvarData.size())
	{
		if (auto& sLanguage = m_ClientCvarData[nSlot.Get()].m_sLanguage; !sLanguage.empty())
			return sLanguage.c_str();
	}

	return nullptr;
}

const char* ClientCvarValue::GetClientOS(CPlayerSlot nSlot)
{
	if (nSlot.Get() >= 0 && nSlot.Get() < m_ClientCvarData.size())
	{
		if (auto& sOperatingSystem = m_ClientCvarData[nSlot.Get()].m_sOperatingSystem; !sOperatingSystem.empty())
			return sOperatingSystem.c_str();
	}

	return nullptr;
}

void ClientCvarValue::ClientCvarData::Reset()
{
	m_QueryCallback.clear();
	m_sLanguage.clear();
	m_sOperatingSystem.clear();
}

///////////////////////////////////////
const char* ClientCvarValue::GetLicense()
{
	return "GPL";
}

const char* ClientCvarValue::GetVersion()
{
	return "1.0.7";
}

const char* ClientCvarValue::GetDate()
{
	return __DATE__;
}

const char* ClientCvarValue::GetLogTag()
{
	return "ClientCvarValue";
}

const char* ClientCvarValue::GetAuthor()
{
	return u8"Phoenix (˙·٠●Феникс●٠·˙)";
}

const char* ClientCvarValue::GetDescription()
{
	return "API for query cvar value of players";
}

const char* ClientCvarValue::GetName()
{
	return "ClientCvarValue";
}

const char* ClientCvarValue::GetURL()
{
	return "https://github.com/komashchenko/ClientCvarValue";
}
