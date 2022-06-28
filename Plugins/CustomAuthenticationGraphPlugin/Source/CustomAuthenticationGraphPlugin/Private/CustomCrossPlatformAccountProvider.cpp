#if EOS_HAS_AUTHENTICATION

#include "CustomCrossPlatformAccountProvider.h"

#include "GetJwtForCustomNode.h"

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraph.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/CrossPlatform/SimpleFirstParty/PerformOpenIdLoginForCrossPlatformFPNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/FailAuthenticationNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/NoopAuthenticationGraphNode.h"

FCustomCrossPlatformAccountId::FCustomCrossPlatformAccountId(int64 InFirstPartyAccountId)
    : DataBytes(nullptr)
    , DataBytesSize(0)
    , FirstPartyAccountId(InFirstPartyAccountId)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(this->DataBytesSize);
    FMemory::Memcpy(this->DataBytes, Str.Get(), Str.Length());
}

bool FCustomCrossPlatformAccountId::Compare(const FCrossPlatformAccountId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == CUSTOM_CROSS_PLATFORM_ACCOUNT_ID)
    {
        const FCustomCrossPlatformAccountId &OtherId = (const FCustomCrossPlatformAccountId &)Other;
        return OtherId.GetFirstPartyAccountId() == this->GetFirstPartyAccountId();
    }

    return (GetType() == Other.GetType() && GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), GetSize()) == 0);
}

FCustomCrossPlatformAccountId::~FCustomCrossPlatformAccountId()
{
    FMemory::Free(this->DataBytes);
}

FName FCustomCrossPlatformAccountId::GetType() const
{
    return CUSTOM_CROSS_PLATFORM_ACCOUNT_ID;
}

const uint8 *FCustomCrossPlatformAccountId::GetBytes() const
{
    return this->DataBytes;
}

int32 FCustomCrossPlatformAccountId::GetSize() const
{
    return this->DataBytesSize;
}

bool FCustomCrossPlatformAccountId::IsValid() const
{
    return this->FirstPartyAccountId != 0;
}

FString FCustomCrossPlatformAccountId::ToString() const
{
    return FString::Printf(TEXT("%lld"), this->FirstPartyAccountId);
}

int64 FCustomCrossPlatformAccountId::GetFirstPartyAccountId() const
{
    return this->FirstPartyAccountId;
}

TSharedPtr<const FCrossPlatformAccountId> FCustomCrossPlatformAccountId::ParseFromString(const FString &In)
{
    return MakeShared<FCustomCrossPlatformAccountId>(FCString::Atoi64(*In));
}

FName FCustomCrossPlatformAccountProvider::GetName()
{
    return CUSTOM_CROSS_PLATFORM_ACCOUNT_ID;
}

TSharedPtr<const FCrossPlatformAccountId> FCustomCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    const FString &InStringRepresentation)
{
    return FCustomCrossPlatformAccountId::ParseFromString(InStringRepresentation);
}

TSharedPtr<const FCrossPlatformAccountId> FCustomCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    uint8 *InBytes,
    int32 InSize)
{
    FString Data = BytesToString(InBytes, InSize);
    return FCustomCrossPlatformAccountId::ParseFromString(Data);
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetInteractiveAuthenticationSequence()
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetJwtForCustomNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformFPNode>());
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetInteractiveOnlyAuthenticationSequence()
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetJwtForCustomNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformFPNode>());
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetNonInteractiveAuthenticationSequence(bool bOnlyUseExternalCredentials)
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetUpgradeCurrentAccountToCrossPlatformAccountSequence()
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence()
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetNonInteractiveDeauthenticationSequence()
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCustomCrossPlatformAccountProvider::
    GetAutomatedTestingAuthenticationSequence()
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    return MakeShared<FNoopAuthenticationGraphNode>();
#else
    return MakeShared<FFailAuthenticationNode>(
        TEXT("Automated testing authentication is not supported in shipping builds."));
#endif
}

#endif // #if EOS_HAS_AUTHENTICATION
