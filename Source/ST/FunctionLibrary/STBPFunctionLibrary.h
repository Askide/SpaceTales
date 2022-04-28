// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "STBPFunctionLibrary.generated.h"


UCLASS()
class ST_API USTBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
};


UENUM(BlueprintType)
enum class EMovementStates: uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
};

UENUM(BlueprintType)
enum class EWeaponType: uint8
{
	Pistol UMETA(DisplayName = "Pistol"),
	Rifle UMETA(DisplayName = "Rifle"),
};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()
	
	// Weapon dispersion by movement state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float DispersionMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float DispersionMin = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float DispersionRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float DispersionReduction = 0.4f;
};

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	UStaticMesh* BulletMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileLifeTime = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileInitSpeed = 2000.0f;

	// vfx trace on spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileFX")
	UParticleSystem* BulletFX = nullptr;
	// sfx on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileFX")
	USoundBase* HitSound = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	// weapon base params
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::Rifle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float RateOfFire = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 MaxRound = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsReloadable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float ReloadTime = 2.0f;
	
	// hit params on projectile spawn failed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float WeaponDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float DistanceTrace = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	UDecalComponent * DecalOnHit = nullptr;
	
	// weapon fx when shoot released 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShootFX")
	USoundBase* SoundFireWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShootFX")
	UParticleSystem* EffectFireWeapon = nullptr;

	// weapon fx when its reloading 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadFX")
	USoundBase* SoundReloadWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadFX")
	UAnimMontage* AnimWeaponReload = nullptr;

	// weapon projectile info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	TSubclassOf<class ASTProjectile_Default> Projectile = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FProjectileInfo ProjectileSetting;
	
	// weapon dispersion check by movement state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	TMap<EMovementStates, FWeaponDispersion> DispersionSettings;

	
};





