// Fill out your copyright notice in the Description page of Project Settings.


#include "ST/Weapons/Projectile/STProjectile_Default.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASTProjectile_Default::ASTProjectile_Default()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

	BulletCollisionSphere->SetSphereRadius(16.f);
	
	//hit event return physMaterial
	BulletCollisionSphere->bReturnMaterialOnMove = true;

	BulletCollisionSphere->SetCanEverAffectNavigation(false);//collision not affect navigation

	RootComponent = BulletCollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCanEverAffectNavigation(false);

	BulletFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Bullet FX"));
	BulletFX->SetupAttachment(BulletMesh);

	BulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet ProjectileMovement"));
	BulletProjectileMovement->UpdatedComponent = RootComponent;
	BulletProjectileMovement->InitialSpeed = 1.f;
	BulletProjectileMovement->MaxSpeed = 0.f;

	BulletProjectileMovement->bRotationFollowsVelocity = true;
	BulletProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void ASTProjectile_Default::BeginPlay()
{
	Super::BeginPlay();
	
	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &ASTProjectile_Default::BulletCollisionSphereHit);
	BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASTProjectile_Default::BulletCollisionSphereBeginOverlap);
	BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ASTProjectile_Default::BulletCollisionSphereEndOverlap);

}

// Called every frame
void ASTProjectile_Default::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASTProjectile_Default::InitProjectile(FProjectileInfo InitParam)
{
	BulletProjectileMovement->InitialSpeed = InitParam.ProjectileInitSpeed;
	BulletProjectileMovement->MaxSpeed = InitParam.ProjectileInitSpeed;
	this->SetLifeSpan(InitParam.ProjectileLifeTime);

	ProjectileSetting = InitParam;

	if (ProjectileSetting.BulletFX)
	{
		BulletFX->SetTemplate(ProjectileSetting.BulletFX);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("FX DELETED")));
		BulletFX->DestroyComponent(true);
	}

	if (ProjectileSetting.BulletMesh)
	{
		BulletMesh->SetStaticMesh(ProjectileSetting.BulletMesh);
	}
	else
	{
		BulletMesh->DestroyComponent(true);
	}
}

void ASTProjectile_Default::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileSetting.ProjectileDamage, Hit.TraceStart, Hit, GetInstigatorController(), this, NULL);
	ImpactProjectile();
}

void ASTProjectile_Default::ImpactProjectile()
{
	this->Destroy();
}