// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Assignment_Test.h"
#include "FireBall.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "QuickTimerEventComponent.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ChainLightningAbility.h"
#include "MyEnemy.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AMyCharacter::AMyCharacter() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	BaseMovementSpeed(350.f),
	SprintMovementSpeed(600.f),
	SprintButtonPressed(false),
	CombatState(ECombatState::ECS_Unoccupied),
	Health(100.f),
	MaxHealth(100.f),
	SelfDamageValue(20.f),
	QTEWidgetDisplayText(TEXT("Empty"))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = (CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	QuickTimerEventComponent = CreateDefaultSubobject<UQuickTimerEventComponent>(TEXT("QuickTimerEventComponent"));

	// Don't rotate when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	EquipWeapon(SpawnDefaultWeapon());
	CurrentCooldownTime = CooldownTime;
}

void AMyCharacter::MoveForward(float Value)
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AMyCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AMyCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * MouseHipTurnRate);
}

void AMyCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * MouseHipLookUpRate);
}

void AMyCharacter::ShiftButtonPressed()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintMovementSpeed;
}

void AMyCharacter::ShiftButtonReleased()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AMyCharacter::Jump()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	ACharacter::Jump();

	if (JumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
	}
}

AMyWeapon* AMyCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		// Spawn the Weapon
		return GetWorld()->SpawnActor<AMyWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AMyCharacter::EquipWeapon(AMyWeapon* WeaponToEquip)
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


// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyCharacter::ShiftButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyCharacter::ShiftButtonReleased);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::AttackButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyCharacter::FireButtonPressed);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::InteractButtonPressed);
}


// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (NextEnemy)
	{
		NiagaraComp->SetNiagaraVariableVec3(TEXT("User.End"), NextEnemy->GetActorLocation() + FVector(0.f, 0.f, 10.f)); // Adjust Z if needed
	}

	if (bCanUseNiagara == false)  // Cooldown is active
	{
		CurrentCooldownTime += DeltaTime;  // Increase the cooldown time

		// If cooldown is finished, reset everything
		if (CurrentCooldownTime >= CooldownTime)
		{
			CurrentCooldownTime = CooldownTime;  // Make sure it doesn't exceed the cooldown time

			// Optionally notify the player that the ability is ready again
			UE_LOG(LogTemp, Warning, TEXT("Ability is ready!"));
		}

		// Map CurrentCooldownTime from range [0, CooldownTime] to [0, 1]
		CooldownProgress = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, CooldownTime), FVector2D(1.0f, 0.0f), CurrentCooldownTime);
	}
	else  // Cooldown is over
	{
		CurrentCooldownTime = 0.0f;  // Reset cooldown timer to 0
		CooldownProgress = 1.0f;  // Set progress to 0 when it's available again
	}
}


void AMyCharacter::ActivateChainLightning()
{
	if (!bCanUseNiagara)
	{
		return;  // Return early if the cooldown is active
	}

	FVector StartLocation, EndLocation;

	// Perform line trace from the camera
	if (GetCameraTrace(StartLocation, EndLocation))
	{
		FHitResult HitResult;
		bool bHitValidTarget = false;

		// Perform the line trace
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility))
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				// Debug visuals
				DrawDebugSphere(GetWorld(), HitResult.Location, 50.0f, 12, FColor::Blue, false, 2.0f);

				// Check if the hit actor is a ChainLightningAbility actor
				AChainLightningAbility* HitChainLightning = Cast<AChainLightningAbility>(HitActor);
				if (HitChainLightning)
				{
					HitChainLightning->PerformSphereTrace(); // Trigger the sphere trace on the hit actor
					bHitValidTarget = true;
				}

				AMyEnemy* HitEnemy = Cast<AMyEnemy>(HitActor);
				if (HitEnemy)
				{
					HitEnemy->PerformSphereTrace();
					HitEnemy->ApplyLightningDamage();
					NextEnemy = Cast<AMyEnemy>(HitEnemy);
					bHitValidTarget = true;
				}
			}
		}

		// Spawn lightning effect at the player's location
		if (LightningEffect)
		{
			FVector FootLocation = GetMesh()->GetSocketLocation(TEXT("FootSocket"));
			FVector OffsetLocation = FootLocation + FVector(0.f, 0.f, 100.f);

			NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				LightningEffect,
				GetMesh(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true
			);

			if (NiagaraComp)
			{
				NiagaraComp->SetNiagaraVariableVec3(TEXT("User.End"), EndLocation);
				NiagaraComp->SetWorldLocation(OffsetLocation);
			}
		}

		if (bHitValidTarget)
		{
			bCanUseNiagara = false;
			// Set a timer to reset the cooldown after CooldownTime seconds
			GetWorld()->GetTimerManager().SetTimer(NiagaraCooldownTimerHandle, this, &AMyCharacter::ResetNiagaraCooldown, CooldownTime, false);
		}

		// If no valid target is hit, schedule deactivation
		if (!bHitValidTarget && NiagaraComp && NiagaraComp->IsActive())
		{
			CooldownTime = 1.0f;
			bCanUseNiagara = false;

			FTimerHandle DeactivateTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				DeactivateTimerHandle,
				[this]()
				{
					if (NiagaraComp)
					{
						bCanUseNiagara = true;
						NiagaraComp->Deactivate();
						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Deactivating Niagara after 1 second (no valid target)."));
					}
				},
				1.0f,
				false // Do not loop
			);
		}
	}
}

