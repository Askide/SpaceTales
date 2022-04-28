// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ST/CharacterComponents/PlayerHealthComponent.h"
#include "ST/CharacterComponents/InventoryComponent.h"
#include "ST/Core/STGameInstance.h"
#include "STCharacter.generated.h"

UCLASS(Blueprintable)
class ASTCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASTCharacter();
	
	

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	void SetupPlayerInputComponent(UInputComponent* NewInputComponent);
	
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() const { return CurrentCursor; }
	
private:

	virtual void BeginPlay() override;
	
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UPlayerHealthComponent* HealthComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UInventoryComponent* InventoryComponent;



public:

	bool bIsAlive = true;
	/** A decal that projects to the cursor location. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* CursorMaterial = nullptr;

	UDecalComponent* CurrentCursor = nullptr;
	
	// Character movement
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EMovementStates CurrentMovementState = EMovementStates::Idle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<ASTWeapon_Default*> BackpackWeapons;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ASTWeapon_Default* CurrentWeapon = nullptr;
	
	void SprintEnable();
	void SprintDisable();

	void BackpackInit();
	UFUNCTION(BlueprintCallable)
	void EquipNewWeapon(ASTWeapon_Default* NewWeapon);
	
	void AttackPressed();
	void AttackReleased();
	void AttackCharEvent(bool bIsFiring);
	UFUNCTION()
	void CharAttack();
	
	void TryReloadWeapon();
	UFUNCTION()
	void WeaponReloadStart();
	UFUNCTION()
	void WeaponReloadEnd(bool bIsSuccess, int32 AmmoInWeapon);

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                 AActor* DamageCauser);
	UFUNCTION()
	void CharDead();

	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadStart_BP(UAnimMontage* CharAnim);
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadEnd_BP(bool bIsSuccess);
	UFUNCTION(BlueprintNativeEvent)
	void CharAttack_BP(UAnimMontage* CharAnim);
	UFUNCTION(BlueprintNativeEvent)
	void CharDead_BP(UAnimMontage* CharAnim);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	FCharacterActionAnim CharacterAnimations;
};




