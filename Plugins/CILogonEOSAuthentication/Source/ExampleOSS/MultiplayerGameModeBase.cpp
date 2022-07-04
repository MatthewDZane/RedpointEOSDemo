// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGameModeBase.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void AMultiplayerGameModeBase::RegisterExistingPlayers()
{
	for (auto It = this->GetWorld()->GetPlayerControllerIterator(); It; --It)
	{
		APlayerController* PlayerController = It->Get();

		check(IsValid(PlayerController));

		if (PlayerController->GetNetDriverName().IsEqual(NAME_DemoNetDriver))
		{
			continue;
		}
        
		FUniqueNetIdRepl UniqueNetIdRepl;
		if (PlayerController->IsLocalPlayerController())
		{
			ULocalPlayer *LocalPlayer = PlayerController->GetLocalPlayer();
			if (IsValid(LocalPlayer))
			{
				UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			else
			{
				UNetConnection *RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
				check(IsValid(RemoteNetConnection));
				UniqueNetIdRepl = RemoteNetConnection->PlayerId;
			}
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(PlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}

		// Get the unique player ID.
		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		check(UniqueNetId != nullptr);
		if (!UniqueNetId.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("No unique net ID assocated with connection, can not register player"));
			continue;
		}

		// Get the online session interface.
		IOnlineSubsystem *Subsystem = Online::GetSubsystem(PlayerController->GetWorld());
		check(Subsystem != nullptr);
		IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
		check(Session != nullptr);

		// Register the player with the "MyLocalSessionName" session; this name should match the name you provided in CreateSession.
		verify(Session->RegisterPlayer(FName(TEXT("MyLocalSessionName")), *UniqueNetId, false));
	}

	this->bAllExistingPlayersRegistered = true;
}

void AMultiplayerGameModeBase::PostLogin(APlayerController* InPlayerController)
{
	if (!this->bAllExistingPlayersRegistered)
	{
		// RegisterExistingPlayers has not run yet. When it does, it will register this incoming player
		// controller.
		Super::PostLogin(InPlayerController);
		return;
	}

	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	// Register the player with the "MyLocalSessionName" session; this name should match the name you provided in CreateSession.
	if (!Session->RegisterPlayer(FName(TEXT("MyLocalSessionName")), *UniqueNetId, false))
	{
		UE_LOG(LogTemp, Warning, TEXT("The player could not be registered; typically you will want to kick the player from the server in this situation."));
	}

	Super::PostLogin(InPlayerController);
}

void AMultiplayerGameModeBase::PreLogout(APlayerController *InPlayerController)
{
	
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);
	if (!UniqueNetId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("No unique net ID assocated with connection, can not unregister player"));
		return;
	}

	IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	check(Subsystem != nullptr);
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	check(Session != nullptr);

	verify(Session->UnregisterPlayer(FName(TEXT("MyLocalSessionName")), *UniqueNetId));
}