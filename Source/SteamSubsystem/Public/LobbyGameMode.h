// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSUBSYSTEM_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
private:
	void ReportPlayerCounter(const int32 Subtract = 0) const;
	void ReportPlayerLoginOrLogout(const AController* Controller, const bool IsLogout = false);
};