void AMyCharacter::ResetNiagaraCooldown()
{
	bCanUseNiagara = true;  // Cooldown is over, allow Niagara to be triggered again
	CurrentCooldownTime = CooldownTime; // Reset cooldown time to initial value
}

void AMyCharacter::ResetChainLightning()
{
	for (AMyEnemy* Enemy : AffectedEnemies)
	{
		if (Enemy)
		{
			if (Enemy->NiagaraComponent)
			{
				Enemy->NiagaraComponent->Deactivate();
			}
			Enemy->GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			Enemy->bHasPerformedSphereTrace = false;  // Reset trace flag
			Enemy->bIsProcessing = false;            // Reset processing flag
		}
	}

	if (NiagaraComp)
	{
		NiagaraComp->Deactivate();
	}
	NextEnemy = nullptr;
	AffectedEnemies.Empty(); // Clear the array

	// Reset chain-related variables
	ChainCount = 0;

	// Debug message
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Chain Lightning Reset"));
}


bool AMyCharacter::GetCameraTrace(FVector& OutStart, FVector& OutEnd)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController) return false;

	FVector WorldLocation, WorldDirection;

	// Get screen midpoint and convert to world direction
	if (PlayerController->DeprojectScreenPositionToWorld(
		GEngine->GameViewport->Viewport->GetSizeXY().X / 2.0f,
		GEngine->GameViewport->Viewport->GetSizeXY().Y / 2.0f,
		WorldLocation,
		WorldDirection))
	{
		OutStart = PlayerController->PlayerCameraManager->GetCameraLocation();
		OutEnd = OutStart + (WorldDirection * LineTraceDistance);
		return true;
	}

	return false;
}

void AMyCharacter::InteractButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (QuickTimerEventComponent && QuickTimerEventComponent->IsQTEActive())
	{
		QuickTimerEventComponent->HandleQTEKeyPress();
	}
}

void AMyCharacter::FinishDeath()
{
	// die logic
}

void AMyCharacter::GetHit()
{
	if (Health == 0.f) return;

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
	}

	if (ImpactPaticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPaticles, GetActorLocation(), FRotator(0.f), true);
	}
}

void AMyCharacter::AttackButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	CombatState = ECombatState::ECS_AttackTimerInProgress;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		FName SectionName = GetRandomAttackSectionName();
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyCharacter::FireButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	//CombatState = ECombatState::ECS_FireTimerInProgress;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);
	}

	ActivateChainLightning();
}

void AMyCharacter::AnimationFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

FName AMyCharacter::GetRandomAttackSectionName()
{
	FName SectionName;
	const int32 Section{ FMath::RandRange(1, 4) };
	switch (Section)
	{
	case 1:
		SectionName = FName(TEXT("Attack1"));
		break;
	case 2:
		SectionName = FName(TEXT("Attack2"));
		break;
	case 3:
		SectionName = FName(TEXT("Attack3"));
		break;
	}
	return SectionName;
}

void AMyCharacter::ActivateWeaponCollision()
{
	EquippedWeapon->GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMyCharacter::DeactivateWeaponCollision()
{
	EquippedWeapon->GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMyCharacter::PlayHitMontage()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	CombatState = ECombatState::ECS_Stunned;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitMontage)
	{
		AnimInstance->Montage_Play(HitMontage);
	}
}

void AMyCharacter::Die()
{
	CombatState = ECombatState::ECS_Dead;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AMyCharacter::AddToChain()
{
    // Increment the chain count when a chain event happens
    ChainCount++;
    // Optionally, log for debugging
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ChainCount: %d"), ChainCount));
}

