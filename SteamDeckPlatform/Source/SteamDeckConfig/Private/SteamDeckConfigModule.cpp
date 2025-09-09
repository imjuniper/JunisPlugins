// MIT License - Copyright (c) Juniper Bouchard

#include "SteamDeckConfigModule.h"

#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "SteamSharedModule.h"
#include "steam/isteamutils.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamDeckConfig, Log, All);


void FSteamDeckConfigModule::StartupModule()
{
	if (IsRunningOnSteamDeck_EarlyInit())
	{
		UE_LOG(LogSteamDeckConfig, Log, TEXT("Adding SteamDeck Dynamic layers to config branches"));
		
		const auto AddLayer = [](const FString ConfigFile)
		{
			if (FConfigBranch* FoundBranch = GConfig->FindBranch(*ConfigFile, FString()))
			{
				FoundBranch->AddDynamicLayerToHierarchy(FPaths::Combine(FPaths::ProjectConfigDir(), "SteamDeck", "SteamDeck" + ConfigFile + ".ini"));
			}
		};

		#define ADD_INI_LAYER(IniName) AddLayer(#IniName);
			// AddLayer("Engine")
			ENUMERATE_KNOWN_INI_FILES(ADD_INI_LAYER);
		#undef ADD_INI_LAYER
	}
}

bool FSteamDeckConfigModule::IsRunningOnSteamDeck()
{
	if (!FSteamSharedModule::IsAvailable())
	{
		FSteamSharedModule::Get();
	}

	if (FSteamSharedModule::IsAvailable() && SteamUtils())
	{
		return SteamUtils()->IsSteamRunningOnSteamDeck();
	}

	return IsRunningOnSteamDeck_EarlyInit();
}

bool FSteamDeckConfigModule::IsRunningOnSteamDeck_EarlyInit()
{
	return FPlatformMisc::GetEnvironmentVariable(TEXT("SteamDeck")).Equals(FString(TEXT("1")));
}

IMPLEMENT_MODULE(FSteamDeckConfigModule, SteamDeckConfig)
