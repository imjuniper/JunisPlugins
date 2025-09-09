// MIT License - Copyright (c) Juniper Bouchard

#include "NekoSteamSubsystem.h"

#include "Async/Async.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NekoSteamSubsystem)

UE_DEFINE_GAMEPLAY_TAG_STATIC(Steam_Achievement_Test, "Steam.Achievement.Test")
UE_DEFINE_GAMEPLAY_TAG_STATIC(Steam_Stat_Test, "Steam.Stat.Test")


DEFINE_LOG_CATEGORY_STATIC(LogNekoSteam, Log, All);

namespace InternalNekoSteamLibrary
{
	/**
	 * Parses the tag name and returns the name of the leaf.
	 * For example, calling this on x.y.z would return the z component.
	 *
	 * @note This is a native function in UE 5.6, FGameplayTag::GetTagLeafName()
	 */
	FName GetTagLeafName(const FGameplayTag GameplayTag)
	{
		FName RawTag = GameplayTag.GetTagName();
		if (RawTag.IsNone())
		{
			return RawTag;
		}

		const TStringBuilder<FName::StringBufferSize> TagBuffer(InPlace, RawTag);
		FStringView TagView = TagBuffer.ToView();
		const int32 DotIndex = UE::String::FindLastChar(TagView, TEXT('.'));
		if (DotIndex == INDEX_NONE)
		{
			return RawTag;
		}

		TagView.RightChopInline(DotIndex + 1);
		return FName(TagView);
	}
}

////////////////////////////////////////////////////////////////////////////////
///  FNekoSteamOverlayHelper

void FNekoSteamOverlayHelper::GameOverlayActivatedCallback(GameOverlayActivated_t* pParam)
{
	bGameOverlayActivated = static_cast<bool>(pParam->m_bActive);
	(void)OnGameOverlayActivated.ExecuteIfBound();
}


////////////////////////////////////////////////////////////////////////////////
///  FNekoSteamUserStatsHelper

void FNekoSteamUserStatsHelper::UserStatsReceivedCallback(UserStatsReceived_t* pParam)
{
	if (!bInitialStatsReceived)
	{
		bInitialStatsReceived = pParam->m_eResult == k_EResultOK;
	}
}


////////////////////////////////////////////////////////////////////////////////
///  UNekoSteamSubsystem

void UNekoSteamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	check(!bSubsystemInitialized);

	bSubsystemInitialized = true;

	if (!SteamAPI_Init())
	{
		UE_LOG(LogNekoSteam, Warning, TEXT("Failed to initialize Steam API, some subsystem functions will not work"));
		return;
	}

	bSteamInitialized = true;

	SteamUserStatsHelper = MakeUnique<FNekoSteamUserStatsHelper>();

	SteamOverlayHelper = MakeUnique<FNekoSteamOverlayHelper>();
	SteamOverlayHelper->OnGameOverlayActivated.BindUObject(this, &UNekoSteamSubsystem::BroadcastGameOverlayActivated);

	if (SteamUser()->BLoggedOn())
	{
		SteamUserStats()->RequestCurrentStats();
	}

	SetTickableTickType(ETickableTickType::Always);

	UE_LOG(LogNekoSteam, Display, TEXT("Initialized Steam API"));
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Blue, TEXT("Neko Steam Subsystem initialized"));
}

void UNekoSteamSubsystem::Deinitialize()
{
	check(bSubsystemInitialized);
	bSubsystemInitialized = false;
	
	SetTickableTickType(ETickableTickType::Never);
}

