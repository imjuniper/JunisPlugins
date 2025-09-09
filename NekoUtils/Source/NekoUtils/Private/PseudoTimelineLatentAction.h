// MIT License - Copyright (c) Juniper Bouchard

#pragma once

#include "Engine/LatentActionManager.h"
#include "LatentActions.h"

#include "PseudoTimelineLatentAction.generated.h"

class UCurveFloat;


UENUM()
enum class EPseudoTimelineOutputPins : uint8
{
	Update,
	Finished
};

/**
 * Latent action that can sample a curve for a certain amount of time, making it similar to timelines in that way,
 * but can be used anywhere that can use latent nodes.
 *
 * Used by UNekoFunctionLibrary::PseudoTimeline and UNekoFunctionLibrary::RetriggerablePseudoTimeline
 *
 * @see https://www.youtube.com/watch?v=h_CX0ZitGXg -- Tutorial for Latent Actions (En)
 * @see https://www.youtube.com/watch?v=h0Q0Q7f68js -- Tutorial for Latent Actions (Fr)
 */
class FPseudoTimelineLatentAction final : public FPendingLatentAction
{
	// The current elapsed time
	float ElapsedTime = 0.0f;

	// Whether to exit on the next update
	bool bShouldExit = false;

public:
	///////////////////////////////////////////////////////////////////////////
	/// Input parameters

	// Information of the latent node (i.e. reference to the blueprint graph that called it)
	const FLatentActionInfo LatentInfo;

	// The time for which the pseudo-timeline will execute
	const float TotalTime = 0.0f;

	// The curve that will be sampled during the execution of the pseudo-timeline
	const UCurveFloat* Curve;

	///////////////////////////////////////////////////////////////////////////
	/// Output parameters (these are _references_, taken from the blueprint node, so it updates them correctly)

	// The pin to execute on the node
	EPseudoTimelineOutputPins& OutputPins;
	
	// The current progress along the curve, from 0 to 1
	float& OutProgress;
	
	// The currently sampled value on the curve, at the "OutProgress" time
	float& OutAlpha;

	FPseudoTimelineLatentAction(const FLatentActionInfo& InLatentInfo, const float InTime,
	                    const UCurveFloat* InCurve, EPseudoTimelineOutputPins& InOutputPins, float& InOutProgress,
	                    float& InOutAlpha)
		: LatentInfo(InLatentInfo),
		  TotalTime(InTime),
		  Curve(InCurve),
		  OutputPins(InOutputPins),
		  OutProgress(InOutProgress),
		  OutAlpha(InOutAlpha)
	{
		OutputPins = EPseudoTimelineOutputPins::Update;
		OutProgress = 0.0f;
		OutAlpha = 0.0f;
	}

	// The equivalent of tick for a latent action
	virtual void UpdateOperation(FLatentResponse& Response) override;

	// Resets the elapsed time. Used for retriggering the same pseudo-timeline
	void Reset() { ElapsedTime = 0.0f; }

#if WITH_EDITOR
	// Shows a description when debugging a blueprint in the editor
	virtual FString GetDescription() const override
	{
		return FString::Printf(TEXT("Time left: %.1f | Progress: %.1f | Alpha %.1f"), FMath::Max(TotalTime - ElapsedTime, 0.0f), OutProgress, OutAlpha);
	}
#endif
};
