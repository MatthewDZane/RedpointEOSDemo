// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerPlayerController.h"

#include "MyGameModeBase.h"

void AMultiplayerPlayerController::OnNetCleanup(UNetConnection *Connection)
{
	if (GetLocalRole() == ROLE_Authority && PlayerState != NULL)
	{
		AMyGameModeBase *GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->PreLogout(this);
		}
	}

	Super::OnNetCleanup(Connection);
}