// No need to check if Steam is initialized, since the tick is disabled by default.
void UNekoSteamSubsystem::Tick(const float DeltaTime)
{
	// Ensure that the initial stats have been received. See Steam's API docs for more info
	if (!SteamUserStatsHelper->bInitialStatsReceived)
	{
		return;
	}

	if (!bShouldStoreStats)
	{
		return;
	}

	for (auto It = AchievementsToSet.CreateIterator(); It; ++It)
	{
		const FString AchievementID = It->ToString();
		if (SteamUserStats()->SetAchievement(TCHAR_TO_ANSI(*AchievementID)))
		{
			It.RemoveCurrent();
		}
	}

	for (auto It = StatsToSet.CreateIterator(); It; ++It)
	{
		const FString StatName = It.Key().ToString();
		if (SteamUserStats()->SetStat(TCHAR_TO_ANSI(*StatName), It.Value()))
		{
			It.RemoveCurrent();
		}
	}

	for (auto It = StatsToIncrement.CreateIterator(); It; ++It)
	{
		const FString StatName = It.Key().ToString();
		int32 CurrentValue;
		if (SteamUserStats()->GetStat(TCHAR_TO_ANSI(*StatName), &CurrentValue))
		{
			if (SteamUserStats()->SetStat(TCHAR_TO_ANSI(*StatName), CurrentValue + It.Value()))
			{
				It.RemoveCurrent();
			}	
		}
	}

	// Try to store stats again if some achievements or stats failed, or if they failed to be sent to the server
	bShouldStoreStats = !AchievementsToSet.IsEmpty() || !StatsToSet.IsEmpty() || !StatsToIncrement.IsEmpty() || !SteamUserStats()->StoreStats();
}

UWorld* UNekoSteamSubsystem::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}

////////////////////////////////////////////////////////////////////////////////
/// Steam Deck

bool UNekoSteamSubsystem::IsRunningOnSteamDeck()
{
	return IsRunningOnSteamDeck_Pure();
}

bool UNekoSteamSubsystem::IsRunningOnSteamDeck_Pure()
{
	return SteamUtils() && SteamUtils()->IsSteamRunningOnSteamDeck();
}

////////////////////////////////////////////////////////////////////////////////
/// Overlay

bool UNekoSteamSubsystem::IsOverlayActivated() const
{
	if (SteamOverlayHelper.IsValid())
	{
		return SteamOverlayHelper->bGameOverlayActivated;
	}

	return false;
}

bool UNekoSteamSubsystem::ActivateGameOverlayToURL(const FString URL)
{
	if (URL.IsEmpty())
	{
		return false;
	}

	if (bSteamInitialized && SteamUtils()->IsOverlayEnabled())
	{
		SteamFriends()->ActivateGameOverlayToWebPage(TCHAR_TO_ANSI(*URL), k_EActivateGameOverlayToWebPageMode_Default);
		return true;
	}

	if (FPlatformProcess::CanLaunchURL(*URL))
	{
		FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
		return true;
	}

	return false;
}

bool UNekoSteamSubsystem::ActivateGameOverlayToStore(const int32 AppID)
{
	if (AppID <= 0)
	{
		return false;
	}

	if (bSteamInitialized && SteamUtils()->IsOverlayEnabled())
	{
		SteamFriends()->ActivateGameOverlayToStore(AppID, k_EOverlayToStoreFlag_None);
		return true;
	}

	const FString URL = FString::Format(TEXT("https://store.steampowered.com/app/{0}"), { AppID });

	if (FPlatformProcess::CanLaunchURL(*URL))
	{
		FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// Achievements & Stats

void UNekoSteamSubsystem::UnlockAchievementByTag(const FGameplayTag AchievementTag)
{
	UnlockAchievement(InternalNekoSteamLibrary::GetTagLeafName(AchievementTag));
}

void UNekoSteamSubsystem::UnlockAchievement(const FName AchievementID)
{
	AchievementsToSet.AddUnique(AchievementID);
	bShouldStoreStats = true;
}

void UNekoSteamSubsystem::ProgressStatByTag(const FGameplayTag StatTag, const int32 Amount, const bool bIncrement)
{
	ProgressStat(InternalNekoSteamLibrary::GetTagLeafName(StatTag), Amount, bIncrement);
}

void UNekoSteamSubsystem::ProgressStat(const FName StatID, const int32 Amount, const bool bIncrement)
{
	if (bIncrement)
	{
		StatsToIncrement.Add(StatID, Amount);
	}
	else
	{
		StatsToSet.Add(StatID, Amount);
	}

	bShouldStoreStats = true;
}

////////////////////////////////////////////////////////////////////////////////
/// Internals

void UNekoSteamSubsystem::BroadcastGameOverlayActivated() const
{
	AsyncTask(ENamedThreads::GameThread, [this]
	{
		OnOverlayToggled.Broadcast(IsOverlayActivated());
	});
}
