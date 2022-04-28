// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ST/Weapons/STWeapon_Default.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChange, EWeaponType, WeaponType, int32, ChangeCount);

USTRUCT(BlueprintType)
struct FWeaponSlot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	TSubclassOf<ASTWeapon_Default> WeaponClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	FName WeaponName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* WeaponImage;
};

USTRUCT(BlueprintType)
struct FAmmoSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	EWeaponType WeaponType = EWeaponType::Rifle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 Count = 60;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 MaxCount = 120;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ST_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(EditAnywhere, BlueprintAssignable, BlueprintReadWrite, Category = "Weapon")
	FNoAmmo NoAmmo;
	UPROPERTY(EditAnywhere, BlueprintAssignable, BlueprintReadWrite, Category = "Weapon")
	FOnAmmoChange OnAmmoChange;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void WeaponAmmoChange(EWeaponType WeaponType, int32 AmmoTaken);
	bool CheckAmmoForWeapon(EWeaponType WeaponType, int8& AvailableAmmo);
	UFUNCTION(BlueprintCallable)
	bool CheckCanTakeAmmo(EWeaponType AmmoWeaponType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FAmmoSlot> AmmoSlots;

};
