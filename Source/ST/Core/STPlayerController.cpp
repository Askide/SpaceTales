// Copyright Epic Games, Inc. All Rights Reserved.

#include "STPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "ST/Characters/STCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

ASTPlayerController::ASTPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ASTPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	ASTCharacter* const playerCharacter = Cast<ASTCharacter>(GetPawn());
	if (playerCharacter)
	{
		playerCharacter->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
		playerCharacter->AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

		if(AxisY == 0 && AxisX == 0)
		{
			playerCharacter->CurrentMovementState = EMovementStates::Idle;
		}
		else
		{
			if(playerCharacter->CurrentMovementState != EMovementStates::Sprinting)
			{
				playerCharacter->CurrentMovementState = EMovementStates::Running;
			}
		}
		
		if (playerCharacter->CurrentMovementState == EMovementStates::Sprinting)
		{
			FVector myRotationVector = FVector(AxisX, AxisY, 0.0f);
			FRotator myRotator = myRotationVector.ToOrientationRotator();
			playerCharacter->SetActorRotation((FQuat(myRotator)));
		}
		else
		{
			FHitResult ResultHit;
			GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);

			float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(playerCharacter->GetActorLocation(), ResultHit.Location).Yaw;
			playerCharacter->SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
		}
		
		MovementStateSpeedUpdate(playerCharacter->CurrentMovementState, playerCharacter->GetCharacterMovement()->MaxWalkSpeed);
	}
}

void ASTPlayerController::SetupInputComponent()
{
	// Set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ASTPlayerController::InputAxisX);
	InputComponent->BindAxis("MoveRight", this, &ASTPlayerController::InputAxisY);

}

void ASTPlayerController::InputAxisY(float Value)
{
	AxisY = Value;
}

void ASTPlayerController::InputAxisX(float Value)
{
	AxisX = Value;
}

void ASTPlayerController::MovementStateSpeedUpdate(EMovementStates MovementState, float &ResultSpeed)			
{
	switch (MovementState)
	{
	case EMovementStates::Sprinting:
		ResultSpeed = 600.0f;
		break;
	default:
		ResultSpeed = 400.0f;
		break;
	}
}

