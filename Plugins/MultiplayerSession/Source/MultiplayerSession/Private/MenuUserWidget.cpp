// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUserWidget.h"

#include "MultiplayerSessionGISubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"

void UMenuUserWidget::Setup(const int32 MaxPlayers, const FString MatchType, const FString PathToLobbyMap)
{
	SavedMaxPlayers = MaxPlayers;
	SavedMatchType = MatchType;
	SavedPathToLobbyMap = FString::Printf(TEXT("%s?listen"), *PathToLobbyMap);
	ActivateMenu();
	ConfigureMultiplayerSessionGISubsystem();
}

bool UMenuUserWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	ConfigureWidgetButtonBindings();

	return true;
}

void UMenuUserWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	DeactivateMenu();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenuUserWidget::OnCreateSession(bool bWasSuccessful)
{
	if (!GEngine)
	{
		HostButton->SetIsEnabled(true);
		return;
	}
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Purple,
		FString::Printf(TEXT("Session was %s!"),
		                bWasSuccessful ? TEXT("created") : TEXT("not created"))
	);

	if (!bWasSuccessful)
	{
		HostButton->SetIsEnabled(true);
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		HostButton->SetIsEnabled(true);
		return;
	}
	World->ServerTravel(SavedPathToLobbyMap);
}

void UMenuUserWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (!MultiplayerSessionGISubsystem)
	{
		JoinButton->SetIsEnabled(true);
		return;
	}
	if (!bWasSuccessful)
	{
		JoinButton->SetIsEnabled(true);
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(TEXT("No session found"))
		);
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Orange,
		FString(TEXT("Before OnFindSession - Loop"))
	);

	FString SearchResultMatchType;
	for (const FOnlineSessionSearchResult SearchResult : SearchResults)
	{
		SearchResult.Session.SessionSettings.Get(MultiplayerSessionGISubsystem->MATCH_TYPE_QUERY_KEY,
		                                         SearchResultMatchType);
		if (SearchResultMatchType != SavedMatchType)
		{
			continue;
		}
		return MultiplayerSessionGISubsystem->JoinSession(SearchResult);
	}

	JoinButton->SetIsEnabled(true);
}

void UMenuUserWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
		return;
	}

	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		JoinButton->SetIsEnabled(true);
		return;
	}
	FString DestinationAddress;
	if (!MultiplayerSessionGISubsystem->GetTravelDestination(DestinationAddress))
	{
		JoinButton->SetIsEnabled(true);
		return;
	}
	PlayerController->ClientTravel(DestinationAddress, TRAVEL_Absolute);
}

void UMenuUserWidget::OnDestroySession(bool bWasSuccessful)
{
}

void UMenuUserWidget::OnStartSession(bool bWasSuccessful)
{
}

void UMenuUserWidget::HandleClickHostButton()
{
	HostButton->SetIsEnabled(false);

	if (!MultiplayerSessionGISubsystem)
	{
		return;
	}
	MultiplayerSessionGISubsystem->CreateSession(SavedMaxPlayers, SavedMatchType);
}

void UMenuUserWidget::HandleClickJoinButton()
{
	JoinButton->SetIsEnabled(false);

	if (!MultiplayerSessionGISubsystem)
	{
		return;
	}
	MultiplayerSessionGISubsystem->FindSessions(10000);
}

void UMenuUserWidget::ActivateMenu()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	EnableUIControls();
}

void UMenuUserWidget::DeactivateMenu()
{
	RemoveFromParent();
	RemoveFromViewport();
	DisableUIControls();
}

void UMenuUserWidget::EnableUIControls()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}
	// Set InputMode and Cursor
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}

void UMenuUserWidget::DisableUIControls()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}
	const FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);
}

void UMenuUserWidget::ConfigureMultiplayerSessionGISubsystem()
{
	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}
	MultiplayerSessionGISubsystem = GameInstance->GetSubsystem<UMultiplayerSessionGISubsystem>();

	if (!MultiplayerSessionGISubsystem)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(TEXT("No MultiplayerSessionGISubsystem"))
		);
		return;
	}
	MultiplayerSessionGISubsystem->OnCreateSessionDelegate.AddDynamic(
		this, &ThisClass::OnCreateSession);
	MultiplayerSessionGISubsystem->OnFindSessionDelegate.AddUObject(
		this, &ThisClass::OnFindSession);
	MultiplayerSessionGISubsystem->OnJoinSessionDelegate.AddUObject(
		this, &ThisClass::OnJoinSession);
	MultiplayerSessionGISubsystem->OnDestroySessionDelegate.AddDynamic(
		this, &ThisClass::OnDestroySession);
	MultiplayerSessionGISubsystem->OnStartSessionDelegate.AddDynamic(
		this, &ThisClass::OnStartSession);
}

void UMenuUserWidget::ConfigureWidgetButtonBindings()
{
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HandleClickHostButton);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::HandleClickJoinButton);
	}
}
