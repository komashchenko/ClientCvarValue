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

#ifndef CLIENTCVARVALUE_H
#define CLIENTCVARVALUE_H

#ifdef _WIN32
#pragma once
#endif

#include <playerslot.h>
#include <functional>

enum class ECvarValueStatus
{
	ValueIntact = 0,	// It got the value fine.
	CvarNotFound = 1,
	NotACvar = 2,		// There's a ConCommand, but it's not a ConVar.
	CvarProtected = 3	// The cvar was marked with FCVAR_SERVER_CAN_NOT_QUERY, so the server is not allowed to have its value.
};

using CvarValueCallback = std::function<void(CPlayerSlot nSlot, ECvarValueStatus eStatus, const char* pszCvarName, const char* pszCvarValue)>;

#define CLIENTCVARVALUE_INTERFACE	"ClientCvarValue001"

class IClientCvarValue
{
public:
	// Sends a query to get value of a cvar on the client.
	// Returns true if the query was successfully sent.
	virtual bool QueryCvarValue(CPlayerSlot nSlot, const char* pszCvarName, CvarValueCallback callback) = 0;

	// Returns nullptr if the language has not been received yet or if the nSlot is invalid.
	virtual const char* GetClientLanguage(CPlayerSlot nSlot) = 0;

	// Returns nullptr if the operating system has not yet been received or if the nSlot is invalid.
	virtual const char* GetClientOS(CPlayerSlot nSlot) = 0;
};

#endif // CLIENTCVARVALUE_H
