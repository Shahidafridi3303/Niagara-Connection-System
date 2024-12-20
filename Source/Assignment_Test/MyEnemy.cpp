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
	Health(1000.f),
	MaxHealth(1000.f),
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

	// Initialize the static reference to null
	NextEnemy = nullptr;
}

// Called when the game starts or when spawned
void AMyEnemy::BeginPlay()
{
	Super::BeginPlay();
	EquipWeapon(SpawnDefaultWeapon());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::AgroSphereOverlap);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AMyEnemy::CombatRangeEndOverlap);

	//GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

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

void AMyEnemy::GetHit(int32 DamageValue)
{
	FTimerHandle TimerHandle2;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle2, [this, DamageValue]()
		{
			PlayerCharacter = Cast<AMyCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

			if (PlayerCharacter)
			{
				// Call AddToChain on the player character to increment the chain count
				PlayerCharacter->AddToChain();
			}

			if (bDying) return;

			if (Health - DamageValue <= 0.f)
			{
				Health = 0.f;
				Die();
			}
			else
			{
				Health -= DamageValue;

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
			
		}, ChainDelay, false);
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
	// Ensure we don't process again if already done or currently processing
	if (bHasPerformedSphereTrace || bIsProcessing)
	{
		return;
	}

	bIsProcessing = true;  // Mark as processing
	FTimerHandle TimerHandle;

	// Disable capsule collision for this enemy to prevent it from being considered in subsequent traces
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

				for (auto& Result : OverlapResults)
				{
					AActor* OverlappedActor = Result.GetActor();
					if (OverlappedActor && OverlappedActor != this && !OverlappedActor->IsPendingKill())
					{
						if (OverlappedActor->ActorHasTag(FName("Enemy")))
						{
							ValidTargets.Add(OverlappedActor);
						}
					}
				}

				if (ValidTargets.Num() > 0)
				{
					HandleOverlappingActors(ValidTargets);
				}
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No overlaps found"));
			}

			// Re-enable capsule collision for this enemy
			//AgroSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			// Reset trace state
		}, ChainDelay, false);
}

void AMyEnemy::HandleOverlappingActors(const TArray<AActor*>& ValidTargets)
{
	if (ValidTargets.Num() == 0)
	{
		bIsProcessing = false;  // Reset processing flag
		return;
	}

	AActor* NearestActor = nullptr;
	float MinDistance = FLT_MAX;

	// Find the closest target
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
		// Set the global reference to the nearest enemy
		NextEnemy = Cast<AMyEnemy>(NearestActor);

		PerformNiagaraEffect(NearestActor);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No nearest actor found"));
	}
}

void AMyEnemy::PerformNiagaraEffect(AActor* TargetActor)
{
	if (NiagaraEffect)
	{
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z += 10.0f;

		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraEffect, SpawnLocation);

		if (NiagaraComponent)
		{
			// Initialize the start and target locations
			TargetLocation = TargetActor->GetActorLocation();
			TargetLocation.Z += 10.0f;

			StartLocation = SpawnLocation;
			InterpolatedEnd = StartLocation;
			InterpolationTime = 1.0f;
			ElapsedTime = 0.0f;
			bIsInterpolating = true;

			// Set the initial Niagara variable for the end location
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), StartLocation);
		}
	}

	// Debug: Check if the actor is of type AMyEnemy
	if (TargetActor->IsA(AMyEnemy::StaticClass()))
	{
		AMyEnemy* ChainTarget = Cast<AMyEnemy>(TargetActor);
		if (ChainTarget && !ChainTarget->bHasPerformedSphereTrace)
		{
			// Debug: Ensure we're triggering the next trace

			// Reset state for the next enemy (bHasPerformedSphereTrace should be false to allow it to perform trace)
			ChainTarget->bHasPerformedSphereTrace = false;  // Ensure it's ready for the next trace

			// Debug: Check if the next trace is being scheduled

			ChainTarget->PerformSphereTrace();
			ChainTarget->GetHit(10);
			// Trigger next trace
		}
		
	}

	// Reset the flags after the effect is performed
	bIsProcessing = false;  // Reset processing flag
	bHasPerformedSphereTrace = true;  // Mark this enemy as having performed the trace
}


void AMyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInterpolating && NiagaraComponent)
	{
		ElapsedTime += DeltaTime;

		// Calculate new end location based on interpolation
		if (ElapsedTime < InterpolationTime)
		{
			// Interpolate between the start and target location
			FVector NewEndLocation = FMath::Lerp(StartLocation, TargetLocation, ElapsedTime / InterpolationTime);
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), NewEndLocation);
		}
		else
		{
			// Set the final target location
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), TargetLocation);
			bIsInterpolating = false;
		}
	}

	// Always update the Niagara effect to follow the enemy's position
	if (NiagaraComponent)
	{
		NiagaraComponent->SetWorldLocation(GetActorLocation() + FVector(0.f, 0.f, 10.f)); // Adjust Z if needed
		if (NextEnemy)
		{
			NiagaraComponent->SetNiagaraVariableVec3(TEXT("User.End"), NextEnemy->GetActorLocation() + FVector(0.f, 0.f, 10.f)); // Adjust Z if needed
		}
	}
}

//float AMyEnemy::CalculateChainDamage(int32 ChainIndex, EDecayModel DecayModel, float InitialDamage, float DecayFactor, float MinimumDamage, float ReductionPerChain)
//{
//	switch (DecayModel)
//	{
//	case EDecayModel::Logarithmic:
//		return InitialDamage / FMath::Log2(ChainIndex + 2);
//	case EDecayModel::Exponential:
//		return FMath::Max(InitialDamage * FMath::Pow(DecayFactor, ChainIndex - 1), MinimumDamage);
//	case EDecayModel::Linear:
//		return FMath::Max(InitialDamage - (ReductionPerChain * (ChainIndex - 1)), MinimumDamage);
//	case EDecayModel::Randomized:
//		return FMath::Max(InitialDamage * FMath::Pow(DecayFactor, ChainIndex - 1) * FMath::FRandRange(0.8f, 1.2f), MinimumDamage);
//	default:
//		return InitialDamage;
//	}
}
