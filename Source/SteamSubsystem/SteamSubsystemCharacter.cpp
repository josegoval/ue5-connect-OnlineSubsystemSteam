// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteamSubsystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASteamSubsystemCharacter

ASteamSubsystemCharacter::ASteamSubsystemCharacter():
	OnCreateSessionCompleteDelegate(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	OnFindSessionsCompleteDelegate(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	OnJoinSessionCompleteDelegate(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	ConfigureOnlineSubsystem();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASteamSubsystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ASteamSubsystemCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ASteamSubsystemCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ASteamSubsystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ASteamSubsystemCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASteamSubsystemCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASteamSubsystemCharacter::TouchStopped);
}

void ASteamSubsystemCharacter::ConfigureOnlineSubsystem()
{
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		return;
	}
	OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!GEngine)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f, FColor::Blue,
		FString::Printf(
			TEXT("Found subsystem %s"),
			*OnlineSubsystem->GetSubsystemName().ToString()));
}

void ASteamSubsystemCharacter::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}


	if (OnlineSessionInterface->GetNamedSession(GameSessionName))
	{
		OnlineSessionInterface->DestroySession(GameSessionName);
	}

	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	const auto PlayerHostingNum = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();
	const TSharedPtr<FOnlineSessionSettings> OnlineSessionSettings = MakeShareable(new FOnlineSessionSettings());
	OnlineSessionSettings->bShouldAdvertise = true;
	OnlineSessionSettings->bUsesPresence = true;
	OnlineSessionSettings->NumPublicConnections = 4;
	OnlineSessionSettings->bAllowJoinViaPresence = true;
	OnlineSessionSettings->bAllowJoinInProgress = true;
	OnlineSessionSettings->bIsLANMatch = false;
	OnlineSessionSettings->bUseLobbiesIfAvailable = true;
	OnlineSessionSettings->Set(MatchTypeKey, MatchTypeValue, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionInterface->CreateSession(*PlayerHostingNum, GameSessionName, *OnlineSessionSettings);
}

void ASteamSubsystemCharacter::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	const auto SearchingPlayerId = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();
	SearchSettings = MakeShareable(new FOnlineSessionSearch());
	SearchSettings->bIsLanQuery = false;
	SearchSettings->MaxSearchResults = 10000;
	SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	OnlineSessionInterface->FindSessions(*SearchingPlayerId, SearchSettings.ToSharedRef());
}

void ASteamSubsystemCharacter::OnCreateSessionComplete(const FName SessionName, const bool bWasSuccessful)
{
	if (!GEngine)
	{
		return;
	}
	if (!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1,
		                                 15.f,
		                                 FColor::Red,
		                                 FString(TEXT("The session couldn't be created")));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1,
	                                 15.f,
	                                 FColor::Green,
	                                 FString::Printf(TEXT("The session %s was created, travelling to lobby..."),
	                                                 *SessionName.ToString()));
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	World->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen"));
}

void ASteamSubsystemCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful || !GEngine || !OnlineSessionInterface.IsValid())
	{
		return;
	}

	for (const auto SearchResult : SearchSettings->SearchResults)
	{
		const FString SessionId = SearchResult.GetSessionIdStr();
		const FString OwningUserName = SearchResult.Session.OwningUserName;
		const auto LocalUserId = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();

		GEngine->AddOnScreenDebugMessage(-1,
		                                 15.f,
		                                 FColor::Purple,
		                                 FString::Printf(
			                                 TEXT("Session found %s owned by %s"),
			                                 *SessionId, *OwningUserName)
		);

		FString MatchType;
		SearchResult.Session.SessionSettings.Get(MatchTypeKey, MatchType);
		if (MatchType != MatchTypeValue)
		{
			GEngine->AddOnScreenDebugMessage(-1,
			                                 15.f,
			                                 FColor::Red,
			                                 FString::Printf(
				                                 TEXT("This session has a different MatchType: %s"),
				                                 *MatchType));
			continue;
		}

		GEngine->AddOnScreenDebugMessage(-1,
		                                 15.f,
		                                 FColor::Purple,
		                                 FString(
			                                 TEXT("Session with desired MatchType found. Joining the match...")
		                                 ));
		// Try to join the session
		OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		OnlineSessionInterface->JoinSession(
			*LocalUserId,
			GameSessionName,
			SearchResult
		);

		return;
	}
}

void ASteamSubsystemCharacter::OnJoinSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		GEngine->AddOnScreenDebugMessage(-1,
		                                 15.f,
		                                 FColor::Red,
		                                 FString(
			                                 TEXT("JoinSessionCompleteResult was not successful")
		                                 ));
		return;
	}

	FString ConnectInfo;
	if (!OnlineSessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
	{
		GEngine->AddOnScreenDebugMessage(-1,
		                                 15.f,
		                                 FColor::Red,
		                                 FString::Printf(
			                                 TEXT("Couldn't join the session named %s."), *SessionName.ToString()
		                                 ));
		return;
	}

	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		GEngine->AddOnScreenDebugMessage(-1,
		                                 15.f,
		                                 FColor::Red,
		                                 FString(
			                                 TEXT("No player controller found.")
		                                 ));
		return;
	}

	PlayerController->ClientTravel(ConnectInfo, TRAVEL_Absolute);
}

void ASteamSubsystemCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ASteamSubsystemCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ASteamSubsystemCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ASteamSubsystemCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ASteamSubsystemCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASteamSubsystemCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
