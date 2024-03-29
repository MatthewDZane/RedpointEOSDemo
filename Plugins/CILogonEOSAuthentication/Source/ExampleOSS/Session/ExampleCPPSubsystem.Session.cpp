// Copyright June Rhodes. MIT Licensed.

#include "../ExampleCPPSubsystem.h"

#include "../ExampleCPPConfigLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "ExampleOSS/MultiplayerGameModeBase.h"

void UExampleCPPSubsystem::OnFindSessionForLeaderFollow(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FOnlineSessionSearchResult &SearchResult,
    FString SessionFollowString)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (bWasSuccessful)
    {
        // Join the session.
        this->JoinSessionForLeaderSessionName = FName(SessionFollowString);
        this->JoinSessionForLeaderFollowDelegateHandle =
            Session->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(
                this,
                &UExampleCPPSubsystem::OnJoinSessionForLeaderFollow));
        if (!Session->JoinSession(0, FName(SessionFollowString), SearchResult))
        {
            UE_LOG(LogTemp, Error, TEXT("Could not call JoinSession after following party leader!"));
        }
    }
}

void UExampleCPPSubsystem::OnJoinSessionForLeaderFollow(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (JoinResult == EOnJoinSessionCompleteResult::Success && SessionName.IsEqual(JoinSessionForLeaderSessionName))
    {
        this->PartyLeaderWasFollowedToSession.Broadcast(SessionName);

        Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionForLeaderFollowDelegateHandle);
    }
}

void UExampleCPPSubsystem::OnSessionUserInviteAccepted(
    const bool bWasSuccessful,
    const int32 LocalUserNum,
    TSharedPtr<const FUniqueNetId> UserId,
    const FOnlineSessionSearchResult &InviteResult)
{
    check(bWasSuccessful); // Should always be true in EOS subsystem.

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    this->JoinSessionForInviteSessionName = FName(*InviteResult.GetSessionIdStr());
    this->JoinSessionForInviteDelegateHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionComplete::FDelegate::CreateUObject(this, &UExampleCPPSubsystem::OnJoinSessionForInviteFollow));
    if (!Session->JoinSession(0, FName(*InviteResult.GetSessionIdStr()), InviteResult))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not call JoinSession after accepting invite!"));
    }
}

void UExampleCPPSubsystem::OnJoinSessionForInviteFollow(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (JoinResult == EOnJoinSessionCompleteResult::Success && SessionName.IsEqual(JoinSessionForInviteSessionName))
    {
        this->SessionJoinedMapTravelRequired.Broadcast(SessionName);

        Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionForInviteDelegateHandle);
    }
}

void UExampleCPPSubsystem::OnSessionJoinedViaOverlay(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    if (JoinResult == EOnJoinSessionCompleteResult::Success &&
        SessionName.ToString().StartsWith(TEXT("JoinViaOverlay_")))
    {
        this->SessionJoinedMapTravelRequired.Broadcast(SessionName);
    }
}

void UExampleCPPSubsystem::StartCreateSession(
    const UObject *WorldContextObject,
    int32 Slots,
    FExampleCPPSubsystemCreateSessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one CreateSession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->CreateSessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    TSharedRef<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
    SessionSettings->NumPublicConnections = Slots;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bUsesPresence = true;
    SessionSettings->Settings.Add(
        FName(TEXT("SessionSetting")),
        FOnlineSessionSetting(FString(TEXT("SettingValue")), EOnlineDataAdvertisementType::ViaOnlineService));
    bool IsDedicated = this->GetWorld()->GetNetMode() == NM_DedicatedServer;
    SessionSettings->Settings.Add(
        FName(TEXT("IsDedicatedServer")),
        FOnlineSessionSetting(IsDedicated, EOnlineDataAdvertisementType::ViaOnlineService));

    this->CreateSessionDelegateHandle =
        Session->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleCreateSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->CreateSession(0, FName(TEXT("MyLocalSessionName")), *SessionSettings))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleCreateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemCreateSessionComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnCreateSessionCompleteDelegate_Handle(this->CreateSessionDelegateHandle);
    this->CreateSessionDelegateHandle.Reset();
}

