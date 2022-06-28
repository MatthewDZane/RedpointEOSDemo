﻿// AuthenticationGraphChangeThis.h

#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraph.h"

#define CUSTOM_EOS_AUTH_GRAPH FName(TEXT("CustomAuthGraph"))

class FCustomAuthenticationGraph : public FAuthenticationGraph
{
protected:
	virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
		const TSharedRef<FAuthenticationGraphState> &InitialState) override;

public:
	static void Register();
};

#endif // #if EOS_HAS_AUTHENTICATION