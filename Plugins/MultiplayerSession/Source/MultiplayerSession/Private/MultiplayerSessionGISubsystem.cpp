// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionGISubsystem.h"

#include "OnlineSubsystem.h"

UMultiplayerSessionGISubsystem::UMultiplayerSessionGISubsystem()
{
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		return;
	}
	SessionInterface = OnlineSubsystem->GetSessionInterface();
}
