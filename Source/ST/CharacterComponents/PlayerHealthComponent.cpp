// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthComponent.h"

void UPlayerHealthComponent::ChangeHealthValue(float ChangeValue)
{
	if (Shield > 0.0f && ChangeValue < 0.0f)
	{
		ChangeShieldValue(ChangeValue);

		if (Shield < 0.0f)
		{
			//FX
			UE_LOG(LogTemp, Warning, TEXT("UTPSCharacterHealthComponent::ChangeHealthValue - Sheild < 0"));
		}
	}
	else
	{
		Super::ChangeHealthValue(ChangeValue);
	}
}

float UPlayerHealthComponent::GetCurrentShield()
{
	return Shield;
}

void UPlayerHealthComponent::ChangeShieldValue(float ChangeValue)
{
	Shield += ChangeValue;

	if (Shield > 100.0f)
	{
		Shield = 100.0f;
	}
	else
	{
		if (Shield < 0.0f)
			Shield = 0.0f;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_CollDownShieldTimer, this, &UPlayerHealthComponent::CoolDownShieldEnd, CoolDownShieldRecoverTime, false);

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);
	}

	OnShieldChange.Broadcast(Shield, ChangeValue);
}

void UPlayerHealthComponent::CoolDownShieldEnd()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShieldRecoveryRateTimer, this, &UPlayerHealthComponent::RecoveryShield, ShieldRecoverRate, true);
	}
}

void UPlayerHealthComponent::RecoveryShield()
{
	float tmp = Shield;
	tmp = tmp + ShieldRecoverValue;
	if (tmp > 100.0f)
	{
		Shield = 100.0f;
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);
		}
	}
	else
	{
		Shield = tmp;
	}

	OnShieldChange.Broadcast(Shield, ShieldRecoverValue);
}
