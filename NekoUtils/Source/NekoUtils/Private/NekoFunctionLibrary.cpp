// MIT License - Copyright (c) Juniper Bouchard

#include "NekoFunctionLibrary.h"

#include "NekoLogCategories.h"
#include "PseudoTimelineLatentAction.h"

#include "Blueprint/UserWidget.h"
#include "CommonInputSubsystem.h"
#include "Components/Widget.h"
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

///////////////////////////////////////////////////////////////////////////////
/// Miscellaneous

void UNekoFunctionLibrary::HideActorForPlayer(APlayerController* Player, AActor* Actor)
{
	if (Player && Actor)
	{
		Player->HiddenActors.Add(Actor);
	}
}
