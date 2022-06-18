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

	if (!SessionInterface.IsValid())
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
	LastSessionSettings->Set(MATCH_TYPE_QUERY_KEY, MatchType,
	                         EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Green,
		FString::Printf(TEXT("IsLanMatch: %d"), IsLanMatch)
	);

	// Create the Session
	const FUniqueNetIdRepl HostingPlayerNum = GetPlayerUniqueNetId();
	if (!HostingPlayerNum.IsValid())
	{
		return;
	}
	if (!SessionInterface->CreateSession(*HostingPlayerNum, NAME_GameSession, *LastSessionSettings))
	{
		// Unbind the Delegate if something goes wrong
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnCreateSessionDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionGISubsystem::FindSessions(const int32 MaxResults)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Orange,
		FString(TEXT("FindSessions Triggered"))
	);


	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FindSessionsCompleteDelegate);

	const FUniqueNetIdRepl SearchingPlayerNum = GetPlayerUniqueNetId();
	if (!SearchingPlayerNum.IsValid())
	{
		return;
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName(TEXT("NULL"));
	LastSessionSearch->MaxSearchResults = MaxResults;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Orange,
		FString(TEXT("Before JoinSessions - FindSessions"))
	);

	if (!SessionInterface->FindSessions(*SearchingPlayerNum, LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionGISubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Orange,
		FString(TEXT("JoinSessions Triggered"))
	);


	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		JoinSessionCompleteDelegate);

	const FUniqueNetIdRepl LocalUserNum = GetPlayerUniqueNetId();
	if (!LocalUserNum.IsValid())
	{
		return;
	}
	if (!SessionInterface->JoinSession(*LocalUserNum, NAME_GameSession, SearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnJoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionGISubsystem::DestroySession()
{
}

void UMultiplayerSessionGISubsystem::StartSession()
{
}


bool UMultiplayerSessionGISubsystem::GetTravelDestination(FString& StringToCopyDestinationAddress)
{
	return SessionInterface->GetResolvedConnectString(NAME_GameSession, StringToCopyDestinationAddress);
}

void UMultiplayerSessionGISubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Yellow,
		FString(TEXT("OnCreateSessionComplete"))
	);

	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	OnCreateSessionDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionGISubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	OnFindSessionDelegate.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionGISubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	OnJoinSessionDelegate.Broadcast(Result);
}

void UMultiplayerSessionGISubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionGISubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

FUniqueNetIdRepl UMultiplayerSessionGISubsystem::GetPlayerUniqueNetId() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	const ULocalPlayer* PlayerController = World->GetFirstLocalPlayerFromController();
	if (!PlayerController)
	{
		return nullptr;
	}
	return PlayerController->GetPreferredUniqueNetId();
}
