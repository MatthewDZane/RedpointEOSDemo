#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraph.h"
#include "PlatformHttp.h"

EOS_ENABLE_STRICT_WARNINGS

class FGetJwtForCILogonNode : public FAuthenticationGraphNode
{
private:
	void OnCILogonTokenResponse(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully,
		TSharedRef<FAuthenticationGraphState> State,
		FAuthenticationGraphNodeOnDone OnDone);

	void OnCILogonUserInfoResponse(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully,
		TSharedRef<FAuthenticationGraphState> State,
		FAuthenticationGraphNodeOnDone OnDone);

	void OnCloudFlareHttpResponse(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully,
		TSharedRef<FAuthenticationGraphState> State,
		FAuthenticationGraphNodeOnDone OnDone);

public:
	UE_NONCOPYABLE(FGetJwtForCILogonNode);
	FGetJwtForCILogonNode() = default;
	virtual ~FGetJwtForCILogonNode() = default;

	virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

	virtual FString GetDebugName() const override
	{
		return TEXT("FGetJwtForCILogonNode");
	}
};

EOS_DISABLE_STRICT_WARNINGS

#endif // #if EOS_HAS_AUTHENTICATION

