// All Rights Reserved (c) Juniper Bouchard

#pragma once

#include "CommonActivatableWidget.h"
#include "NekoActivatableWidget.generated.h"

struct FUIInputConfig;


UENUM(BlueprintType)
enum class ENekoWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

UCLASS()
class NEKOUTILS_API UNekoActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

protected:
	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	ENekoWidgetInputMode InputConfig = ENekoWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta=(EditCondition="InputConfig != ENekoWidgetInputMode::Menu"))
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

public:
#if WITH_EDITOR
	//~UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~End of UObject interface
	
	//~UUserWidget interface
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const override;
	//~End of UUserWidget interface
#endif
};
