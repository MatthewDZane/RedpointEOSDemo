// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class EXAMPLEOSS_API AMultiplayerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	bool bAllExistingPlayersRegistered = false;
	
	void RegisterExistingPlayers();
	virtual void PostLogin(APlayerController* InPlayerController) override;
	

public:
	
	void PreLogout(APlayerController *InPlayerController);
};
