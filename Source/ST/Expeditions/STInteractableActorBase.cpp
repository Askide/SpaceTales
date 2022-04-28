// Fill out your copyright notice in the Description page of Project Settings.


#include "ST/Expeditions/STInteractableActorBase.h"

// Sets default values
ASTPickUpActorBase::ASTPickUpActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASTPickUpActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASTPickUpActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

