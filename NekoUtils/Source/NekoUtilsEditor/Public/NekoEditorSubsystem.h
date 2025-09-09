// MIT License - Copyright (c) Juniper Bouchard

#pragma once

#include "EditorSubsystem.h"

#include "NekoEditorSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorSelectionChangedSignature, const TArray<AActor*>&, NewSelection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorSelectedSignature, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDeselectedSignature, AActor*, Actor);


/**
 * Subsystem that provides events for actor selection change. The editor already provides a way to get selected actors,
 * but no way to know when the selection changes or if a new actor was selected.
 */
UCLASS(MinimalAPI)
class UNekoEditorSubsystem final : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

public:
	/**
	 * Called when the selection in the level editor is changed. Returns an array with the new Actor selection.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Editor Scripting | Actor Utility")
	FOnActorSelectionChangedSignature OnActorSelectionChanged;

	/**
	 * Called when an Actor is selected. Returns the Actor that was selected.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Editor Scripting | Actor Utility")
	FOnActorSelectedSignature OnActorSelected;

	/**
	 * Called when an Actor is deselected. Returns the Actor that was deselected.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Editor Scripting | Actor Utility")
	FOnActorDeselectedSignature OnActorDeselected;

	/**
	 * Find all Actors of the specified class in the current editor selection.
	 *
	 * @param ActorClass The class of Actor to find
	 * @param OutActors Found Actors of the specified class
	 */
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | Actor Utility", meta = (DeterminesOutputType = "ActorClass", DynamicOutputParam = "OutActors"))
	static void GetSelectedLevelActorsOfClass(const TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors);

private:
	UFUNCTION()
	void HandleLevelEditorActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh);

	UPROPERTY()
	TArray<AActor*> CurrentActorSelection;
};
