// Fill out your copyright notice in the Description page of Project Settings.


#include "STGameInstance.h"

bool USTGameInstance::GetWeaponInfoByName(const FName ItemName, const UDataTable* SourceDataTable, FWeaponInfo& OutInfo)
{
	bool bIsFind = false;

	if (SourceDataTable)
	{
		const FWeaponInfo* WeaponInfoRow = SourceDataTable->FindRow<FWeaponInfo>(ItemName, "", false);
		if (WeaponInfoRow)
		{
			bIsFind = true;
			OutInfo = *WeaponInfoRow;
		}
	}
	return bIsFind;
}