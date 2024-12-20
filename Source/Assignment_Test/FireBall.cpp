// Fill out your copyright notice in the Description page of Project Settings.


#include "FireBall.h"
#include "MyCharacter.h"
#include "MyEnemy.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AFireBall::AFireBall() :
	LifeSpantime(10.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetGenerateOverlapEvents(true);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
	ParticleComponent->SetupAttachment(SphereCollision);
	// Create the projectile movement component and attach it to the sphere collision component
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereCollision);

	// Configure the projectile movement component's properties
	ProjectileMovementComponent->InitialSpeed = 2000.0f;   // Initial speed of the projectile
	ProjectileMovementComponent->MaxSpeed = 2000.0f;       // Max speed of the projectile
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // Rotate the projectile in the direction of travel
	ProjectileMovementComponent->bShouldBounce = false;    // Set to true if you want the projectile to bounce off surfaces
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f; // Disable gravity for the fireball if needed

}

// Called when the game starts or when spawned
void AFireBall::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AFireBall::OnOverlap);

	SetLifeSpan(LifeSpantime);
}

void AFireBall::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Enemy = Cast<AMyEnemy>(OtherActor);
	if (Enemy)
	{
		Enemy->GetHit(100);
		Destroy();
	}
	
	/*auto Character = Cast<AMyCharacter>(OtherActor);
	if (Character)
	{
		Character->GetHit();
		Destroy();
	}*/
}

// Called every frame
void AFireBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/** Function to initialize the velocity of the fireball in the given direction */
void AFireBall::FireInDirection(const FVector& ShootDirection)
{
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}