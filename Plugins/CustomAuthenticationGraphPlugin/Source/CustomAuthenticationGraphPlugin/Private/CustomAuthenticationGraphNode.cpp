// FNoopAuthenticationGraphNode.cpp

#include "CustomAuthenticationGraphNode.h"

#if EOS_HAS_AUTHENTICATION

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/NoopAuthenticationGraphNode.h"

void FCustomAuthenticationGraphNode::Execute(
	TSharedRef<FAuthenticationGraphState> State,
	FAuthenticationGraphNodeOnDone OnDone)
{
	
	OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

#endif // #if EOS_HAS_AUTHENTICATION