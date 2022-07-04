#include "CILogonAuthenticationGraphResolver.h"

FName FCILogonAuthenticationGraphResolver::Resolve(
	TSharedRef<class FAuthenticationGraphRegistry> InRegistry,
	TSharedRef<class FEOSConfig> InConfig,
	FOnlineAccountCredentials InProvidedCredentials,
	TSoftObjectPtr<UWorld> InWorld)
{
	return FName(TEXT("NameOfAnotherAuthenticationGraph"));
}