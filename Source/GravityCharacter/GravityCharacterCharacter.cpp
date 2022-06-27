// Copyright Epic Games, Inc. All Rights Reserved.

#include "GravityCharacterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GravityMovementcomponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include <Kismet/GameplayStatics.h>

//////////////////////////////////////////////////////////////////////////
// AGravityCharacterCharacter

AGravityCharacterCharacter::AGravityCharacterCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(
		ObjectInitializer.
		SetDefaultSubobjectClass<UGravityMovementcomponent>(ACharacter::CharacterMovementComponentName)
	)
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
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGravityCharacterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &AGravityCharacterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGravityCharacterCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AGravityCharacterCharacter::TurnAtRate);
	//	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AGravityCharacterCharacter::LookUpAtRate);
	//	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("FClick", IE_Pressed, this, &AGravityCharacterCharacter::OnFClick);
}

void AGravityCharacterCharacter::Destroyed()
{
	Super::Destroyed();
}

void AGravityCharacterCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AGravityCharacterCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AGravityCharacterCharacter::OnFClick()
{
	FMinimalViewInfo DesiredView;
	FollowCamera->GetCameraView(0, DesiredView);

	auto StartPt = DesiredView.Location;
	auto StopPt = DesiredView.Location + (DesiredView.Rotation.Vector() * 1000);

	FHitResult Result;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	FCollisionQueryParams Params;
	Params.bTraceComplex = false;

	if (GetWorld()->LineTraceSingleByObjectType(
		Result,
		StartPt,
		StopPt,
		ObjectQueryParams,
		Params)
		)
	{
		TArray<AActor*>ActorAry;
		UGameplayStatics::GetAllActorsWithTag(this, TEXT("ttt"), ActorAry);
		if (ActorAry.IsValidIndex(0))
		{
			auto CharacterPtr = Cast<AGravityCharacterCharacter>(ActorAry[0]);

			const auto AIControllerPtr = UAIBlueprintHelperLibrary::GetAIController(CharacterPtr->Controller);

			DrawDebugSphere(GetWorld(), Result.ImpactPoint, 10, 10, FColor::Red, false, 10);

			UAIBlueprintHelperLibrary::SimpleMoveToLocation(
				AIControllerPtr,
				Result.ImpactPoint
			);
		}
	}
}

void AGravityCharacterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddActorLocalRotation(FRotator(0, Rate * 5, 0));
}

void AGravityCharacterCharacter::LookUpAtRate(float Rate)
{
	auto Rot = CameraBoom->GetRelativeRotation();

	// calculate delta for this frame from the rate information
	CameraBoom->SetRelativeRotation(FRotator(FMath::Clamp(Rot.Pitch + Rate * 5,-70, 70), 0, 0));
}

void AGravityCharacterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		auto Dir = GetCapsuleComponent()->GetForwardVector();

		// get forward vector
		AddMovementInput(Dir, Value);
	}
}

void AGravityCharacterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		auto Dir = GetCapsuleComponent()->GetRightVector();

		// get forward vector
		AddMovementInput(Dir, Value);
	}
}
