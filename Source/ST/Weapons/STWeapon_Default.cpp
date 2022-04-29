// Fill out your copyright notice in the Description page of Project Settings.


#include "STWeapon_Default.h"

#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/STProjectile_Default.h"
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

	FireTick(DeltaTime);	
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
			if (WeaponSetting.bIsReloadable)
			{
				FireTimer -= DeltaTime;
			}
		}
	}	
}

bool ASTWeapon_Default::CheckWeaponCanFire() const
{
	return !bIsFireBlocked && GetWeaponRound() > 0;
}

void ASTWeapon_Default::Fire()
{
	FireTimer = WeaponSetting.RateOfFire;
	if (WeaponSetting.bIsReloadable)
	{
		CurrentRound = CurrentRound - 1;	
	}
	ChangeDispersionByShot();

	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());
	
	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();

		FVector EndLocation = GetFireEndLocation();
		FVector Dir = EndLocation - SpawnLocation;
		Dir.Normalize();
		
		FMatrix MyMatrix(Dir, FVector(0, 1, 0), FVector(0, 0, 1), FVector::ZeroVector);
		SpawnRotation = MyMatrix.Rotator();
		
		OnFire.Broadcast();
		if (WeaponSetting.Projectile)
		{
			//Projectile Init Ballistic Fire

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();

			ASTProjectile_Default* MyProjectile = Cast<ASTProjectile_Default>(GetWorld()->SpawnActor(WeaponSetting.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (MyProjectile)
			{
				MyProjectile->ProjectileSetting = ProjectileInfo;
				MyProjectile->InitProjectile(MyProjectile->ProjectileSetting);
			}
		}
		else
		{
			FHitResult HitResult;

			FVector TraceStartPoint = SpawnLocation;
			FVector TraceEndPoint = EndLocation;
		
			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartPoint, TraceEndPoint, ECC_WorldStatic))
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("TraceWork")));
				if(HitResult.GetActor() && HitResult.PhysMaterial.IsValid())
				{
					if (ProjectileInfo.HitSound)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileInfo.HitSound, HitResult.ImpactPoint);
					}
				}
			
				UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), ProjectileInfo.ProjectileDamage, HitResult.TraceStart, HitResult, GetInstigatorController(),this, NULL);
			
			}
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

FProjectileInfo ASTWeapon_Default::GetProjectile()
{
	return WeaponSetting.ProjectileSetting;
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
		FireTimer = -0.01f;
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
	const FVector EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(ShootLocation->GetForwardVector()) * 2000.0f;
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
