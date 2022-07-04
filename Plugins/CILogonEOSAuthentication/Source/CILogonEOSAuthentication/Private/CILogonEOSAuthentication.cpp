// Copyright Epic Games, Inc. All Rights Reserved.

#include "CILogonEOSAuthentication.h"

#include "CILogonAuthenticationGraph.h"
#include "CILogonCrossPlatformAccountProvider.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraphRegistry.h"

#define LOCTEXT_NAMESPACE "FCILogonEOSAuthenticationModule"

DEFINE_LOG_CATEGORY(LogCILogon);

void FCILogonEOSAuthenticationModule::StartupModule()
{
	FCILogonAuthenticationGraph::Register();
	FAuthenticationGraphRegistry::RegisterCrossPlatformAccountProvider(
	CILOGON_CROSS_PLATFORM_ACCOUNT_ID,
	MakeShared<FCILogonCrossPlatformAccountProvider>());
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCILogonEOSAuthenticationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCILogonEOSAuthenticationModule, CILogonEOSAuthentication)