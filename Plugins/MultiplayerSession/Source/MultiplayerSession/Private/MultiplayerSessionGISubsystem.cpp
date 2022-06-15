// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionGISubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionGISubsystem::UMultiplayerSessionGISubsystem():
	CreateSessionCompleteDelegate(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))


{
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		return;
	}
	SessionInterface = OnlineSubsystem->GetSessionInterface();
}

void UMultiplayerSessionGISubsystem::CreateSession(int32 MaxPlayers, FString MatchType)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Green,
		FString(TEXT("CreateSession Triggered"))
	);

	if (!SessionInterface)
	{
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	// Set the Delegate
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	// set the SessionSettings
	const bool IsLanMatch = IOnlineSubsystem::Get()->GetSubsystemName() == FName(TEXT("NULL"));
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->NumPublicConnections = MaxPlayers;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->bIsLANMatch = IsLanMatch;
	LastSessionSettings->Set(FName(TEXT("MatchType")), MatchType,
	                         EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Green,
		FString::Printf(TEXT("IsLanMatch: %d"), IsLanMatch)
	);

	// Create the Session
	const auto World = GetWorld();
	if (!World)
	{
		return;
	}
	const auto FirstLocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!FirstLocalPlayer)
	{
		return;
	}
	const auto HostingPlayerNum = FirstLocalPlayer->GetPreferredUniqueNetId();
	if (!SessionInterface->CreateSession(*HostingPlayerNum, NAME_GameSession, *LastSessionSettings))
	{
		// Unbind the Delegate if something goes wrong
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
}

void UMultiplayerSessionGISubsystem::FindSessions(int32 MaxResults)
{
}

void UMultiplayerSessionGISubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
}

void UMultiplayerSessionGISubsystem::DestroySession()
{
}

void UMultiplayerSessionGISubsystem::StartSession()
{
}

void UMultiplayerSessionGISubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionGISubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionGISubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UMultiplayerSessionGISubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionGISubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
