// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::WeaponAmmoChange(EWeaponType WeaponType, int32 AmmoTaken)
{
	bool bIsFind = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() &&!bIsFind)
	{
		if (AmmoSlots[i].WeaponType == WeaponType)
		{
			AmmoSlots[i].Count += AmmoTaken;
			if (AmmoSlots[i].Count > AmmoSlots[i].MaxCount)
			{
				AmmoSlots[i].Count = AmmoSlots[i].MaxCount;
			}
			else 
			{
				if (AmmoSlots[i].Count < 0)
				{
					AmmoSlots[i].Count = 0;
				}
			}
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("ResultCount: %i, AmmoTaken: %i"), AmmoSlots[i].Count, AmmoTaken));
			OnAmmoChange.Broadcast(AmmoSlots[i].WeaponType, AmmoSlots[i].Count);

			bIsFind = true;
		}
		i++;
	}

}
//
//void UTPSInventoryComponent::WeaponAdditionalInfoChange(FAdditionalWeaponInfo NewInfo)
//{
//	OnWeaponAdditionalInfoChange.Broadcast(NewInfo);
//}

bool UInventoryComponent::CheckAmmoForWeapon(EWeaponType WeaponType, int8 &AvailableAmmo)
{
	AvailableAmmo = 0;
	for (int i = 0; i < AmmoSlots.Num(); i++)
	{
		if (AmmoSlots[i].WeaponType == WeaponType)
		{
			if (AmmoSlots[i].Count > 0)
			{
				AvailableAmmo = AmmoSlots[i].Count;
				return true;
			}
		}
	}

	NoAmmo.Broadcast();

	return false;
}

bool UInventoryComponent::CheckCanTakeAmmo(EWeaponType AmmoWeaponType)
{
	bool result = false;
	int8 i = 0;

	while (i < AmmoSlots.Num() && !result)
	{
		if (AmmoSlots[i].WeaponType == AmmoWeaponType && AmmoSlots[i].Count < AmmoSlots[i].MaxCount)
		{
			result = true;
		}
		i++;
	}
	return result;
}


