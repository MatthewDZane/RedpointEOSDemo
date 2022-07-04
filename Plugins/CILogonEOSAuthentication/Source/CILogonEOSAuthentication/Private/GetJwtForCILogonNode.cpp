#if EOS_HAS_AUTHENTICATION

#include "GetJwtForCILogonNode.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "OnlineSubsystemRedpointEOS/Shared/Authentication/AuthenticationHelpers.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSConfig.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSString.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonTypes.h"


EOS_ENABLE_STRICT_WARNINGS

void FGetJwtForCILogonNode::OnCILogonTokenResponse(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpRequestPtr Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpResponsePtr Response,
    bool bConnectedSuccessfully,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (!bConnectedSuccessfully)
    {
        State->ErrorMessages.Add(TEXT("Unable to connect to login URL for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
    {
        State->ErrorMessages.Add(Response->GetContentAsString());
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    TSharedPtr<FJsonValue> JsonValue;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonValue))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't JSON, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    const TSharedPtr<FJsonObject> *JsonObject;
    if (!JsonValue.IsValid() || !JsonValue->TryGetObject(JsonObject))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (!(*JsonObject)->HasTypedField<EJson::String>("access_token") ||
        !(*JsonObject)->HasTypedField<EJson::String>("id_token") ||
        !(*JsonObject)->HasTypedField<EJson::String>("token_type") ||
        !(*JsonObject)->HasTypedField<EJson::Number>("expires_in"))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL gdgwasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    FString AccessToken = (*JsonObject)->GetStringField("access_token");
    FString IdToken = (*JsonObject)->GetStringField("id_token");
    FString TokenType = (*JsonObject)->GetStringField("token_type");
    int ExpiresIn = (*JsonObject)->GetIntegerField("expires_in");
    
    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("Obtained CILogon data for first party login, access token: '%s', id token: '%s', token type: '%s', expires in: '%d'"),
        *AccessToken,
        *IdToken,
        *TokenType,
        ExpiresIn);

    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpRequest->SetURL("https://cilogon.org/oauth2/userinfo");
    HttpRequest->SetContentAsString(FString::Printf(TEXT("access_token=%s"), *AccessToken));
    HttpRequest->OnProcessRequestComplete()
        .BindSP(this, &FGetJwtForCILogonNode::OnCILogonUserInfoResponse, State, OnDone);
    if (!HttpRequest->ProcessRequest())
    {
        State->ErrorMessages.Add(TEXT("Unable to start login request for first party login."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
}

void FGetJwtForCILogonNode::OnCILogonUserInfoResponse(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpRequestPtr Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpResponsePtr Response,
    bool bConnectedSuccessfully,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (!bConnectedSuccessfully)
    {
        State->ErrorMessages.Add(TEXT("Unable to connect to login URL for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
    {
        State->ErrorMessages.Add(TEXT("Non-200 response from login URL when obtaining JWT for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    TSharedPtr<FJsonValue> JsonValue;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonValue))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't JSON, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    const TSharedPtr<FJsonObject> *JsonObject;
    if (!JsonValue.IsValid() || !JsonValue->TryGetObject(JsonObject))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (!(*JsonObject)->HasTypedField<EJson::String>("sub") ||
        !(*JsonObject)->HasTypedField<EJson::String>("idp_name") ||
        !(*JsonObject)->HasTypedField<EJson::String>("eppn") ||
        !(*JsonObject)->HasTypedField<EJson::String>("cert_subject_dn") ||
        !(*JsonObject)->HasTypedField<EJson::String>("given_name") ||
        !(*JsonObject)->HasTypedField<EJson::String>("acr") ||
        !(*JsonObject)->HasTypedField<EJson::String>("aud") ||
        !(*JsonObject)->HasTypedField<EJson::String>("idp") ||
        !(*JsonObject)->HasTypedField<EJson::String>("affiliation") ||
        !(*JsonObject)->HasTypedField<EJson::String>("name") ||
        !(*JsonObject)->HasTypedField<EJson::String>("family_name") ||
        !(*JsonObject)->HasTypedField<EJson::String>("email"))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    TArray<FString> SubStrings;
    (*JsonObject)->GetStringField("sub").ParseIntoArray(SubStrings, TEXT("/"));
    FString Sub = SubStrings[SubStrings.Num() - 1];
    FString IdpName = (*JsonObject)->GetStringField("idp_name");
    FString Eppn = (*JsonObject)->GetStringField("eppn");
    FString CertSubjectDn = (*JsonObject)->GetStringField("cert_subject_dn");
    FString GivenName = (*JsonObject)->GetStringField("given_name");
    FString Acr = (*JsonObject)->GetStringField("acr");
    FString Aud = (*JsonObject)->GetStringField("aud");
    FString Idp = (*JsonObject)->GetStringField("idp");
    FString Affiliation = (*JsonObject)->GetStringField("affiliation");
    FString Name = (*JsonObject)->GetStringField("name");
    FString FamilyName = (*JsonObject)->GetStringField("family_name");
    FString Email = (*JsonObject)->GetStringField("email");

    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpRequest->SetURL("https://simple-first-party-auth.matthewdzane.workers.dev/issue");
    HttpRequest->SetContentAsString(FString::Printf(
        TEXT("secret=%s&sub=%d&nickname=%s"), *FString("o1Baj2l6IzfF"), FCString::Atoi(*Sub), *Name));
    HttpRequest->OnProcessRequestComplete()
        .BindSP(this, &FGetJwtForCILogonNode::OnCloudFlareHttpResponse, State, OnDone);
    if (!HttpRequest->ProcessRequest())
    {
        State->ErrorMessages.Add(TEXT("Unable to start login request for first party login."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
}

void FGetJwtForCILogonNode::OnCloudFlareHttpResponse(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpRequestPtr Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpResponsePtr Response,
    bool bConnectedSuccessfully,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (!bConnectedSuccessfully)
    {
        State->ErrorMessages.Add(TEXT("Unable to connect to login URL for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
    {
        State->ErrorMessages.Add(TEXT("Non-200 response from login URL when obtaining JWT for authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    TSharedPtr<FJsonValue> JsonValue;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonValue))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't JSON, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    const TSharedPtr<FJsonObject> *JsonObject;
    if (!JsonValue.IsValid() || !JsonValue->TryGetObject(JsonObject))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (!(*JsonObject)->HasTypedField<EJson::String>("access_token") ||
        !(*JsonObject)->HasTypedField<EJson::String>("sub"))
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("Response data from login URL wasn't expected JSON format, got: '%s'."),
            *Response->GetContentAsString()));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    FString AccessToken = (*JsonObject)->GetStringField("access_token");
    FString UserId = (*JsonObject)->GetStringField("sub");
    if (FCString::Atoi64(*UserId) == 0)
    {
        State->ErrorMessages.Add(
            TEXT("Got response from login URL, but it indicated a user ID of 0 which is not valid. "
                 "User IDs are non-zero 64-bit integers."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    State->Metadata.Add(TEXT("FIRST_PARTY_ACCESS_TOKEN"), AccessToken);
    State->Metadata.Add(TEXT("FIRST_PARTY_USER_ID"), UserId);

    
    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("Obtained data for first party login, user ID: '%s', access token: '%s'"),
        *UserId,
        *AccessToken);
    
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    return;
    
}

void FGetJwtForCILogonNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    FString FirstPartyUrl = State->Config->GetSimpleFirstPartyLoginUrl();
    if (FirstPartyUrl.IsEmpty())
    {
        State->ErrorMessages.Add(TEXT("First party login URL not configured in Project Settings."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    auto HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");

    const FString GrantType = "authorization_code";
    const FString ClientID = "cilogon:/client_id/461cf71cc6f4d3fc73cfd4e926000ff";
    const FString ClientSecret = "LLEZd5377glkfv-ELKyAkjTxBdcnCOeN7sf9aembizV3v1-fEZZluFyrtkwFGv3jG0O7A62m_LoZ_hbgHOTKhw";
    const FString RedirectURI = "https://matthew_zane.nrp-nautilus.io/hub/oauth_callback";
    
    HttpRequest->SetURL("https://cilogon.org/oauth2/token");
    HttpRequest->SetContentAsString(FString::Printf(
        TEXT("grant_type=%s&client_id=%s&client_secret=%s&code=%s&redirect_uri=%s"),
        *FPlatformHttp::UrlEncode(GrantType),
        *FPlatformHttp::UrlEncode(ClientID),
        *FPlatformHttp::UrlEncode(ClientSecret),
        *FPlatformHttp::UrlEncode(State->ProvidedCredentials.Token),
        *FPlatformHttp::UrlEncode(RedirectURI)));
    HttpRequest->OnProcessRequestComplete()
        .BindSP(this, &FGetJwtForCILogonNode::OnCILogonTokenResponse, State, OnDone);
    if (!HttpRequest->ProcessRequest())
    {
        State->ErrorMessages.Add(TEXT("Unable to start login request for first party login."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
}

EOS_DISABLE_STRICT_WARNINGS

#endif // #if EOS_HAS_AUTHENTICATION
