// Copyright Epic Games, Inc. All Rights Reserved.

#include "STCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "ST/Core/STPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Animation/AnimPhysicsSolver.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


ASTCharacter::ASTCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("HealthComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	if (HealthComponent)
	{
		HealthComponent->OnDead.AddDynamic(this, &ASTCharacter::CharDead);
	}
	if (InventoryComponent)
	{
		
	}
	
	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASTCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (InventoryComponent)
	{
		if (InventoryComponent->WeaponSlots.Num() > 0)
		{
			BackpackInit();
		}
	}
	
	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, FVector(20.0f, 40.0f, 40.0f), FVector(0));
	}
}

void ASTCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CurrentCursor)
	{
		const ASTPlayerController* MyPC = Cast<ASTPlayerController>(GetController());
		if (MyPC)
		{
			FHitResult TraceHitResult;
			MyPC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			
			const FVector CursorLocation = TraceHitResult.ImpactNormal;
			const FRotator CursorRotation = CursorLocation.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorRotation);
			if(CurrentWeapon)
			{
				CurrentWeapon->ShootEndLocation = TraceHitResult.Location;
			}
		}
	}
}

void ASTCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAction("Sprinting", IE_Pressed, this, &ASTCharacter::SprintEnable);
	NewInputComponent->BindAction("Sprinting", IE_Released, this, &ASTCharacter::SprintDisable);
	NewInputComponent->BindAction("Reload", IE_Pressed, this, &ASTCharacter::TryReloadWeapon);
	NewInputComponent->BindAction("Fire", IE_Pressed, this, &ASTCharacter::AttackPressed);
	NewInputComponent->BindAction("Fire", IE_Released, this, &ASTCharacter::AttackReleased);	
}

void ASTCharacter::SprintEnable()
{
	CurrentMovementState = EMovementStates::Sprinting;
}

void ASTCharacter::SprintDisable()
{
	CurrentMovementState = EMovementStates::Running;
}

void ASTCharacter::BackpackInit()
{
	FWeaponInfo WeaponInfo;
	FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
	
	int8 i = 0;
	while (i != InventoryComponent->WeaponSlots.Num())
	{
				
		if(InventoryComponent->WeaponSlots[i].WeaponClass)
		{
			FVector SpawnLocation = FVector(0);
			FRotator SpawnRotation = FRotator(0);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			ASTWeapon_Default* MyWeapon = Cast<ASTWeapon_Default>(GetWorld()->SpawnActor(InventoryComponent->WeaponSlots[i].WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (MyWeapon)
			{
				MyWeapon->AttachToComponent(GetMesh(), Rule, MyWeapon->WeaponBackpackSlot);

				USTGameInstance* MyGI = Cast<USTGameInstance>(GetWorld()->GetGameInstance());
				if (MyGI)
				{
					FWeaponInfo MyWeaponInfo;
					if(MyGI->GetWeaponInfoByName(MyWeapon->WeaponName, MyGI->WeaponInfoTable, MyWeaponInfo))
					{
						MyWeapon->WeaponSetting = MyWeaponInfo;
						MyWeapon->CurrentRound = MyWeaponInfo.MaxRound;
					}
				}
				
				BackpackWeapons.Add(MyWeapon);
				BackpackWeapons[i]->OnWeaponReloadStart.AddDynamic(this, &ASTCharacter::WeaponReloadStart);
				BackpackWeapons[i]->OnFire.AddDynamic(this, &ASTCharacter::CharAttack);
				BackpackWeapons[i]->OnWeaponReloadEnd.AddDynamic(this, &ASTCharacter::WeaponReloadEnd);
			}
		}
		i++;
	}
	EquipNewWeapon(BackpackWeapons[0]);
}

void ASTCharacter::EquipNewWeapon(ASTWeapon_Default* NewWeapon)
{
	FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
	if(!CurrentWeapon)
	{
		CurrentWeapon = NewWeapon;
		CurrentWeapon->AttachToComponent(GetMesh(), Rule, CurrentWeapon->WeaponHandsSlot);
		InventoryComponent->OnSwitchWeapon.Broadcast(NewWeapon->WeaponType);
	}
	else
	{
		if(CurrentWeapon != NewWeapon)
		{
			CurrentWeapon->AttachToComponent(GetMesh(), Rule, CurrentWeapon->WeaponBackpackSlot);
			NewWeapon->AttachToComponent(GetMesh(), Rule, CurrentWeapon->WeaponHandsSlot);
			CurrentWeapon = NewWeapon;
			InventoryComponent->OnSwitchWeapon.Broadcast(NewWeapon->WeaponType);
		}
	}
	
}

void ASTCharacter::AttackPressed()
{
	AttackCharEvent(true);
	if (!CurrentWeapon->WeaponSetting.bIsReloadable && !bIsSingleShootEnable)
	{
		bIsSingleShootEnable = true;
		GetWorld()->GetTimerManager().SetTimer(PistolFireRate, this, &ASTCharacter::PistolRateOfFireTimer, GetWorld()->GetDeltaSeconds(), true);
	}
}

void ASTCharacter::AttackReleased()
{
	AttackCharEvent(false);
	if (!CurrentWeapon->WeaponSetting.bIsReloadable && bIsSingleShootEnable)
	{
		bIsSingleShootEnable = false;
		GetWorld()->GetTimerManager().SetTimer(PistolFireRate, this, &ASTCharacter::PistolRateOfFireTimer, GetWorld()->GetDeltaSeconds(), true);
	}
}

void ASTCharacter::PistolRateOfFireTimer()
{
	CurrentWeapon->FireTimer -= GetWorld()->GetDeltaSeconds();
	if (CurrentWeapon->FireTimer < 0.f)
	{
		bIsSingleShootEnable = true;
		GetWorld()->GetTimerManager().ClearTimer(PistolFireRate);
	}
}

void ASTCharacter::AttackCharEvent(bool bIsFiring)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->SetWeaponStateFire(bIsFiring);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
	}
}

