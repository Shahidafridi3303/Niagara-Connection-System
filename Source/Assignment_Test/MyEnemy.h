// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyEnemy.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class ASSIGNMENT_TEST_API AMyEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AEnemyWeapon> DefaultWeaponClass;
	FTimerHandle ChainTimerHandle;

	AEnemyWeapon* SpawnDefaultWeapon();

	void EquipWeapon(AEnemyWeapon* WeaponToEquip);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactPaticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	UFUNCTION(BlueprintCallable)
	void ActivateWeaponCollision();
	UFUNCTION(BlueprintCallable)
	void DeactivateWeaponCollision();

	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	UFUNCTION()
	void CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AEnemyWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	class AEnemyController* EnemyController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;

	FName GetRandomAttackSectionName();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ApplyLightningDamage();

	//float CalculateChainDamage(int32 ChainIndex, EDecayModel DecayModel, float InitialDamage, float DecayFactor, float MinimumDamage, float ReductionPerChain);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float SelfDamageValue;

	void PlayHitMontage();
	void Die();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetHit(int32 DamageValue);

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }


	/// ******  Niagara conenction system  ****///

	void PerformSphereTrace();
	void HandleOverlappingActors(const TArray<AActor*>& ValidTargets);
	void PerformNiagaraEffect(AActor* TargetActor);
	void ResetCollision();

	// Flag to ensure the actor doesn't perform the sphere trace more than once
	bool bHasPerformedSphereTrace;

	// Used to prevent actors from interacting with each other recursively
	bool bIsProcessing;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float TraceRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* NiagaraEffect;

	FTimerHandle TimerHandle_ResetCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float InitialDamage = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float DecayFactor = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	int32 ChainLimit = 3;

	// Declare these variables in the class header
private:
	FVector StartLocation;
	FVector TargetLocation;
	FVector InterpolatedEnd;
	float InterpolationTime;
	float ElapsedTime;
	UNiagaraComponent* NiagaraComponent;
	bool bIsInterpolating;

	AMyEnemy* NextEnemy;

	int32 ChainDelay = 1;

	class AMyCharacter* PlayerCharacter;

	public:
		// Function to validate if there's a clear line of sight to the target
		bool IsLineOfSightClear(AActor* PotentialTarget);

private:
	// Debug visualization toggle for line traces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowLineTrace = true;

	// Line trace parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line Trace", meta = (AllowPrivateAccess = "true"))
	float TraceHeightOffset = 50.f; // Offset from the ground for the trace

};
