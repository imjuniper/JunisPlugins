// MIT License - Copyright (c) Juniper Bouchard

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "steam/steam_api.h"

#include "NekoSteamSubsystem.generated.h"

struct FGameplayTag;


/**
 * Small helper class that receives the overlay callbacks from Steam and stores its current state.
 */
class FNekoSteamOverlayHelper final
{
public:
	FNekoSteamOverlayHelper():
		m_CallbackGameOverlayActivated(this, &FNekoSteamOverlayHelper::GameOverlayActivatedCallback)
	{}
		
	STEAM_CALLBACK(FNekoSteamOverlayHelper, GameOverlayActivatedCallback, GameOverlayActivated_t, m_CallbackGameOverlayActivated);

	FSimpleDelegate OnGameOverlayActivated;

	bool bGameOverlayActivated = false;
};

/**
 * Small helper class that receives the user stats received callbacks from Steam and stores its current state.
 */
class FNekoSteamUserStatsHelper final
{
public:
	FNekoSteamUserStatsHelper():
		m_CallbackUserStatsReceived(this, &FNekoSteamUserStatsHelper::UserStatsReceivedCallback)
	{}
		
	STEAM_CALLBACK(FNekoSteamUserStatsHelper, UserStatsReceivedCallback, UserStatsReceived_t, m_CallbackUserStatsReceived);

	bool bInitialStatsReceived = false;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamOverlayActivated, bool, bOpen);

/**
 * Small subsystem relaying overlay events from Steam and providing achievement and stats functions.
 */
UCLASS()
class NEKOSTEAM_API UNekoSteamSubsystem final : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem interface

	// Begin FTickableGameObject interface
	virtual void Tick(const float DeltaTime) override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Never; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UNekoSteamSubsystem, STATGROUP_Tickables); }
	// End FTickableGameObject interface
	
	///////////////////////////////////////////////////////////////////////////
	/// Steam Deck

	/**
	 * Check if the game is running on Steam Deck
	 *
	 * @return Whether the game is running on Steam Deck
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam", meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool IsRunningOnSteamDeck();

	/**
	 * Check if the game is running on Steam Deck
	 *
	 * @return Whether the game is running on Steam Deck
	 */
	UFUNCTION(BlueprintPure, Category = "Steam", DisplayName = "Is Running on Steam Deck (pure)")
	static bool IsRunningOnSteamDeck_Pure();
	
	///////////////////////////////////////////////////////////////////////////
	/// Overlay

	/**
	 * Check if the Steam overlay is currently open
	 *
	 * @return Whether the overlay is currently open
	 */
	UFUNCTION(BlueprintPure, Category = "Steam")
	bool IsOverlayActivated() const;

	/**
	 * Opens the Steam Overlay to a specific URL
	 *
	 * Will try to open in the OS' default browser if the user has the Steam overlay disabled
	 * 
	 * @param URL The URL to open
	 * @return True if the page was opened
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	bool ActivateGameOverlayToURL(const FString URL);

	/**
	 * Opens the Steam Overlay to the store page of the specified App ID
	 *
	 * Will try to open in the OS' default browser if the user has the Steam overlay disabled
	 *
	 * @param AppID The App ID of the store page to open
	 * @return True if the page was opened
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	bool ActivateGameOverlayToStore(const int32 AppID = 0);
	
	///////////////////////////////////////////////////////////////////////////
	/// Achievements & Stats

	/**
	 * Unlocks the specified achievement using the last leaf of a Gameplay Tag
	 *
	 * @param AchievementTag The achievement to unlock
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void UnlockAchievementByTag(UPARAM(meta = (Categories = "Steam.Achievement")) const FGameplayTag AchievementTag);

	/**
	 * Unlocks the specified achievement
	 * 
	 * @param AchievementID The achievement to unlock
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void UnlockAchievement(const FName AchievementID);

	/**
	 * Progresses the specified stat using the last leaf of a Gameplay Tag.
	 * If Increment is checked, will add to the current stat instead of replacing it.
	 * 
	 * @param StatTag The stat to progress
	 * @param Amount The amount to set or increment by
	 * @param bIncrement Whether to increment the stat or replace the current value
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void ProgressStatByTag(UPARAM(meta = (Categories = "Steam.Stat")) const FGameplayTag StatTag, const int32 Amount, const bool bIncrement);

	/**
	 * Progresses the specified stat.
	 * If Increment is checked, will add to the current stat instead of replacing it.
	 * 
	 * @param StatID The stat to progress
	 * @param Amount The amount to set or increment by
	 * @param bIncrement Whether to increment the stat or replace the current value
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void ProgressStat(const FName StatID, const int32 Amount, const bool bIncrement);

public:
	/**
	 * Called when the overlay opens and closes. Returns the current overlay state
	 */
	UPROPERTY(BlueprintAssignable, Category = "Steam")
	FOnSteamOverlayActivated OnOverlayToggled;

private:
	void BroadcastGameOverlayActivated() const;

private:
	UPROPERTY()
	bool bSubsystemInitialized = false;

	UPROPERTY()
	bool bSteamInitialized = false;

	// Whether stats and/or achievements should be sent to Steam on the next tick.
	UPROPERTY()
	bool bShouldStoreStats = false;

	// The stats to set on the next tick.
	UPROPERTY()
	TMap<FName, int32> StatsToSet;

	// The stats to increment on the next tick.
	UPROPERTY()
	TMap<FName, int32> StatsToIncrement;

	// The achievements to set on the next tick.
	UPROPERTY()
	TArray<FName> AchievementsToSet;

	// Object handling the overlay callbacks from Steam
	TUniquePtr<FNekoSteamOverlayHelper> SteamOverlayHelper;

	// Object handling some user stats callbacks from Steam
	TUniquePtr<FNekoSteamUserStatsHelper> SteamUserStatsHelper;
};
