// FNoopAuthenticationGraphNode.h

#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraphNode.h"

class FCustomAuthenticationGraphNode : public FAuthenticationGraphNode
{
public:
	virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

	virtual FString GetDebugName() const override
	{
		return TEXT("FNoopAuthenticationGraphNode");
	}
};

#endif // #if EOS_HAS_AUTHENTICATION