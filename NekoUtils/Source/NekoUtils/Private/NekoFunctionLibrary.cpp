// MIT License - Copyright (c) Juniper Bouchard

#include "NekoFunctionLibrary.h"

#include "NekoLogCategories.h"
#include "PseudoTimelineLatentAction.h"
#include "UI/NekoRootUILayout.h"
#include "UI/NekoUIManager.h"

#include "Blueprint/UserWidget.h"
#include "CommonInputSubsystem.h"
#include "Components/Widget.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "GeneralProjectSettings.h"
#include "Input/CommonUIActionRouterBase.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NekoFunctionLibrary)


namespace InternalNekoLibrary
{
	template <typename T>
	T ExponentialDecay(const T A, const T B, const float Decay, const float DeltaTime)
	{
		// See https://www.youtube.com/watch?v=LSNQuFEDOyQ
		return B + (A - B) * FMath::Exp(-Decay * DeltaTime);
	}

	// Taken from UGameplayTagsManager::AddChildrenTags(), which is a private function
	void AddChildrenTags(FGameplayTagContainer& TagContainer, TSharedPtr<FGameplayTagNode> GameplayTagNode, const bool bRecursive)
	{
		if (!GameplayTagNode.IsValid())
		{
			return;
		}

		TArray<TSharedPtr<FGameplayTagNode>>& ChildrenNodes = GameplayTagNode->GetChildTagNodes();
		for (TSharedPtr<FGameplayTagNode> ChildNode : ChildrenNodes)
		{
			if (!ChildNode.IsValid())
			{
				continue;
			}

			TagContainer.AddTag(ChildNode->GetCompleteTag());

			if (bRecursive)
			{
				AddChildrenTags(TagContainer, ChildNode, true);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Debugging and Dev Utils

void UNekoFunctionLibrary::CrashGame()
{
	UE_LOG(LogNekoUtils, Warning, TEXT("UNekoFunctionLibrary::CrashGame() called, please make sure this was not a mistake!"));
	int *Ptr = nullptr;
	*Ptr += 1;
}

bool UNekoFunctionLibrary::IsRunningInEditor(const ENekoEditorWorldType WorldType)
{
	return IsRunningInEditor_Pure(WorldType);
}

bool UNekoFunctionLibrary::IsRunningInEditor_Pure(const ENekoEditorWorldType WorldType)
{
#ifdef WITH_EDITOR
	if (const UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		switch (WorldType)
		{
			case ENekoEditorWorldType::PIE:
				return World->WorldType == EWorldType::PIE;

			case ENekoEditorWorldType::Standalone:
				return World->WorldType != EWorldType::PIE;

			case ENekoEditorWorldType::All:
			default:
				return true;
		}
	}
#endif
	return false;
}

bool UNekoFunctionLibrary::IsRunningInShipping()
{
	return IsRunningInShipping_Pure();
}

bool UNekoFunctionLibrary::IsRunningInShipping_Pure()
{
#ifdef UE_BUILD_SHIPPING
	return true;
#else
	return false;
#endif
}

FString UNekoFunctionLibrary::GetProjectVersion(const bool bWithBuildNumber)
{
	FString ProjectVersion = GetDefault<UGeneralProjectSettings>()->ProjectVersion;

#if WITH_EDITOR
	if (bWithBuildNumber)
	{
		ProjectVersion = FString::Format(TEXT("{0}+editor"), { ProjectVersion });
	}
#else 
	if (!bWithBuildNumber)
	{
		FString LeftSide;
		FString RightSide;
		ProjectVersion.Split("+", &LeftSide, &RightSide, ESearchCase::CaseSensitive);
		
		return LeftSide;
	}
#endif

	return ProjectVersion;
}

///////////////////////////////////////////////////////////////////////////////
/// Gameplay Tags

FGameplayTagContainer UNekoFunctionLibrary::GetGameplayTagChildren(const FGameplayTag GameplayTag, bool bRecursive)
{
	const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	FGameplayTagContainer TagContainer;
	// Note this purposefully does not include the passed in GameplayTag in the container.
	TSharedPtr<FGameplayTagNode> GameplayTagNode = TagsManager.FindTagNode(GameplayTag);
	if (GameplayTagNode.IsValid())
	{
		InternalNekoLibrary::AddChildrenTags(TagContainer, GameplayTagNode, bRecursive);
	}

	return TagContainer;
}

FGameplayTag UNekoFunctionLibrary::GetRandomGameplayTagFromContainer(const FGameplayTagContainer GameplayTagContainer)
{
	if (GameplayTagContainer.IsEmpty())
	{
		return FGameplayTag::EmptyTag;
	}
	
	return GameplayTagContainer.GetByIndex(FMath::RandRange(0, GameplayTagContainer.Num() - 1));
}

FName UNekoFunctionLibrary::GetTagLeafName(const FGameplayTag GameplayTag)
{
	// This function is taken straight from UE 5.6's FGameplayTag::GetTagLeafName()
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

///////////////////////////////////////////////////////////////////////////////
/// Timings and math

void UNekoFunctionLibrary::PseudoTimeline(const UObject* WorldContext, const FLatentActionInfo LatentInfo,
	const float Time, const UCurveFloat* Curve, EPseudoTimelineOutputPins& OutputPins, float& Progress, float& Alpha)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		UE_LOG(LogNekoUtils, Error, TEXT("Tried to execute a PseudoTimeline, but could not find World!"));
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	if (LatentActionManager.FindExistingAction<FPseudoTimelineLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
	{
		UE_LOG(LogNekoUtils, Warning, TEXT("Tried to re-execute a non-retriggerable PseudoTimeline that was already started"));
		return;
	}

	FPseudoTimelineLatentAction* Action = new FPseudoTimelineLatentAction(LatentInfo, Time, Curve, OutputPins, Progress, Alpha);
	LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
}

void UNekoFunctionLibrary::RetriggerablePseudoTimeline(const UObject* WorldContext, const FLatentActionInfo LatentInfo,
	const float Time, const UCurveFloat* Curve, EPseudoTimelineOutputPins& OutputPins, float& Progress, float& Alpha)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		UE_LOG(LogNekoUtils, Error, TEXT("Tried to execute a PseudoTimeline, but could not find World!"));
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	if (FPseudoTimelineLatentAction* Action = LatentActionManager.FindExistingAction<FPseudoTimelineLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
	{
		Action->Reset();
		return;
	}

	FPseudoTimelineLatentAction* Action = new FPseudoTimelineLatentAction(LatentInfo, Time, Curve, OutputPins, Progress, Alpha);
	LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
}

double UNekoFunctionLibrary::ExponentialDecay_Double(const double A, const double B, const float Decay, const float DeltaTime)
{
	return InternalNekoLibrary::ExponentialDecay(A, B, Decay, DeltaTime);
}

FVector UNekoFunctionLibrary::ExponentialDecay_Vector(const FVector A, const FVector B, const float Decay, const float DeltaTime)
{
	return InternalNekoLibrary::ExponentialDecay(A, B, Decay, DeltaTime);
}

double UNekoFunctionLibrary::GetInfinity_Double()
{
	return std::numeric_limits<double>::infinity();
}

///////////////////////////////////////////////////////////////////////////////
/// Assets

TSoftObjectPtr<UObject> UNekoFunctionLibrary::GetTypedSoftObjectReferenceFromPrimaryAssetId(const FPrimaryAssetId PrimaryAssetId,
	const TSubclassOf<UObject> ExpectedAssetType)
{
	if (UAssetManager* Manager = UAssetManager::GetIfInitialized())
	{
		FPrimaryAssetTypeInfo Info;
		if (Manager->GetPrimaryAssetTypeInfo(PrimaryAssetId.PrimaryAssetType, Info) && !Info.bHasBlueprintClasses)
		{
			if (UClass* AssetClass = Info.AssetBaseClassLoaded)
			{
				if ((ExpectedAssetType == nullptr) || !AssetClass->IsChildOf(ExpectedAssetType))
				{
					return nullptr;
				}
			}
			else
			{
				UE_LOG(LogNekoUtils, Warning, TEXT("GetTypedSoftObjectReferenceFromPrimaryAssetId(%s, %s) - AssetBaseClassLoaded was unset so we couldn't validate it, returning null"),
					*PrimaryAssetId.ToString(),
					*GetPathNameSafe(*ExpectedAssetType));
			}

			return TSoftObjectPtr<UObject>(Manager->GetPrimaryAssetPath(PrimaryAssetId));
		}
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
/// Inputs

void UNekoFunctionLibrary::SetCommonInputMode(APlayerController* PlayerController, ECommonInputMode InputMode, EMouseCaptureMode MouseMode, bool bHideCursorDuringViewportCapture)
{
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	check(LocalPlayer != nullptr);

	if (UCommonUIActionRouterBase* Subsystem = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>())
	{
		Subsystem->SetActiveUIInputConfig(FUIInputConfig(InputMode, MouseMode, bHideCursorDuringViewportCapture));
	}
}

void UNekoFunctionLibrary::SetCommonMouseMode(APlayerController* PlayerController, EMouseCaptureMode MouseMode, bool bHideCursorDuringViewportCapture)
{
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	check(LocalPlayer != nullptr);

	if (UCommonUIActionRouterBase* Subsystem = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>())
	{
		Subsystem->SetActiveUIInputConfig(FUIInputConfig(Subsystem->GetActiveInputMode(), MouseMode, bHideCursorDuringViewportCapture));
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Widgets and UI stuff

UWidget* UNekoFunctionLibrary::FindFocusedWidget(APlayerController* Player)
{
	for (TObjectIterator<UWidget> It; It; ++It)
	{
		if (It->HasUserFocus(Player))
		{
			return *It;
		}
	}
	return nullptr;
}

ECommonInputType UNekoFunctionLibrary::GetOwningPlayerInputType(const UUserWidget* Widget)
{
	if (Widget == nullptr) {
		return ECommonInputType::Count;
	}

	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(Widget->GetOwningLocalPlayer()))
	{
		return InputSubsystem->GetCurrentInputType();
	}

	return ECommonInputType::Count;
}

bool UNekoFunctionLibrary::IsOwningPlayerUsingGamepad(const UUserWidget* Widget)
{
	if (Widget == nullptr) {
		return false;
	}

	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(Widget->GetOwningLocalPlayer()))
	{
		return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
	}

	return false;
}

bool UNekoFunctionLibrary::IsOwningPlayerUsingTouch(const UUserWidget* Widget)
{
	if (Widget == nullptr) {
		return false;
	}

	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(Widget->GetOwningLocalPlayer()))
	{
		return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
	}

	return false;
}

UNekoRootUILayout* UNekoFunctionLibrary::GetRootUILayout_ForPlayer(const APlayerController* PlayerController)
{
	if (const ULocalPlayer* LP = PlayerController->GetLocalPlayer())
	{
		if (const UNekoUIManager* Subsystem = LP->GetSubsystem<UNekoUIManager>())
		{
			return Subsystem->GetRootUILayout();
		}
	}
	return nullptr;
}

UCommonActivatableWidget* UNekoFunctionLibrary::PushWidgetToLayer_ForPlayer(APlayerController* PlayerController,
	FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (UNekoRootUILayout* Layout = GetRootUILayout_ForPlayer(PlayerController))
	{
		return Layout->PushWidgetToLayerStack(LayerName, WidgetClass);
	}
	return nullptr;
}

void UNekoFunctionLibrary::PushWidgetInstanceToLayer_ForPlayer(APlayerController* PlayerController,
	FGameplayTag LayerName, UCommonActivatableWidget* Widget)
{
	if (UNekoRootUILayout* Layout = GetRootUILayout_ForPlayer(PlayerController))
	{
		Layout->PushWidgetInstanceToLayerStack(LayerName, Widget);
	}
}

void UNekoFunctionLibrary::PopWidgetFromLayer(UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		// Already destroyed
		return;
	}

	if (UNekoRootUILayout* Layout = GetRootUILayout_ForPlayer(Widget->GetOwningPlayer()))
	{
		Layout->FindAndRemoveWidgetFromLayer(Widget);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Miscellaneous

void UNekoFunctionLibrary::HideActorForPlayer(APlayerController* Player, AActor* Actor)
{
	if (Player && Actor)
	{
		Player->HiddenActors.Add(Actor);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Custom thunk functions

void UNekoFunctionLibrary::Map_Get(const TMap<int32, int32>& TargetMap, const int32 Index, int32& Key, int32& Value)
{
	checkNoEntry();
}

void UNekoFunctionLibrary::GenericMap_Get(void* TargetMap, const FMapProperty* MapProp, int32 Index, void* KeyPtr,
                                          void* ValuePtr)
{
	if (TargetMap)
	{
		FScriptMapHelper MapHelper(MapProp, TargetMap);

		FProperty* KeyProp = MapHelper.KeyProp;
		FProperty* ValueProp = MapHelper.ValueProp;

		if (MapHelper.IsValidIndex(Index))
		{
			KeyProp->CopyCompleteValueFromScriptVM(KeyPtr, MapHelper.GetKeyPtr(Index));
			ValueProp->CopyCompleteValueFromScriptVM(ValuePtr, MapHelper.GetValuePtr(Index));
		}
		else
		{
			FFrame::KismetExecutionMessage(*FString::Printf(TEXT("Attempted to access index %d from map '%s' of length %d in '%s'!"),
			                                                Index,
			                                                *MapProp->GetName(),
			                                                MapHelper.Num(),
			                                                *MapProp->GetOwnerVariant().GetPathName()),
			                               ELogVerbosity::Warning,
			                               FName("GetOutOfBoundsWarning"));
			KeyProp->InitializeValue(KeyPtr);
			ValueProp->InitializeValue(ValuePtr);
		}
	}
}
