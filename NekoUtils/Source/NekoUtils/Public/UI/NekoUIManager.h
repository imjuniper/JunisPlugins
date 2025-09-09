// All Rights Reserved (c) Juniper Bouchard

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "NekoUIManager.generated.h"

class UNekoRootUILayout;


UCLASS(Config = Game)
class NEKOUTILS_API UNekoUIManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	//~ULocalPlayerSubsystem interface
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	//~End of ULocalPlayerSubsystem interface

	UNekoRootUILayout* GetRootUILayout() const { return CurrentRootUILayout; }

protected:
	void CreateLayoutWidget();
	
protected:
	UPROPERTY(Transient)
	TObjectPtr<UNekoRootUILayout> CurrentRootUILayout = nullptr;

	UPROPERTY(Config)
	TSoftClassPtr<UNekoRootUILayout> DefaultRootUILayoutClass;
};
