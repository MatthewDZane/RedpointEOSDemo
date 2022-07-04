// FNoopAuthenticationGraphNode.h

#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraph.h"

EOS_ENABLE_STRICT_WARNINGS

class FPerformOpenIdLoginForCrossPlatformCILNode : public FAuthenticationGraphNode
{
private:
	void HandleEOSAuthenticationCallback(
		const EOS_Connect_LoginCallbackInfo *Data,
		TSharedRef<FAuthenticationGraphState> State, 
		FAuthenticationGraphNodeOnDone OnDone);
public:
	UE_NONCOPYABLE(FPerformOpenIdLoginForCrossPlatformCILNode);
	FPerformOpenIdLoginForCrossPlatformCILNode() = default;
	virtual ~FPerformOpenIdLoginForCrossPlatformCILNode() = default;
	
	virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

	virtual FString GetDebugName() const override
	{
		return TEXT("FCustomAuthenticationGraphNode");
	}
};

EOS_ENABLE_STRICT_WARNINGS

#endif // #if EOS_HAS_AUTHENTICATION