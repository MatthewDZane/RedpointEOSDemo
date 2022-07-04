
#if EOS_HAS_AUTHENTICATION

#include "CILogonCrossPlatformAccountProvider.h"

#include "GetJwtForCILogonNode.h"

#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationGraph.h"
#include "PerformOpenIdLoginForCrossPlatformCILNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/FailAuthenticationNode.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/Nodes/NoopAuthenticationGraphNode.h"

FCILogonCrossPlatformAccountId::FCILogonCrossPlatformAccountId(EOS_ProductUserId InFirstPartyAccountId)
    : DataBytes(nullptr)
    , DataBytesSize(0)
    , FirstPartyAccountId(InFirstPartyAccountId)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(this->DataBytesSize);
    FMemory::Memcpy(this->DataBytes, Str.Get(), Str.Length());
}

bool FCILogonCrossPlatformAccountId::Compare(const FCrossPlatformAccountId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == CILOGON_CROSS_PLATFORM_ACCOUNT_ID)
    {
        const FCILogonCrossPlatformAccountId &OtherId = (const FCILogonCrossPlatformAccountId &)Other;
        return OtherId.GetFirstPartyAccountId() == this->GetFirstPartyAccountId();
    }

    return (GetType() == Other.GetType() && GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), GetSize()) == 0);
}

FCILogonCrossPlatformAccountId::~FCILogonCrossPlatformAccountId()
{
    FMemory::Free(this->DataBytes);
}

FName FCILogonCrossPlatformAccountId::GetType() const
{
    return CILOGON_CROSS_PLATFORM_ACCOUNT_ID;
}

const uint8 *FCILogonCrossPlatformAccountId::GetBytes() const
{
    return this->DataBytes;
}

int32 FCILogonCrossPlatformAccountId::GetSize() const
{
    return this->DataBytesSize;
}

bool FCILogonCrossPlatformAccountId::IsValid() const
{
    return this->FirstPartyAccountId != 0;
}

FString FCILogonCrossPlatformAccountId::ToString() const
{
    return FString::Printf(TEXT("%lld"), this->FirstPartyAccountId);
}

FString FCILogonCrossPlatformAccountId::GetFirstPartyAccountId() const
{
    if (EOSString_ProductUserId::IsNone(this->FirstPartyAccountId))
    {
        return TEXT("");
    }

    FString Str;
    if (EOSString_ProductUserId::ToString(this->FirstPartyAccountId, Str) == EOS_EResult::EOS_Success)
    {
        return Str;
    }

    return TEXT("");
}

TSharedPtr<const FCrossPlatformAccountId> FCILogonCrossPlatformAccountId::ParseFromString(const FString &In)
{
    EOS_ProductUserId ProductUserId = nullptr;
    if (EOSString_ProductUserId::FromString(In, ProductUserId) != EOS_EResult::EOS_Success)
    {
        UE_LOG(LogTemp, Error, TEXT("Malformed Epic account ID component of unique net ID: %s"), *In);
        return nullptr;
    }
    return MakeShared<FCILogonCrossPlatformAccountId>(ProductUserId);
}

FName FCILogonCrossPlatformAccountProvider::GetName()
{
    return CILOGON_CROSS_PLATFORM_ACCOUNT_ID;
}

TSharedPtr<const FCrossPlatformAccountId> FCILogonCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    const FString &InStringRepresentation)
{
    return FCILogonCrossPlatformAccountId::ParseFromString(InStringRepresentation);
}

TSharedPtr<const FCrossPlatformAccountId> FCILogonCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    uint8 *InBytes,
    int32 InSize)
{
    FString Data = BytesToString(InBytes, InSize);
    return FCILogonCrossPlatformAccountId::ParseFromString(Data);
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
    GetInteractiveAuthenticationSequence()
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetJwtForCILogonNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformCILNode>());
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
    GetInteractiveOnlyAuthenticationSequence()
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetJwtForCILogonNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformCILNode>());
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
    GetNonInteractiveAuthenticationSequence(bool bOnlyUseExternalCredentials)
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
    GetUpgradeCurrentAccountToCrossPlatformAccountSequence()
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
    GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence()
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
    GetNonInteractiveDeauthenticationSequence()
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FCILogonCrossPlatformAccountProvider::
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
