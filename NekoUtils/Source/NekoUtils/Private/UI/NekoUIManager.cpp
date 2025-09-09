// All Rights Reserved (c) Juniper Bouchard

#include "UI/NekoUIManager.h"

#include "NekoLogCategories.h"
#include "UI/NekoRootUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NekoUIManager)


void UNekoUIManager::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	if (CurrentRootUILayout)
	{
		UE_LOG(LogNekoUtils, Log, TEXT("[%s] is refreshing the player context of player [%s]'s root layout [%s]"), *GetName(), *GetNameSafe(GetLocalPlayer()), *GetNameSafe(CurrentRootUILayout));
		CurrentRootUILayout->RemoveFromParent();
		CurrentRootUILayout->SetPlayerContext(FLocalPlayerContext(GetLocalPlayer()));
		CurrentRootUILayout->AddToPlayerScreen(1000);
	}
	else
	{
		CreateLayoutWidget();
	}
}

void UNekoUIManager::CreateLayoutWidget()
{
	auto* LocalPlayer = GetLocalPlayer();
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		if (DefaultRootUILayoutClass.IsNull())
		{
			return;
		}

		auto LayoutClass = DefaultRootUILayoutClass.LoadSynchronous();
		if (ensure(LayoutClass && !LayoutClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			CurrentRootUILayout = CreateWidget<UNekoRootUILayout>(PlayerController, LayoutClass);
			CurrentRootUILayout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
			CurrentRootUILayout->AddToPlayerScreen(1000);
			UE_LOG(LogNekoUtils, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(CurrentRootUILayout));
#if WITH_EDITOR
			if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
			{
				// So our controller will work in PIE without needing to click in the viewport
				FSlateApplication::Get().SetUserFocusToGameViewport(0);
			}
#endif
		}
	}
}
