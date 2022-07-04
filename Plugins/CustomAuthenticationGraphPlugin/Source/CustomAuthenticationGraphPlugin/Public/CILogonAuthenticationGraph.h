// AuthenticationGraphChangeThis.h

#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraph.h"

#define CILOGON_EOS_AUTH_GRAPH FName(TEXT("CILogonAuthGraph"))

class FCILogonAuthenticationGraph : public FAuthenticationGraph
{
protected:
	virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
		const TSharedRef<FAuthenticationGraphState> &InitialState) override;

public:
	static void Register();
};

#endif // #if EOS_HAS_AUTHENTICATION