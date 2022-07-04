// AuthenticationGraphChangeThis.cpp

#include "CILogonAuthenticationGraph.h"

#include "GetJwtForCILogonNode.h"
#include "PerformOpenIdLoginForCrossPlatformCILNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Graphs/AuthenticationGraphCrossPlatformOnly.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/AuthenticationGraphNodeUntil_CrossPlatformAccountPresent.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/FailAuthenticationNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/SelectCrossPlatformAccountNode.h"

#if EOS_HAS_AUTHENTICATION

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraphRegistry.h"

// Add the #include directives here for the nodes that you want to use in your graph.

TSharedRef<FAuthenticationGraphNode> FCILogonAuthenticationGraph::CreateGraph(
	const TSharedRef<FAuthenticationGraphState> &InitialState)
{
	return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
	->Add(MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>()
	   ->Add(MakeShared<FGetJwtForCILogonNode>())
	   ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformCILNode>()));
}

void FCILogonAuthenticationGraph::Register()
{
	FAuthenticationGraphRegistry::Register(
		CILOGON_EOS_AUTH_GRAPH,
		NSLOCTEXT("YourGame", "CILogonAuthGraph", "CILogon Authentication Graph"),
		MakeShared<FAuthenticationGraphCrossPlatformOnly>());
}

#endif // #if EOS_HAS_AUTHENTICATION