void UExampleCPPSubsystem::StartFindSessions(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFindSessionsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<UExampleCPPSessionSearchResult *>());
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one FindSessions process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->FindSessionsDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false, TArray<UExampleCPPSessionSearchResult *>());
        return;
    }

    TSharedRef<FOnlineSessionSearch> Search = MakeShared<FOnlineSessionSearch>();
    // Remove the default search parameters that FOnlineSessionSearch sets up.
    Search->MaxSearchResults = 100;
    Search->QuerySettings.SearchParams.Empty();
    Search->QuerySettings.Set(
        FName(TEXT("SessionSetting")),
        FString(TEXT("SettingValue")),
        EOnlineComparisonOp::Equals);

    this->FindSessionsDelegateHandle =
        Session->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleFindSessionsComplete,
            WorldContextObject,
            OnDone,
            Search));
    if (!Session->FindSessions(0, Search))
    {
        OnDone.ExecuteIfBound(false, TArray<UExampleCPPSessionSearchResult *>());
    }
}

void UExampleCPPSubsystem::HandleFindSessionsComplete(
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFindSessionsComplete OnDone,
    TSharedRef<FOnlineSessionSearch> Search)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    check(Session != nullptr);

    TArray<UExampleCPPSessionSearchResult *> Results;
    if (bWasSuccessful)
    {
        for (auto RawResult : Search->SearchResults)
        {
            if (RawResult.IsValid())
            {
                auto Result = NewObject<UExampleCPPSessionSearchResult>(this);
                Result->Result = RawResult;
                Result->SessionId = RawResult.GetSessionIdStr();
                Result->OpenSlots = RawResult.Session.NumOpenPublicConnections;
                Result->TotalSlots = RawResult.Session.SessionSettings.NumPublicConnections;
                auto DedicatedServerAttr = RawResult.Session.SessionSettings.Settings.Find("IsDedicatedServer");
                if (DedicatedServerAttr != nullptr)
                {
                    DedicatedServerAttr->Data.GetValue(Result->bIsDedicatedServer);
                }
                else
                {
                    Result->bIsDedicatedServer = false;
                }
                Session->GetResolvedConnectString(RawResult, NAME_GamePort, Result->ConnectionString);
                Results.Add(Result);
            }
        }
    }

    OnDone.ExecuteIfBound(bWasSuccessful, Results);

    Session->ClearOnFindSessionsCompleteDelegate_Handle(this->FindSessionsDelegateHandle);
    this->FindSessionsDelegateHandle.Reset();
}

FString UExampleCPPSubsystem::GetCurrentSessionId(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return TEXT("");
    }

    FNamedOnlineSession *NamedSession = Session->GetNamedSession(FName(TEXT("MyLocalSessionName")));
    if (NamedSession == nullptr || !NamedSession->SessionInfo.IsValid() || !NamedSession->SessionInfo->IsValid())
    {
        return TEXT("");
    }

    return NamedSession->GetSessionIdStr();
}

TArray<FString> UExampleCPPSubsystem::GetPlayersInSession(const UObject *WorldContextObject)
{
    TArray<FString> Results;

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return Results;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return Results;
    }

    FNamedOnlineSession *NamedSession = Session->GetNamedSession(FName(TEXT("MyLocalSessionName")));
    if (NamedSession == nullptr || !NamedSession->SessionInfo.IsValid() || !NamedSession->SessionInfo->IsValid())
    {
        return Results;
    }

    for (TSharedRef<const FUniqueNetId> PlayerId : NamedSession->RegisteredPlayers)
    {
        if (NamedSession->LocalOwnerId != nullptr && *PlayerId == *NamedSession->LocalOwnerId)
        {
            Results.Add(FString::Printf(TEXT("(host): %s"), *PlayerId->ToString()));
        }
        else
        {
            Results.Add(FString::Printf(TEXT("(client): %s"), *PlayerId->ToString()));
        }
    }

    return Results;
}

