// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_AttackTimerInProgress UMETA(DisplayName = "AttackTimerInProgress"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),
	ECS_Dead UMETA(DisplayName = "Dead"),

	ECS_NAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ASSIGNMENT_TEST_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	class UQuickTimerEventComponent* QuickTimerEventComponent;

public:
	// Sets default values for this character's properties
	AMyCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate  This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to look up/down at a given rate.
	* @param Rate  This is a normalized rate, i.e. 1.0 means 100% of desired rate
	*/
	void LookUpAtRate(float Rate);

	/**
	* Rotate controller based on mouse X movement
	* @param Value   The input value from mouse movement
	*/
	void Turn(float Value);

	/**
	* Rotate controller based on mouse Y movement
	* @param Value   The input value from mouse movement
	*/
	void LookUp(float Value);

	void ShiftButtonPressed();
	void ShiftButtonReleased();

	virtual void Jump() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AMyWeapon> DefaultWeaponClass;

	AMyWeapon* SpawnDefaultWeapon();

	void EquipWeapon(AMyWeapon* WeaponToEquip);

	void AttackButtonPressed();

	void FireButtonPressed();

	void InteractButtonPressed();

	UFUNCTION(BlueprintCallable)
	void AnimationFinished();

	FName GetRandomAttackSectionName();

	UFUNCTION(BlueprintCallable)
	void ActivateWeaponCollision();
	UFUNCTION(BlueprintCallable)
	void DeactivateWeaponCollision();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactPaticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	void PlayHitMontage();
	void Die();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float SprintMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool SprintButtonPressed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AMyWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float SelfDamageValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	USoundCue* JumpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

public:
	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString QTEWidgetDisplayText;

	UFUNCTION(BlueprintImplementableEvent)
	void CreateQTE_Widget();

	/** Returns CameraBoom subobject **/
	FORCEINLINE  USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE  UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE UQuickTimerEventComponent* GetQuickTimerEventComponent() const { return QuickTimerEventComponent; }
	void GetHit();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float LineTraceDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AChainLightningAbility> ChainLightningAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* LightningEffect;

	void ActivateChainLightning();
	void ResetNiagaraCooldown();
	void AutoDeactivateNiagara();
	void ResetChainLightning();
	void DeactivateNiagaraAndEnableCollision();
	bool GetCameraTrace(FVector& OutStart, FVector& OutEnd);

	class AMyEnemy* NextEnemy;

	class UNiagaraComponent* NiagaraComp;

	// Integer variable to store the chain count
	UPROPERTY(BlueprintReadWrite, Category = "Chain")
	int32 ChainCount;

	// Function to update the chain count
	UFUNCTION(BlueprintCallable, Category = "Chain")
	void AddToChain();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Chain Lightning")
	TArray<AMyEnemy*> AffectedEnemies;

	bool bCanUseNiagara = true;  // Flag to check if Niagara can be triggered
	FTimerHandle NiagaraCooldownTimerHandle;  // Timer to manage cooldown

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float CooldownTime = 5.0f;  // Cooldown period in seconds (5 seconds)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float CurrentCooldownTime = 0.0f; // Timer for the current cooldown

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float MillisecondsPerTick = 0.05f; // For updating every 50ms or adjust to your preference

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain Lightning", meta = (AllowPrivateAccess = "true"))
	float CooldownProgress = 0.0f;  // Progress from 0 to 1, starts at 1 because cooldown starts from full
};
