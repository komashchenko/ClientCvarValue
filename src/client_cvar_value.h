/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * ClientCvarValue
 * Written by Phoenix (˙·٠●Феникс●٠·˙) 2026.
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

#ifndef _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_

#include <ISmmPlugin.h>
#include "iclientcvarvalue.h"
#include <playerslot.h>
#include <networksystem/netmessage.h>
#include <netmessages.pb.h>
#include <string>
#include <array>

class CServerSideClient;

class ClientCvarValue final : public ISmmPlugin, public IMetamodListener, public IClientCvarValue
{
public:
	ClientCvarValue();

	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	void* OnMetamodQuery(const char* iface, int* ret);
	
private:
	const char* GetAuthor();
	const char* GetName();
	const char* GetDescription();
	const char* GetURL();
	const char* GetLicense();
	const char* GetVersion();
	const char* GetDate();
	const char* GetLogTag();

	KHook::Return<bool> OnProcessRespondCvarValue(CServerSideClient* pClient, const CNetMessagePB<CCLCMsg_RespondCvarValue>& msg);
	KHook::Return<void> OnClientConnected(ISource2GameClients*, CPlayerSlot nSlot, const char* pszName, uint64 xuid, const char* pszNetworkID, const char* pszAddress, bool bFakePlayer);
	KHook::Return<void> OnClientDisconnect(ISource2GameClients*, CPlayerSlot nSlot, ENetworkDisconnectionReason reason, const char* pszName, uint64 xuid, const char* pszNetworkID);

	int SendCvarValueQueryToClient(CPlayerSlot nSlot, const char* pszCvarName, int iQueryCvarCookieOverride = -1);

	KHook::Virtual<CServerSideClient, bool, const CNetMessagePB<CCLCMsg_RespondCvarValue>&> m_ProcessRespondCvarValueHook;
	KHook::Virtual<ISource2GameClients, void, CPlayerSlot, char const*, uint64, const char*, const char*, bool> m_OnClientConnectedHook;
	KHook::Virtual<ISource2GameClients, void, CPlayerSlot, ENetworkDisconnectionReason, const char*, uint64, const char*> m_ClientDisconnectHook;

private: // IClientCvarValue
	bool QueryCvarValue(CPlayerSlot nSlot, const char* pszCvarName, CvarValueCallback callback) override;
	const char* GetClientLanguage(CPlayerSlot nSlot) override;
	const char* GetClientOS(CPlayerSlot nSlot) override;

	struct ClientCvarData
	{
		ClientCvarData() = default;
		void Reset();

		std::unordered_map<int, CvarValueCallback> m_QueryCallback;
		std::string m_sLanguage;
		std::string m_sOperatingSystem;
	};

	std::array<ClientCvarData, 64> m_ClientCvarData;
};

#endif //_INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
