// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionGISubsystem.h"

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
