// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ST/FunctionLibrary/STBPFunctionLibrary.h"
#include "STWeapon_Default.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponReloadStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadEnd, bool, bIsSuccess, int32, AmmoInWeapon);

USTRUCT(BlueprintType)
struct FCharacterActionAnim
{
	GENERATED_BODY()
	
	// Weapon dispersion by movement state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	UAnimMontage* CharacterAttack = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	UAnimMontage* CharacterReload = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	TArray<UAnimMontage*> CharacterHit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	TArray<UAnimMontage*> CharacterDeath;
};

UCLASS()
class ST_API ASTWeapon_Default : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASTWeapon_Default();

	FOnWeaponReloadStart OnWeaponReloadStart;
	FOnWeaponReloadEnd OnWeaponReloadEnd;
	UPROPERTY(EditAnywhere, BlueprintAssignable, BlueprintReadWrite, Category = "Weapon")
	FOnFire OnFire;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USkeletalMeshComponent* SkeletalMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UStaticMeshComponent* StaticMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* ShootLocation = nullptr;

	UPROPERTY()
	FWeaponInfo WeaponSetting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	FName WeaponName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Spawn")
	FName WeaponBackpackSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Spawn")
	FName WeaponHandsSlot;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon Stats")
	EWeaponType WeaponType = EWeaponType::Rifle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 CurrentRound = 0;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void WeaponInit() const;

	void FireTick(float DeltaTime);
	bool CheckWeaponCanFire() const;
	void Fire();
	UFUNCTION(BlueprintCallable)
	void SetWeaponStateFire(bool bIsFiring);
	
	bool bIsFireBlocked = false;
	float FireTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
	bool bIsWeaponFiring = false;
	
	void DispersionTick(float DeltaTime);
	void ChangeDispersionByShot();
	float GetCurrentDispersion() const;
	void UpdateStateWeapon(EMovementStates NewMovementState);
	FVector ApplyDispersionToShoot(FVector DirectionShoot)const;
	FVector GetFireEndLocation()const;
	
	bool bShouldDispersionReducing = false;
	float CurrentDispersion = 0.0f;
	float CurrentDispersionMax = 1.0f;
	float CurrentDispersionMin = 0.1f;
	float CurrentDispersionRecoil = 0.1f;
	float CurrentDispersionReduction = 0.1f;
	FVector ShootEndLocation = FVector(0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float SizeVectorToChangeShootDirectionLogic = 100.0f;
	
	void ReloadTick(float DeltaTime);
	bool CanWeaponReload();
	void InitReload();
	int8 GetAvailableAmmoForReload();
	void FinishReload();
	void CancelReload();
	UFUNCTION(BlueprintCallable)
	int32 GetWeaponRound() const;
	
	bool bIsReloadBlocked = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	bool bIsWeaponReloading = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	float ReloadTimer = 0.0f;
	
	FProjectileInfo GetProjectile() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bShowDebug = true;

	UFUNCTION(BlueprintNativeEvent)
		void WeaponReloadStart_BP(UAnimMontage* WeaponAnimReload);
	
	
	
};
