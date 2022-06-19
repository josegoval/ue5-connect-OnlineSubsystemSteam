// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionGISubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateSessionDelegate, bool, bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFindSessionDelegate, const TArray<FOnlineSessionSearchResult>& SearchResults,
                                     bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinSessionDelegate, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestroySessionDelegate, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartSessionDelegate, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiplayerSessionGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionGISubsystem();

	// Public API entry points 
	void CreateSession(int32 MaxPlayers, FString MatchType);
	void FindSessions(const int32 MaxResults);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void DestroySession();
	void StartSession();

	FOnCreateSessionDelegate OnCreateSessionDelegate;
	FOnFindSessionDelegate OnFindSessionDelegate;
	FOnJoinSessionDelegate OnJoinSessionDelegate;
	FOnDestroySessionDelegate OnDestroySessionDelegate;
	FOnStartSessionDelegate OnStartSessionDelegate;

	const FName MATCH_TYPE_QUERY_KEY = FName(TEXT("MatchType"));

	// Utils
	bool GetTravelDestination(FString& StringToCopyDestinationAddress);
protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	// Session related data
	bool bCreateSessionAfterDestroy = false;
	int32 DesiredMaxPlayers;
	FString DesiredMatchType;

	// SessionInterface Delegate(s) and DelegateHandler(s)
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	// Utils
	FUniqueNetIdRepl GetPlayerUniqueNetId() const;
};