void UExampleCPPSubsystem::RegisterExistingPlayers(const UObject *WorldContextObject)
{
    AMultiplayerGameModeBase *GameMode = Cast<AMultiplayerGameModeBase>(GetWorld()->GetAuthGameMode());
    GameMode->RegisterExistingPlayers();
}

FUniqueNetIdRepl UExampleCPPSubsystem::RegisterPlayer(APlayerController *InPlayerController)
{
    check(IsValid(InPlayerController));

    if (InPlayerController->GetNetDriverName().IsEqual(NAME_DemoNetDriver))
    {
        return FUniqueNetIdRepl();
    }

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

    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    if (!UniqueNetId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("No unique net ID assocated with connection, can not register player"));
        return FUniqueNetIdRepl();
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    check(Session != nullptr);

    verify(Session->RegisterPlayer(FName(TEXT("MyLocalSessionName")), *UniqueNetId, false));
    return UniqueNetIdRepl;
}

void UExampleCPPSubsystem::UnregisterPlayer(APlayerController *InPlayerController, FUniqueNetIdRepl UniqueNetIdRepl)
{
    AMultiplayerGameModeBase *GameMode = Cast<AMultiplayerGameModeBase>(GetWorld()->GetAuthGameMode());
    GameMode->PreLogout(InPlayerController);
}

void UExampleCPPSubsystem::StartStartSession(
    const UObject *WorldContextObject,
    FName SessionName,
    FExampleCPPSubsystemStartSessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one StartSession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->StartSessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    this->StartSessionDelegateHandle =
        Session->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleStartSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->StartSession(SessionName))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleStartSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemStartSessionComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnStartSessionCompleteDelegate_Handle(this->StartSessionDelegateHandle);
    this->StartSessionDelegateHandle.Reset();
}

void UExampleCPPSubsystem::StartEndSession(
    const UObject *WorldContextObject,
    FName SessionName,
    FExampleCPPSubsystemEndSessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one EndSession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->EndSessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    this->EndSessionDelegateHandle =
        Session->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleEndSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->EndSession(SessionName))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleEndSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemEndSessionComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnEndSessionCompleteDelegate_Handle(this->EndSessionDelegateHandle);
    this->EndSessionDelegateHandle.Reset();
}

void UExampleCPPSubsystem::StartDestroySession(
    const UObject *WorldContextObject,
    FName SessionName,
    FExampleCPPSubsystemDestroySessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one DestroySession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->DestroySessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    this->DestroySessionDelegateHandle =
        Session->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleDestroySessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->DestroySession(SessionName))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleDestroySessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemDestroySessionComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionDelegateHandle);
    this->DestroySessionDelegateHandle.Reset();
}

void UExampleCPPSubsystem::StartJoinSession(
    const UObject *WorldContextObject,
    UExampleCPPSessionSearchResult *SearchResult,
    FExampleCPPSubsystemJoinSessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Subsystem is null"));
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one JoinSession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->JoinSessionDelegateHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JoinSessionDelegateHandle is not valid."));
        OnDone.ExecuteIfBound(false);
        return;
    }

    check(IsValid(SearchResult));

    this->JoinSessionDelegateHandle =
        Session->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleJoinSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->JoinSession(0, FName(TEXT("MyLocalSessionName")), SearchResult->Result))
    {
        UE_LOG(LogTemp, Error, TEXT("JoinSession failed."));
        ResultConnectInfo = "";
        OnDone.ExecuteIfBound(false);
    }
    else
    {
        ResultConnectInfo = SearchResult->ConnectionString;
    }
}