// play character attack anim montage
void ASTCharacter::CharAttack()
{
	if (CharacterAnimations.CharacterAttack)
	{
		CharAttack_BP(CharacterAnimations.CharacterAttack);
	}
}

void ASTCharacter::CharDead_BP_Implementation(UAnimMontage* CharAnim)
{
	// in BP
}

void ASTCharacter::TryReloadWeapon() 
{
	if (CurrentWeapon && !CurrentWeapon->bIsWeaponReloading && CurrentWeapon->CanWeaponReload())
	{
		if (CurrentWeapon->GetWeaponRound() < CurrentWeapon->WeaponSetting.MaxRound)
		{
			CurrentWeapon->InitReload();
		}
	}
}

// play character reload anim montage
void ASTCharacter::WeaponReloadStart()
{
	if (CharacterAnimations.CharacterReload)
	{
		WeaponReloadStart_BP(CharacterAnimations.CharacterReload);
	}
}

void ASTCharacter::WeaponReloadEnd(bool bIsSuccess, int32 AmmoInWeapon)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->WeaponAmmoChange(CurrentWeapon->WeaponSetting.WeaponType, AmmoInWeapon);
		//InventoryComponent->SetAdditionalWeaponInfo(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}
	
	WeaponReloadEnd_BP(bIsSuccess);
}

float ASTCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bIsAlive)
	{
		HealthComponent->ChangeHealthValue(-DamageAmount);
	}

	return ActualDamage;
}

void ASTCharacter::CharDead()
{
	UE_LOG(LogTemp, Warning, TEXT("Dead"));

	if (CharacterAnimations.CharacterDeath.Num()>0)
	{
		CharDead_BP(CharacterAnimations.CharacterDeath[rand() % CharacterAnimations.CharacterDeath.Num()]);
	}
	bIsAlive = false;
	UnPossessed();
	GetCursorToWorld()->SetVisibility(false);
}

void ASTCharacter::CharAttack_BP_Implementation(UAnimMontage* CharAnim)
{
	// in BP
}

void ASTCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* CharAnim)
{
	// in BP
}

void ASTCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSuccess)
{
	// in BP
}
