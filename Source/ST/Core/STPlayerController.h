// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ST/FunctionLibrary/STBPFunctionLibrary.h"
#include "STPlayerController.generated.h"

UCLASS()
class ASTPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASTPlayerController();

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

public:

	// Character movement
	bool bInputPressed = false;
	
	void InputAxisY(float Value);
	void InputAxisX(float Value);
	
	void MovementStateSpeedUpdate(EMovementStates MovementState, float &ResultSpeed);
	
	float AxisY;
	float AxisX;
};


