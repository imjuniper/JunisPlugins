// MIT License - Copyright (c) Juniper Bouchard

#include "PseudoTimelineLatentAction.h"

#include "Curves/CurveFloat.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PseudoTimelineLatentAction)


void FPseudoTimelineLatentAction::UpdateOperation(FLatentResponse &Response)
{
	// The exit and "Finished" pin get called on the next tick, so that the last "Update" pin execution receives the value of 1.0
	if (bShouldExit)
	{
		OutputPins = EPseudoTimelineOutputPins::Finished;
		Response.FinishAndTriggerIf(true, LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget);
		return;
	}
	
	ElapsedTime += Response.ElapsedTime();
	
	OutProgress = FMath::Clamp(ElapsedTime / TotalTime, 0.0f, 1.0f);
	OutAlpha = Curve ? Curve->GetFloatValue(OutProgress) : OutProgress;
	bShouldExit = OutProgress == 1.0f;
	
	Response.TriggerLink(LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget);
}
