#pragma once

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/CrossPlatform/EpicGames/GatherEASAccountsWithExternalCredentialsNode.h"

class CILOGONEOSAUTHENTICATION_API FCILogonAuthenticationGraphResolver
{
public:
	FName Resolve(TSharedRef<class FAuthenticationGraphRegistry> InRegistry,
				  TSharedRef<class FEOSConfig> InConfig,
				  FOnlineAccountCredentials InProvidedCredentials,
				  TSoftObjectPtr<UWorld> InWorld);
};
