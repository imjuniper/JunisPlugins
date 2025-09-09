// MIT License - Copyright (c) Juniper Bouchard

#include "NekoEditorSubsystem.h"

#include "EditorScriptingHelpers.h"
#include "LevelEditor.h"
#include "Selection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NekoEditorSubsystem)


namespace InternalNekoEditorSubsystemLibrary
{
	template <class T = AActor>
	bool IsEditorLevelActor(T* Actor)
	{
		if (Actor && IsValidChecked(Actor))
		{
			const UWorld* World = Actor->GetWorld();
			if (World && World->WorldType == EWorldType::Editor)
			{
				return true;
			}
		}
		return false;
	}
}

void UNekoEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditor.OnActorSelectionChanged().AddUObject(this, &ThisClass::HandleLevelEditorActorSelectionChanged);
}

void UNekoEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditor.OnActorSelectionChanged().RemoveAll(this);
}

void UNekoEditorSubsystem::GetSelectedLevelActorsOfClass(const TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors)
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	if (!EditorScriptingHelpers::CheckIfInEditorAndPIE())
	{
		return;
	}

	for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
	{
		AActor* Actor = Cast<AActor>(*Iter);
		if (!InternalNekoEditorSubsystemLibrary::IsEditorLevelActor(Actor))
		{
			return;
		}
		if (!Actor->GetClass()->IsChildOf(ActorClass))
		{
			return;
		}
		OutActors.Add(Actor);
	}
}

void UNekoEditorSubsystem::HandleLevelEditorActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh)
{
	TArray<AActor*> NewActorSelection;

	bool bActorsModified = false;

	// Loops through the selection, looking for Actors, and removing them from the current list
	for (UObject* Object : NewSelection)
	{
		if (AActor* Actor = Cast<AActor>(Object))
		{
			NewActorSelection.Add(Actor);

			// If the Actor wasn't already selected, broadcast the event
			if (CurrentActorSelection.Find(Actor) == INDEX_NONE)
			{
				bActorsModified = true;
				OnActorSelected.Broadcast(Actor);
			}
			else
			{
				CurrentActorSelection.Remove(Actor);
			}
		}
	}

	// If there are still Actors in the list, they are no longer selected so broadcast it
	if (CurrentActorSelection.Num() > 0)
	{
		bActorsModified = true;
		for (AActor* Actor : CurrentActorSelection)
		{
			OnActorDeselected.Broadcast(Actor);
		}
	}

	CurrentActorSelection = NewActorSelection;
	if (bActorsModified)
	{
		OnActorSelectionChanged.Broadcast(CurrentActorSelection);	
	}
}
