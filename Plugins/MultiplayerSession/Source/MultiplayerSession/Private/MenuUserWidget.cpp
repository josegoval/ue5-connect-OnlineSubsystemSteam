// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUserWidget.h"

#include "MultiplayerSessionGISubsystem.h"
#include "Components/Button.h"

void UMenuUserWidget::Setup(const int32 MaxPlayers, const FString MatchType)
{
	SavedMaxPlayers = MaxPlayers;
	SavedMatchType = MatchType;
	ActivateMenu();
	ConfigureWidgetButtonBindings();
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

void UMenuUserWidget::HandleClickHostButton()
{
	if (!GEngine)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Yellow,
		FString(TEXT("HostButton clicked"))
	);

	if (!MultiplayerSessionGISubsystem)
	{
		return;
	}
	MultiplayerSessionGISubsystem->CreateSession(SavedMaxPlayers, SavedMatchType);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	World->ServerTravel(FString(TEXT("/Game/ThirdPerson/Maps/Lobby?listen")));
}

void UMenuUserWidget::HandleClickJoinButton()
{
	if (!GEngine)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Yellow,
		FString(TEXT("JoinButton clicked"))
	);

	// if (!MultiplayerSessionGISubsystem)
	// {
	// 	return;
	// }
	// MultiplayerSessionGISubsystem->JoinSession();
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
