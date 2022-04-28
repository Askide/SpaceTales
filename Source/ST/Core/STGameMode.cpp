// Copyright Epic Games, Inc. All Rights Reserved.

#include "STGameMode.h"
#include "STPlayerController.h"
#include "ST/Characters/STCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASTGameMode::ASTGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASTPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SpaceTales/Characters/MainCharacter/BP_MainCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/SpaceTales/Core/BP_PlayerController"));
	if(PlayerControllerBPClass.Class != nullptr)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}