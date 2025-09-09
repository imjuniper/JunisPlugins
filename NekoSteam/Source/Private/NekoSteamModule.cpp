// MIT License - Copyright (c) Juniper Bouchard

#include "Modules/ModuleManager.h"


class FNekoSteamModule final : public IModuleInterface
{
	virtual void StartupModule() override
	{
		FModuleManager::Get().LoadModuleChecked(TEXT("SteamShared"));
	}
};

IMPLEMENT_MODULE(FNekoSteamModule, NekoSteam)