void UExampleCPPSubsystem::HandleJoinSessionComplete(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemJoinSessionComplete OnDone)
{
    if (!SessionName.IsEqual(FName(TEXT("MyLocalSessionName"))))
    {
        // Since we can conflict with the party leader following stuff... ugh the sessions API...
        UE_LOG(LogTemp, Error, TEXT("SessionName is different."));
        return;
    }

    OnDone.ExecuteIfBound(
        JoinResult == EOnJoinSessionCompleteResult::Success ||
        JoinResult == EOnJoinSessionCompleteResult::AlreadyInSession);

    if (JoinResult == EOnJoinSessionCompleteResult::Success ||
    JoinResult == EOnJoinSessionCompleteResult::AlreadyInSession)
    {
        if (GEngine != nullptr)
        {
            FURL NewURL(nullptr, *ResultConnectInfo, ETravelType::TRAVEL_Absolute);
            FString BrowseError;
            if (GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(this->GetWorld()), NewURL, BrowseError) ==
                EBrowseReturnVal::Failure)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to start browse: %s"), *BrowseError);
            }
        }
    }
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionDelegateHandle);
    this->JoinSessionDelegateHandle.Reset();
}

void UExampleCPPSubsystem::SendBeaconPingToSearchResult(
    const UObject *WorldContextObject,
    UExampleCPPSessionSearchResult *SearchResult,
    FExampleCPPSubsystemBeaconPingComplete OnDone)
{
    if (!UExampleCPPConfigLibrary::GetAreBeaconsSupported(WorldContextObject))
    {
        OnDone.ExecuteIfBound(false, TEXT("Beacons are not supported on the legacy networking stack"));
        return;
    }

    if (!IsValid(SearchResult))
    {
        OnDone.ExecuteIfBound(false, TEXT("Search result is not valid"));
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TEXT("No online subsystem"));
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // The port name is set to FName(TEXT("12345")) as per the comment in DemoBeacon.cpp.
    // Allow dedicated servers to specify an override for the beacon port as needed.
    FString BeaconPortString = TEXT("12345");
    FString OverrideBeaconPort;
    if (SearchResult->Result.Session.SessionSettings.Get("OverrideBeaconPort", OverrideBeaconPort))
    {
        BeaconPortString = OverrideBeaconPort;
    }

    FString ConnectInfo;
    if (!Session->GetResolvedConnectString(SearchResult->Result, FName(*BeaconPortString), ConnectInfo))
    {
        OnDone.ExecuteIfBound(false, TEXT("Connect info not found"));
        return;
    }

    auto Beacon = WorldContextObject->GetWorld()->SpawnActor<AOnlineBeacon>(AExampleCPPBeaconClient::StaticClass());
    if (Beacon)
    {
        Cast<AExampleCPPBeaconClient>(Beacon)->OnComplete = OnDone;
        FURL OldURL;
        FURL URL(&OldURL, *ConnectInfo, ETravelType::TRAVEL_Absolute);
        Cast<AExampleCPPBeaconClient>(Beacon)->InitClient(URL);
    }
}

FString UExampleCPPSubsystem::GetResolvedConnectString(const UObject *WorldContextObject)
{
    return GetResolvedConnectStringForSession(WorldContextObject, FName(TEXT("MyLocalSessionName")));
}

FString UExampleCPPSubsystem::GetResolvedConnectStringForSession(const UObject *WorldContextObject, FName SessionName)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    FString ConnectInfo;
    if (Session->GetResolvedConnectString(SessionName, ConnectInfo))
    {
        return ConnectInfo;
    }

    return TEXT("");
}

bool UExampleCPPSubsystem::InviteFriendToSession(
    const UObject *WorldContextObject,
    FName SessionName,
    FUniqueNetIdRepl FriendId)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return false;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    return Session->SendSessionInviteToFriend(0, SessionName, *FriendId.GetUniqueNetId());
}