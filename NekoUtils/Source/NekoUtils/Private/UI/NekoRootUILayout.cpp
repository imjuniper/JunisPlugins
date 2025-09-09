// All Rights Reserved (c) Juniper Bouchard

#include "UI/NekoRootUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NekoRootUILayout)


DEFINE_LOG_CATEGORY_STATIC(LogNekoRootUILayout, Log, All);

namespace NekoUILayers
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "UI.Layer.Game", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_GameMenu, "UI.Layer.GameMenu", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Menu, "UI.Layer.Menu", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Modal, "UI.Layer.Modal", "");
}

void UNekoRootUILayout::RegisterLayer(const FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerTag, LayerWidget);
	}
}

UCommonActivatableWidget* UNekoRootUILayout::PushWidgetToLayerStack(const FGameplayTag LayerName, const TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (WidgetClass.IsNull())
	{
		UE_LOG(LogNekoRootUILayout, Warning, TEXT("No WidgetClass was provided to PushWidgetToLayerStack"));
		return nullptr;
	}

	return PushWidgetToLayerStack(LayerName, WidgetClass.LoadSynchronous());
}

void UNekoRootUILayout::PushWidgetInstanceToLayerStack(const FGameplayTag LayerName, UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(LogNekoRootUILayout, Warning, TEXT("No Widget was provided to PushWidgetInstanceToLayerStack"));
		return;
	}

	if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
	{
		return Layer->AddWidgetInstance(*Widget);
	}
}

void UNekoRootUILayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	// We're not sure what layer the widget is on so go searching.
	for (const auto& Layer : Layers)
	{
		Layer.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UNekoRootUILayout::GetLayerWidget(const FGameplayTag LayerName) const
{
	return Layers.FindRef(LayerName);
}
