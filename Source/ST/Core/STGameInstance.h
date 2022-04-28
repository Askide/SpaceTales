// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "ST/FunctionLibrary/STBPFunctionLibrary.h"
#include "STGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class ST_API USTGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " PickUp ")
	UDataTable* WeaponInfoTable = nullptr;
	
	static bool GetWeaponInfoByName(FName ItemName, const UDataTable* SourceDataTable,  FWeaponInfo& OutInfo);
};
