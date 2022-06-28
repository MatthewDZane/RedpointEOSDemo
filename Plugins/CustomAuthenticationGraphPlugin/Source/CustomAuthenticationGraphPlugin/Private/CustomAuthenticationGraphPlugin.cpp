// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomAuthenticationGraphPlugin.h"

#include "CustomAuthenticationGraph.h"
#include "CustomCrossPlatformAccountProvider.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraphRegistry.h"

#define LOCTEXT_NAMESPACE "FCustomAuthenticationGraphPluginModule"

DEFINE_LOG_CATEGORY(LogCustom);

void FCustomAuthenticationGraphPluginModule::StartupModule()
{
	FCustomAuthenticationGraph::Register();
	FAuthenticationGraphRegistry::RegisterCrossPlatformAccountProvider(
	CUSTOM_CROSS_PLATFORM_ACCOUNT_ID,
	MakeShared<FCustomCrossPlatformAccountProvider>());
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCustomAuthenticationGraphPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomAuthenticationGraphPluginModule, CustomAuthenticationGraphPlugin)