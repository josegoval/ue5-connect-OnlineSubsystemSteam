// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUserWidget.h"

#include "MultiplayerSessionGISubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"

void UMenuUserWidget::Setup(const int32 MaxPlayers, const FString MatchType)
{
	SavedMaxPlayers = MaxPlayers;
	SavedMatchType = MatchType;
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
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	World->ServerTravel(FString(TEXT("/Game/ThirdPerson/Maps/Lobby?listen")));
}

void UMenuUserWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (!MultiplayerSessionGISubsystem)
	{
		return;
	}
	if (!bWasSuccessful)
	{
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
}

void UMenuUserWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		return;
	}
	FString DestinationAddress;
	if (!MultiplayerSessionGISubsystem->GetTravelDestination(DestinationAddress))
	{
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
	if (!MultiplayerSessionGISubsystem)
	{
		return;
	}
	MultiplayerSessionGISubsystem->CreateSession(SavedMaxPlayers, SavedMatchType);
}

void UMenuUserWidget::HandleClickJoinButton()
{
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
