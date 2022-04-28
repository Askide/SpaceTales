// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::GetCurrentHealth()
{
	return Health;
}

void UHealthComponent::ChangeHealthValue(float ChangeValue)
{
	Health += ChangeValue;

	if (Health > 100.0f)
	{
		Health = 100.0f;
	}
	else
	{
		if (Health <= 0.0f)
		{
			OnDead.Broadcast();
		}
	}

	OnHealthChange.Broadcast(Health, ChangeValue);
}