// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup();

protected:
	virtual bool Initialize() override;

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;


	UPROPERTY()
	class UMultiplayerSessionGISubsystem* MultiplayerSessionGISubsystem;

	UFUNCTION()
	void HandleClickHostButton();
	UFUNCTION()
	void HandleClickJoinButton();

	void ConfigureMultiplayerSessionGISubsystem();
	void ConfigureWidgetButtonBindings();
};