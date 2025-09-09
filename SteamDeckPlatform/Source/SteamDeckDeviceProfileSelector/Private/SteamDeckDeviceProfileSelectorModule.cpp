// MIT License - Copyright (c) Juniper Bouchard

#include "IDeviceProfileSelectorModule.h"
#include "Modules/ModuleManager.h"

class FSteamDeckDeviceProfileSelectorModule final : public IDeviceProfileSelectorModule
{
public:
	virtual const FString GetRuntimeDeviceProfileName() override
	{
		return TEXT("SteamDeck");
	}
};

IMPLEMENT_MODULE(FSteamDeckDeviceProfileSelectorModule, SteamDeckDeviceProfileSelector);
