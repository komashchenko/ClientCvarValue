/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * ClientCvarValue
 * Copyright (C) 2024-2026 komashchenko (Phoenix)
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

#include "clientcvarvalue.h"
#include <networksystem/inetworkserializer.h>
#include <networksystem/inetworkmessages.h>
#include <dynlibutils/module.hpp>
#include <climits>
#include <memory>
#include <utility>

constexpr int CLIENTLANGUAGEID = INT_MAX;
constexpr int CLIENTOPERATINGSYSTEMID = INT_MAX - 1;

ClientCvarValue g_ClientCvarValue;
PLUGIN_EXPOSE(ClientCvarValue, g_ClientCvarValue);

ClientCvarValue::ClientCvarValue()
	: m_ProcessRespondCvarValueHook(&CServerSideClient::ProcessRespondCvarValue, this, nullptr, &ClientCvarValue::OnProcessRespondCvarValue),
	  m_OnClientConnectedHook(&ISource2GameClients::OnClientConnected, this, nullptr, &ClientCvarValue::OnClientConnected),
	  m_ClientDisconnectHook(&ISource2GameClients::ClientDisconnect, this, &ClientCvarValue::OnClientDisconnect, nullptr)
{
}

bool ClientCvarValue::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION)
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2GameClients, ISource2GameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

	void* pCServerSideClientVTable = DynLibUtils::CModule(g_pEngineServer).GetVirtualTableByName("CServerSideClient");
	if (!pCServerSideClientVTable)
	{
		V_strncpy(error, "Failed to find the CServerSideClient vtable.", maxlen);
		return false;
	}

	m_ProcessRespondCvarValueHook.AddGlobal(reinterpret_cast<CServerSideClient*>(&pCServerSideClientVTable));
	m_OnClientConnectedHook.Add(g_pSource2GameClients);
	m_ClientDisconnectHook.Add(g_pSource2GameClients);

	g_SMAPI->AddListener(this, this);

	return true;
}

bool ClientCvarValue::Unload(char* error, size_t maxlen)
{
	m_ClientDisconnectHook.Remove(g_pSource2GameClients);
	m_OnClientConnectedHook.Remove(g_pSource2GameClients);
	m_ProcessRespondCvarValueHook.ClearHooks();

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

KHook::Return<bool> ClientCvarValue::OnProcessRespondCvarValue(CServerSideClient* pClient, const CCLCMsg_RespondCvarValue_t& msg)
{
	CPlayerSlot nSlot = pClient->GetPlayerSlot();

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
				// The callback may issue more queries or disconnect the client.
				auto callback = std::move(it->second);
				queryCallback.erase(it);
				callback(nSlot, static_cast<ECvarValueStatus>(msg.status_code()), msg.name().c_str(), msg.value().c_str());
			}

			break;
		}
	}

	return {KHook::Action::Ignore, true};
}

KHook::Return<void> ClientCvarValue::OnClientConnected(ISource2GameClients*, CPlayerSlot nSlot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer)
{
	if (!bFakePlayer)
	{
		SendCvarValueQueryToClient(nSlot, "cl_language", CLIENTLANGUAGEID);
		SendCvarValueQueryToClient(nSlot, "engine_ostype", CLIENTOPERATINGSYSTEMID);
	}

	return {KHook::Action::Ignore};
}

KHook::Return<void> ClientCvarValue::OnClientDisconnect(ISource2GameClients*, CPlayerSlot nSlot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID)
{
	m_ClientCvarData[nSlot.Get()].Reset();

	return {KHook::Action::Ignore};
}

int ClientCvarValue::SendCvarValueQueryToClient(CPlayerSlot nSlot, const char* pszCvarName, int iQueryCvarCookieOverride)
{
	if (g_pEngineServer->GetPlayerNetInfo(nSlot))
	{
		static INetworkMessageInternal* pNetMsg = g_pNetworkMessages->FindNetworkMessage(CSVCMsg_GetCvarValue_t::sm_binding.GetName());
		static int iQueryCvarCookieCounter = 0;
		int iQueryCvarCookie = iQueryCvarCookieOverride == -1 ? ++iQueryCvarCookieCounter : iQueryCvarCookieOverride;

		std::unique_ptr<CSVCMsg_GetCvarValue_t> msg(pNetMsg->AllocateMessage()->As<CSVCMsg_GetCvarValue_t>());
		msg->set_cookie(iQueryCvarCookie);
		msg->set_cvar_name(pszCvarName);

		if(msg->Send(nSlot))
			return iQueryCvarCookie;
	}

	return -1;
}

bool ClientCvarValue::QueryCvarValue(CPlayerSlot nSlot, const char* pszCvarName, CvarValueCallback callback)
{
	if (pszCvarName && callback)
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
	return PLUGIN_VERSION;
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
	return "komashchenko (Phoenix)";
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
