// Fill out your copyright notice in the Description page of Project Settings.


#include "STWeapon_Default.h"

#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ST/CharacterComponents/InventoryComponent.h"

// Sets default values
ASTWeapon_Default::ASTWeapon_Default()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	SkeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshWeapon->SetGenerateOverlapEvents(false);
	SkeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshWeapon->SetupAttachment(RootComponent);

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASTWeapon_Default::BeginPlay()
{
	Super::BeginPlay();
	WeaponInit();
}

// Called every frame
void ASTWeapon_Default::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WeaponSetting.bIsReloadable)
	{
		FireTick(DeltaTime);	
	}
	ReloadTick(DeltaTime);
	DispersionTick(DeltaTime);
}

void ASTWeapon_Default::WeaponInit() const
{
	if (SkeletalMeshWeapon && !SkeletalMeshWeapon->SkeletalMesh)
	{
		SkeletalMeshWeapon->DestroyComponent(true);
	}

	if (StaticMeshWeapon && !StaticMeshWeapon->GetStaticMesh())
	{
		StaticMeshWeapon->DestroyComponent(true);
	}

}

void ASTWeapon_Default::FireTick(float DeltaTime)
{
	if (bIsWeaponFiring && GetWeaponRound() > 0 && !bIsWeaponReloading)
	{
		if (FireTimer < 0.f)
		{
			Fire();
		}
		else
		{
			FireTimer -= DeltaTime;
		}
	}	
}

bool ASTWeapon_Default::CheckWeaponCanFire() const
{
	return GetWeaponRound() > 0;
}

void ASTWeapon_Default::Fire()
{
	
	if (WeaponSetting.bIsReloadable)
	{
		FireTimer = WeaponSetting.RateOfFire;
		CurrentRound = CurrentRound - 1;	
	}
	ChangeDispersionByShot();

	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());
	
	if (ShootLocation)
	{
		const FVector SpawnLocation = ShootLocation->GetComponentLocation();

		const FVector EndLocation = GetFireEndLocation();
		FVector Dir = EndLocation - SpawnLocation;
		Dir.Normalize();

		OnFire.Broadcast();
		
		FHitResult HitResult;

		const FVector TraceStartPoint = SpawnLocation;
		const FVector TraceEndPoint = EndLocation;
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartPoint, TraceEndPoint, ECC_WorldStatic))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("TraceWork")));
			DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), HitResult.Location, FColor::Yellow, false, 5.f, (uint8)'\000', 0.5f);
			DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Green, false, 4.0f);
			
			if(HitResult.GetActor() && HitResult.PhysMaterial.IsValid())
			{
				// EPhysicalSurface mySurfaceType = UGameplayStatics::GetSurfaceType(HitResult);
				//
				// if (WeaponSetting.ProjectileSetting.HitDecals.Contains(mySurfaceType))
				// {
				// 	UMaterialInterface* myMaterial = WeaponSetting.ProjectileSetting.HitDecals[mySurfaceType];
				//
				// 	if (myMaterial && HitResult.GetComponent())
				// 	{
				// 		UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.0f), HitResult.GetComponent(), NAME_None, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.0f);
				// 	}
				// }
				// if (WeaponSetting.ProjectileSetting.HitFXs.Contains(mySurfaceType))
				// {
				// 	UParticleSystem* myParticle = WeaponSetting.ProjectileSetting.HitFXs[mySurfaceType];
				// 	if (myParticle)
				// 	{
				// 		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), myParticle, FTransform(HitResult.ImpactNormal.Rotation(), HitResult.ImpactPoint, FVector(1.0f)));
				// 	}
				// }
				//
				// if (WeaponSetting.ProjectileSetting.HitSound)
				// {
				// 	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponSetting.ProjectileSetting.HitSound, HitResult.ImpactPoint);
				// }
				//
				// UTypes::AddEffectBySurfaceType(HitResult.GetActor(), HitResult.BoneName, ProjectileInfo.SurfaceEffect, mySurfaceType);
				
			}
			
			UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), WeaponSetting.ProjectileSetting.ProjectileDamage, HitResult.TraceStart, HitResult, GetInstigatorController(),this, NULL);
			
		}
	}

	if (GetWeaponRound() <= 0 && !bIsWeaponReloading)
	{
		if (CanWeaponReload())
		{
			InitReload();
		}
	}
}

void ASTWeapon_Default::SetWeaponStateFire(const bool bIsFiring)
{
	if (CheckWeaponCanFire())
	{
		bIsWeaponFiring = bIsFiring;
	}
	else
	{
		bIsWeaponFiring = false;
		FireTimer = 0.01f;
	}
}

void ASTWeapon_Default::DispersionTick(float DeltaTime)
{
	if (!bIsWeaponReloading)
	{
		if (!bIsWeaponFiring)
		{
			if (bShouldDispersionReducing)
				CurrentDispersion = CurrentDispersion - CurrentDispersionReduction;
			else
				CurrentDispersion = CurrentDispersion + CurrentDispersionReduction;
		}

		if (CurrentDispersion < CurrentDispersionMin)
		{
			CurrentDispersion = CurrentDispersionMin;
		}
		else
		{
			if (CurrentDispersion > CurrentDispersionMax)
			{
				CurrentDispersion = CurrentDispersionMax;
			}
		}
	}
}

