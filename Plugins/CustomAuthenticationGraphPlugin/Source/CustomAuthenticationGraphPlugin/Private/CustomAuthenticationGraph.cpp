// AuthenticationGraphChangeThis.cpp

#include "CustomAuthenticationGraph.h"

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Graphs/AuthenticationGraphCrossPlatformOnly.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/FailAuthenticationNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/SelectCrossPlatformAccountNode.h"

#if EOS_HAS_AUTHENTICATION

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraphRegistry.h"

// Add the #include directives here for the nodes that you want to use in your graph.

TSharedRef<FAuthenticationGraphNode> FCustomAuthenticationGraph::CreateGraph(
	const TSharedRef<FAuthenticationGraphState> &InitialState)
{
	// Force the cross-platform provider to be Epic Games if we are using this provider.
	InitialState->CrossPlatformAccountProvider = MakeShared<FEpicGamesCrossPlatformAccountProvider>();
	
	// Implement the authentication graph here.
	if (InitialState->CrossPlatformAccountProvider.IsValid())
	{
		return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
			->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
			->Add(InitialState->CrossPlatformAccountProvider->GetInteractiveAuthenticationSequence())
			->Add(MakeShared<FSelectCrossPlatformAccountNode>())
			->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
	}
	else
	{
		return MakeShared<FFailAuthenticationNode>(TEXT("There is no cross-platform account provider configured."));
	}
}

void FCustomAuthenticationGraph::Register()
{
	FAuthenticationGraphRegistry::Register(
		CUSTOM_EOS_AUTH_GRAPH,
		NSLOCTEXT("YourGame", "CustomAuthGraph", "Custom Authentication Graph"),
		MakeShared<FAuthenticationGraphCrossPlatformOnly>());
}

#endif // #if EOS_HAS_AUTHENTICATION