// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEnemy.h"

#include "Assignment_Test.h"
#include "EnemyWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "MyCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"

// Sets default values
AMyEnemy::AMyEnemy() :
	Health(100.f),
	MaxHealth(100.f),
	bStunned(false),
	SelfDamageValue(20.f),
	bDying(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	bHasPerformedSphereTrace = false;
	bIsProcessing = false;
}

// Called when the game starts or when spawned
void AMyEnemy::BeginPlay()
{
	Super::BeginPlay();
	EquipWeapon(SpawnDefaultWeapon());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::AgroSphereOverlap);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AMyEnemy::CombatRangeEndOverlap);

	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(
		GetActorTransform(),
		PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(
		GetActorTransform(),
		PatrolPoint2);

	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(
			TEXT("PatrolPoint"),
			WorldPatrolPoint);

		EnemyController->GetBlackboardComponent()->SetValueAsVector(
			TEXT("PatrolPoint2"),
			WorldPatrolPoint2);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}

	Tags.Add(FName("Enemy"));
}

AEnemyWeapon* AMyEnemy::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		// Spawn the Weapon
		return GetWorld()->SpawnActor<AEnemyWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AMyEnemy::EquipWeapon(AEnemyWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		// Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// Attach the Weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		EquippedWeapon = WeaponToEquip;
	}
}

void AMyEnemy::ActivateWeaponCollision()
{
	EquippedWeapon->GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMyEnemy::DeactivateWeaponCollision()
{
	EquippedWeapon->GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMyEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	auto Character = Cast<AMyCharacter>(OtherActor);
	if (Character)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(
			TEXT("Target"),
			Character);
	}

}

void AMyEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			TEXT("Stunned"),
			Stunned);
	}
}

void AMyEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	auto Character = Cast<AMyCharacter>(OtherActor);

	if (Character)
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}
	}
}

void AMyEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	auto Character = Cast<AMyCharacter>(OtherActor);

	if (Character)
	{
		bInAttackRange = false;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
		}
	}
}

void AMyEnemy::GetHit()
{
	if (bDying) return;

	if (Health - SelfDamageValue <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= SelfDamageValue;

		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		}

		PlayHitMontage();
		SetStunned(true);
	}
	if (ImpactPaticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPaticles, GetActorLocation(), FRotator(0.f), true);
	}
}

FName AMyEnemy::GetRandomAttackSectionName()
{
	FName SectionName;
	const int32 Section{ FMath::RandRange(1, 2) };
	switch (Section)
	{
	case 1:
		SectionName = FName(TEXT("Attack1"));
		break;
	case 2:
		SectionName = FName(TEXT("Attack2"));
		break;
	}
	return SectionName;
}

void AMyEnemy::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		FName SectionName = GetRandomAttackSectionName();
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyEnemy::FinishDeath()
{
	Destroy();
	EquippedWeapon->Destroy();
}

void AMyEnemy::PlayHitMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitMontage)
	{
		AnimInstance->Montage_Play(HitMontage);
	}
}

void AMyEnemy::Die()
{
	if (bDying) return;
	bDying = true;

	HideHealthBar();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Dead"), true);
		EnemyController->StopMovement();
	}
}

// Called to bind functionality to input
void AMyEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



//// ***** Niagara connection **** /////



void AMyEnemy::PerformSphereTrace()
{
	if (bHasPerformedSphereTrace || bIsProcessing)
	{
		return; // Avoid processing if the actor has already performed a trace or is in the middle of processing
	}

	bIsProcessing = true; // Mark as processing to prevent further recursive calls
	FTimerHandle TimerHandle;
	// Delay execution of the sphere trace by 1 second
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			FVector StartLocation = GetActorLocation();
			TArray<FOverlapResult> OverlapResults;
			FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(TraceRadius);

			bool bHit = GetWorld()->OverlapMultiByChannel(OverlapResults, StartLocation, FQuat::Identity, ECC_Visibility, CollisionSphere);

			if (bHit)
			{
				TArray<AActor*> ValidTargets;
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Found %d overlaps"), OverlapResults.Num()));

				for (auto& Result : OverlapResults)
				{
					AActor* OverlappedActor = Result.GetActor();
					if (OverlappedActor && OverlappedActor != this && !OverlappedActor->IsPendingKill())
					{
						// Check if the actor has the "ChainLightningTarget" tag
						if (OverlappedActor->ActorHasTag(FName("Enemy")))
						{
							ValidTargets.Add(OverlappedActor);
						}
					}
				}

				HandleOverlappingActors(ValidTargets);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No overlaps found"));
			}

			bHasPerformedSphereTrace = true;
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable collision to avoid further traces
		}, 1.0f, false); // Delay for 1 second before performing the sphere trace
}

void AMyEnemy::HandleOverlappingActors(const TArray<AActor*>& ValidTargets)
{
	if (ValidTargets.Num() == 0)
	{
		bIsProcessing = false; // Reset processing flag
		return;
	}

	AActor* NearestActor = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Target : ValidTargets)
	{
		float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			NearestActor = Target;
		}

	}

	if (NearestActor)
	{
		PerformNiagaraEffect(NearestActor);
	}
}

void AMyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Interpolate the endpoint over time
	if (bIsInterpolating && NiagaraComponent)
	{
		ElapsedTime += DeltaTime;
		if (ElapsedTime < InterpolationTime)
		{
			// Lerp the endpoint between StartLocation and TargetLocation
			FVector NewEndLocation = FMath::Lerp(StartLocation, TargetLocation, ElapsedTime / InterpolationTime);
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), NewEndLocation);
		}
		else
		{
			// Set the endpoint to the final target location when interpolation is complete
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), TargetLocation);
			bIsInterpolating = false; // Stop interpolation
		}
	}
}

void AMyEnemy::PerformNiagaraEffect(AActor* TargetActor)
{
	if (NiagaraEffect)
	{
		// Apply Z offset to the spawn location
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z += 100.0f; // Apply the Z offset here

		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraEffect, SpawnLocation);

		if (NiagaraComponent)
		{
			// Apply Z offset to the target location
			TargetLocation = TargetActor->GetActorLocation();
			TargetLocation.Z += 100.0f; // Apply the Z offset to the end location

			// Initialize the interpolation variables
			StartLocation = SpawnLocation;
			InterpolatedEnd = StartLocation;
			InterpolationTime = 1.0f; // 1 second for interpolation
			ElapsedTime = 0.0f;
			bIsInterpolating = true;

			// Set the initial endpoint
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), StartLocation);
		}
	}

	if (TargetActor->IsA(AMyEnemy::StaticClass()))
	{
		AMyEnemy* ChainTarget = Cast<AMyEnemy>(TargetActor);
		if (ChainTarget && !ChainTarget->bHasPerformedSphereTrace)
		{
			ChainTarget->PerformSphereTrace();
		}
	}

	bIsProcessing = false; // Reset processing flag after completing the chain lightning
}