void ASTWeapon_Default::ChangeDispersionByShot()
{
	CurrentDispersion = CurrentDispersion + CurrentDispersionRecoil;
}

float ASTWeapon_Default::GetCurrentDispersion() const
{
	return CurrentDispersion;
}

void ASTWeapon_Default::UpdateStateWeapon(EMovementStates NewMovementState)
{
	bIsFireBlocked = false;
	switch (NewMovementState)
	{
	case EMovementStates::Idle:
		CurrentDispersionMax = WeaponSetting.DispersionSettings.Find(EMovementStates::Idle)->DispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionSettings.Find(EMovementStates::Idle)->DispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionSettings.Find(EMovementStates::Idle)->DispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionSettings.Find(EMovementStates::Idle)->DispersionReduction;
		break;
	case EMovementStates::Running:
		CurrentDispersionMax = WeaponSetting.DispersionSettings.Find(EMovementStates::Running)->DispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionSettings.Find(EMovementStates::Running)->DispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionSettings.Find(EMovementStates::Running)->DispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionSettings.Find(EMovementStates::Running)->DispersionReduction;
		break;
	case EMovementStates::Sprinting:
		bIsFireBlocked = true;
		SetWeaponStateFire(false);
		break;
	default:
		break;
	}
}

FVector ASTWeapon_Default::ApplyDispersionToShoot(FVector DirectionShoot) const
{
	return FMath::VRandCone(DirectionShoot, GetCurrentDispersion() * PI / 180.f);
}

FVector ASTWeapon_Default::GetFireEndLocation() const
{
	const FVector EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(ShootLocation->GetForwardVector()) * WeaponSetting.DistanceTrace;
	return EndLocation;
}

void ASTWeapon_Default::ReloadTick(float DeltaTime)
{
	if (bIsWeaponReloading)
	{
		if (ReloadTimer < 0.f)
		{
			FinishReload();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::Yellow, FString::Printf(TEXT("ReloadTimer: %f"), ReloadTimer));
			ReloadTimer -= DeltaTime;
		}
	}
}

bool ASTWeapon_Default::CanWeaponReload()
{
	bool bResult = true;
	if (GetOwner())
	{
		UInventoryComponent* MyInv = Cast<UInventoryComponent>(GetOwner()->GetComponentByClass(UInventoryComponent::StaticClass()));
		if (MyInv)
		{
			int8 Ammo = 0;
			if (!MyInv->CheckAmmoForWeapon(WeaponSetting.WeaponType, Ammo))
			{
				bResult = false;
			}
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("WeaponCanReload")));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CanNotWeaponReload"));
	}

	return bResult;
}

void ASTWeapon_Default::InitReload()
{
	CurrentDispersion = 0.0f;
	bIsWeaponReloading = true;
	ReloadTimer = WeaponSetting.ReloadTime;

	if (WeaponSetting.AnimWeaponReload)
	{
		OnWeaponReloadStart.Broadcast();
		WeaponReloadStart_BP(WeaponSetting.AnimWeaponReload);
	}
}

int8 ASTWeapon_Default::GetAvailableAmmoForReload()
{
	int8 ResultAmmo = 0;
	if (GetOwner())
	{
		UInventoryComponent* MyInv = Cast<UInventoryComponent>(GetOwner()->GetComponentByClass(UInventoryComponent::StaticClass()));
		if (MyInv)
		{
			int8 AvailableAmmo = 0;
			if (MyInv->CheckAmmoForWeapon(WeaponSetting.WeaponType, AvailableAmmo))
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("AvailableAmmo: %i"), AvailableAmmo));
				ResultAmmo = AvailableAmmo;
			}
		}
	}
	return ResultAmmo;
}

void ASTWeapon_Default::FinishReload()
{
	bIsWeaponReloading = false;

	int32 AmmoInWeapon = GetWeaponRound();
	int8 AvailableAmmoForReload = GetAvailableAmmoForReload();
	int8 AmmoNeedTaken = WeaponSetting.MaxRound - AmmoInWeapon;
	int8 AmmoTaken;
	
	if (AvailableAmmoForReload >= AmmoNeedTaken)
	{
		AmmoTaken = AmmoNeedTaken;
	}
	else
	{
		AmmoTaken = AvailableAmmoForReload;
	}

	CurrentRound = AmmoInWeapon + AmmoTaken;
	
	OnWeaponReloadEnd.Broadcast(true, -AmmoTaken);
}

void ASTWeapon_Default::CancelReload()
{
	bIsWeaponReloading = false;
	if (SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
		SkeletalMeshWeapon->GetAnimInstance()->StopAllMontages(0.0f);

	OnWeaponReloadEnd.Broadcast(false, 0);
}

int32 ASTWeapon_Default::GetWeaponRound() const
{
	return CurrentRound;
}

FProjectileInfo ASTWeapon_Default::GetProjectile() const
{
	return WeaponSetting.ProjectileSetting;
}

void ASTWeapon_Default::WeaponReloadStart_BP_Implementation(UAnimMontage* WeaponAnimReload)
{
	
}
