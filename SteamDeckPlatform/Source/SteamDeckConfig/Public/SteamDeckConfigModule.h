// MIT License - Copyright (c) Juniper Bouchard

#pragma once

#include "Modules/ModuleInterface.h"


class FSteamDeckConfigModule final : public IModuleInterface
{
public:
	//~Begin IModuleInterface interface
	virtual void StartupModule() override;
	//~End IModuleInterface interface

	STEAMDECKCONFIG_API static bool IsRunningOnSteamDeck();
	STEAMDECKCONFIG_API static bool IsRunningOnSteamDeck_EarlyInit();
};
