// All Rights Reserved (c) Juniper Bouchard

#pragma once

#include "CommonUserWidget.h"
#include "NativeGameplayTags.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "NekoRootUILayout.generated.h"


namespace NekoUILayers
{
	NEKOUTILS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	NEKOUTILS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_GameMenu);
	NEKOUTILS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);
	NEKOUTILS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);
}

/**
 * The primary game UI layout of a game.
 *
 * @todo Add async versions of push to layer
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class NEKOUTILS_API UNekoRootUILayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Register a layer that widgets can be pushed onto. */
	UFUNCTION(BlueprintCallable, Category="Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InstanceInitFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InstanceInitFunc);
		}

		return nullptr;
	}

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Layer")
	UCommonActivatableWidget* PushWidgetToLayerStack(UPARAM(meta=(Categories="UI.Layer")) FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Layer")
	void PushWidgetInstanceToLayerStack(UPARAM(meta=(Categories="UI.Layer")) FGameplayTag LayerName, UCommonActivatableWidget* Widget);
	
	// Find the widget if it exists on any of the layers and remove it from the layer.
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// Get the layer widget for the given layer tag.
	UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName) const;

private:
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};
