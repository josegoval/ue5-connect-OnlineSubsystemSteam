// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ReportPlayerCounter();
	ReportPlayerLoginOrLogout(NewPlayer);
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ReportPlayerCounter(1);
	ReportPlayerLoginOrLogout(Exiting);
}

void ALobbyGameMode::ReportPlayerCounter(const int32 Subtract) const
{
	if (!GameState || !GEngine)
	{
		return;
	}

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num() - Subtract;
	GEngine->AddOnScreenDebugMessage(
		1,
		60.f,
		FColor::Turquoise,
		FString::Printf(TEXT("Players Online: %d"), NumberOfPlayers)
	);
}

void ALobbyGameMode::ReportPlayerLoginOrLogout(const AController* Controller, const bool IsLogout)
{
	if (!Controller)
	{
		return;
	}
	const APlayerState* PlayerState = Controller->GetPlayerState<APlayerState>();
	if (!PlayerState)
	{
		return;
	}
	const FString PlayerName = PlayerState->GetPlayerName();

	GEngine->AddOnScreenDebugMessage(
		1,
		60.f,
		IsLogout ? FColor::Red : FColor::Green,
		IsLogout
			? FString::Printf(TEXT("%s has exited the game!"),
			                  *PlayerName)
			: FString::Printf(
				TEXT("%s has joined the game!"),
				*PlayerName)
	);
}
