// MIT License - Copyright (c) Juniper Bouchard

#pragma once

#include "CommonInputModeTypes.h"
#include "CommonInputTypeEnum.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "NekoFunctionLibrary.generated.h"

struct FGameplayTag;
struct FGameplayTagContainer;
class UWidget;
enum class EPseudoTimelineOutputPins : uint8;


UENUM(BlueprintType)
enum class ENekoEditorWorldType : uint8 {
	PIE UMETA(DisplayName = "PIE"),
	Standalone,
	All
};


UCLASS()
class NEKOUTILS_API UNekoFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	///////////////////////////////////////////////////////////////////////////
	/// Debugging and Dev Utils

	/**
	 * Instantly crashes the game by dereferencing a null pointer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Development")
	static void CrashGame();

	/**
	 * Helper function to know whether we are running in editor.
	 *
	 * @param bIncludeStandalone Will also return true when running in Standalone
	 * @return Whether the world is running in PIE.
	 */
	UFUNCTION(BlueprintCallable, Category = "Development", meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool IsRunningInEditor(const ENekoEditorWorldType WorldType);

	/**
	 * Helper function to know whether we are running in editor.
	 *
	 * @param bIncludeStandalone Will also return true when running in Standalone
	 * @return Whether the world is running in PIE.
	 */
	UFUNCTION(BlueprintPure, Category = "Development", DisplayName = "Is Running in Editor (pure)")
	static bool IsRunningInEditor_Pure(const ENekoEditorWorldType WorldType);

	/**
	 * Helper function to know whether we are running in Shipping.
	 *
	 * @return Whether the world is running in Shipping.
	 */
	UFUNCTION(BlueprintCallable, Category = "Development", meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool IsRunningInShipping();

	/**
	 * Helper function to know whether we are running in Shipping.
	 *
	 * @return Whether the world is running in Shipping.
	 */
	UFUNCTION(BlueprintPure, Category = "Development", DisplayName = "Is Running in Shipping (pure)")
	static bool IsRunningInShipping_Pure();

	/**
	 * Does absolutely nothing. Useful to explicitly mark empty paths in Blueprints
	 *
	 * @see https://landelare.github.io/2022/04/30/uses-of-a-useless-node.html
	 */
	UFUNCTION(BlueprintCallable, Category = "Development", meta = (DevelopmentOnly, CompactNodeTitle = "Do Nothing"))
	static void DoNothing() {}

	/**
	 * Similar to do DoNothing, but will trigger a breakpoint if running with a debugger attached
	 * 
	 * @see https://landelare.github.io/2022/04/30/uses-of-a-useless-node.html
	 */
	UFUNCTION(BlueprintCallable, Category = "Development", meta = (DevelopmentOnly))
	static void ShouldNotHappen(const FString Message = TEXT("")) { ensureAlwaysMsgf(false, TEXT("ShouldNotHappen: %s"), *Message); }

	/**
	 * Gets the project version from the project settings
	 *
	 * Format: {Version}+{BuildNumber}
	 * Example: 0.1.0+1234
	 * In editor: 0.1.0+editor
	 *
	 * @param bWithBuildNumber Whether to include the build number (changelist number when building from TeamCity, and "editor" when running in editor)
	 * @return The project version
	 */
	UFUNCTION(BlueprintPure, Category = "Development")
	static FString GetProjectVersion(const bool bWithBuildNumber = true);

	///////////////////////////////////////////////////////////////////////////
	/// Gameplay Tags

	/**
	 * Get all the children of a gameplay tag.
	 * For example, calling this on x.y, which has "x.y.z" and "x.y.w" children would return
	 * a container with "x.y.z" and "x.y.w"
	 *
	 * @param GameplayTag The parent gameplay tag to search from
	 * @param bRecursive Whether to get the children of the children
	 */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	static FGameplayTagContainer GetGameplayTagChildren(const FGameplayTag GameplayTag, bool bRecursive);

	/**
	 * Get a random gameplay tag from the provided tag container
	 */
	UFUNCTION(BlueprintPure, Category = "Gameplay Tags")
	static FGameplayTag GetRandomGameplayTagFromContainer(const FGameplayTagContainer GameplayTagContainer);

	/**
	 * Parses the tag name and returns the name of the leaf.
	 * For example, calling this on x.y.z would return the z component.
	 *
	 * @note This is a native function in UE 5.6, FGameplayTag::GetTagLeafName()
	 */
	UFUNCTION(BlueprintPure, Category = "Gameplay Tags")
	static FName GetTagLeafName(const FGameplayTag GameplayTag);

	///////////////////////////////////////////////////////////////////////////
	/// Timings and math

	/**
	 * Samples from a curve, if provided. If none is provided, it will do a simple linear interpolation from 0 to 1.
	* 
	 * @param Progress The current progress along the curve, from 0 to 1
	 * @param Alpha The value of the curve at the current progress value (will be 0-1 if no curve is provided)
	 * @param Time How much time to run for
	 * @param Curve The curve to sample (optional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", Keywords = "lerp linear interpolate over time", ExpandEnumAsExecs = "OutputPins", Time = "1.0f"))
	static void PseudoTimeline(
		const UObject* WorldContextObject,
		const FLatentActionInfo LatentInfo,
		const float Time,
		const UCurveFloat* Curve,
		EPseudoTimelineOutputPins& OutputPins,
		float& Progress,
		float& Alpha);

	/**
	 * Samples from a curve, if provided. If none is provided, it will do a simple linear interpolation from 0 to 1.
	 *
	 * Calling it again will reset the progress to 0
	* 
	 * @param Progress The current progress along the curve, from 0 to 1
	 * @param Alpha The value of the curve at the current progress value (will be 0-1 if no curve is provided)
	 * @param Time How much time to run for
	 * @param Curve The curve to sample (optional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", Keywords = "lerp linear interpolate over time", ExpandEnumAsExecs = "OutputPins", Time = "1.0f"))
	static void RetriggerablePseudoTimeline(
		const UObject* WorldContextObject,
		const FLatentActionInfo LatentInfo,
		const float Time,
		const UCurveFloat* Curve,
		EPseudoTimelineOutputPins& OutputPins,
		float& Progress,
		float& Alpha);

	/**
	 * A lerp-like function that doesn't depend on framerate at all.
	 *
	 * See https://www.youtube.com/watch?v=LSNQuFEDOyQ
	 * 
	 * @note This actually returns a double, because floats in Blueprints actually default to double-precision since UE 5.0
	 * 
	 * @param Decay Decay constant. Approximately from 1 to 25, slow to fast.
	 */
	UFUNCTION(BlueprintPure, Category = "Math | Float", DisplayName = "Exponential Decay (Float)", meta = (Keywords = "lerp float", Decay = "16.0f"))
	static double ExponentialDecay_Double(const double A, const double B, const float Decay, const float DeltaTime);

	/**
	 * A lerp-like function that doesn't depend on framerate at all.
	 *
	 * See https://www.youtube.com/watch?v=LSNQuFEDOyQ
	 * 
	 * @param Decay Decay constant. Approximately from 1 to 25, slow to fast.
	 */
	UFUNCTION(BlueprintPure, Category = "Math | Vector", DisplayName = "Exponential Decay (Vector)", meta = (Keywords = "lerp", Decay = "16.0f"))
	static FVector ExponentialDecay_Vector(const FVector A, const FVector B, const float Decay, const float DeltaTime);

	// Note: there are no exponential decay for rotators and transforms, because they should be done with quaternions and I didn't want to deal with it.
	// This can be seen in UKismetMathLibrary::RLerp() and UKismetMathLibrary::TLerp()

	/**
	 * @note This actually returns a double, because floats in Blueprints default to double-precision since UE 5.0
	 *
	 * @return std::numeric_limits<double>::infinity()
	 */
	UFUNCTION(BlueprintPure, Category = "Math | Float", DisplayName = "Get Infinity (Float)", meta = (Keywords = "infinity infinite", CompactNodeTitle = "∞"))
	static double GetInfinity_Double();

	// Note: There are no other GetInfinity functions (for example GetInfinity_Int), because they always return 0 in Blueprints

	///////////////////////////////////////////////////////////////////////////
	/// Inputs

	/**
	 * Set the input mode for CommonInput.
	 * When using Common UI, this should be used over the "Set Input Mode X" nodes
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Input")
	static void SetCommonInputMode(APlayerController* PlayerController, ECommonInputMode InputMode, EMouseCaptureMode MouseMode, bool bHideCursorDuringViewportCapture = true);

	/**
	 * Set the mouse mode for CommonInput.
	 * Can be used to capture the mouse while keeping the same input mode
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Input")
	static void SetCommonMouseMode(APlayerController* PlayerController, EMouseCaptureMode MouseMode, bool bHideCursorDuringViewportCapture = true);

	///////////////////////////////////////////////////////////////////////////
	/// Widgets and UI stuff

	/**
	 * Finds the widget that is focused by the provided PlayerController, by looping through all widgets until it finds
	 * it.
	 * This is a slow operation, use with caution e.g. do not use every frame.
	 *
	 * @param Player The user's focus to look for.
	 * @return The widget focused by the provided user.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Widget")
	static UWidget* FindFocusedWidget(APlayerController* Player);

	/**
	 * Get the owning player's UCommonInputSubsystem's current input type
	 * 
	 * @note This requires the game to be using CommonInput/CommonUI
	 * @return The owning player's input type
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Widget", meta = (DefaultToSelf = "Widget"))
	static ECommonInputType GetOwningPlayerInputType(const UUserWidget* Widget);

	/**
	 * Check if the owning player's UCommonInputSubsystem's current input type is Gamepad
	 * 
	 * @note This requires the game to be using CommonInput/CommonUI
	 * @return Whether the owning player is using the gamepad
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Widget", meta = (DefaultToSelf = "Widget"))
	static bool IsOwningPlayerUsingGamepad(const UUserWidget* Widget);

	///////////////////////////////////////////////////////////////////////////
	/// Miscellaneous

	/**
	 * Hides an actor for a specific player. Useful for split-screen
	 *
	 * @param Player The player for which to hide the provided Actor
	 * @param Actor The Actor to be hidden
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	static void HideActorForPlayer(APlayerController* Player, AActor* Actor);
};
