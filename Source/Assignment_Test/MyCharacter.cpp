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

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

void AMyCharacter::ActivateChainLightning()
{
	FVector StartLocation, EndLocation;

	// Perform line trace from the camera
	if (GetCameraTrace(StartLocation, EndLocation))
	{
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility))
		{
			// Spawn lightning effect at the hit location
			if (LightningEffect)
			{
				// Get the location of the character's feet (mesh location)
				FVector FootLocation = GetMesh()->GetSocketLocation(TEXT("FootSocket")); // You can replace "FootSocket" with an actual socket name if needed

				// Add an offset to the Z location (for example, 100 units above the feet)
				FVector OffsetLocation = FootLocation + FVector(0.f, 0.f, 100.f); // Adjust the Z value as needed

				// Attach Niagara to player with the offset location
				UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
					LightningEffect,
					GetMesh(), // Attach to the player's mesh
					NAME_None,
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::KeepRelativeOffset,
					true
				);

				if (NiagaraComp)
				{
					// Update the end location of the lightning effect
					NiagaraComp->SetNiagaraVariableVec3(TEXT("User.End"), HitResult.Location);
					NiagaraComp->SetWorldLocation(OffsetLocation); // Set the Niagara effect position to the offset location
				}
			}

			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				// Debug visuals
				DrawDebugLine(GetWorld(), StartLocation, HitResult.Location, FColor::Green, false, 2.0f, 0, 2.0f);
				DrawDebugSphere(GetWorld(), HitResult.Location, 50.0f, 12, FColor::Blue, false, 2.0f);

				// On-screen messages
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Actor: %s"), *HitActor->GetName()));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Start: %s, End: %s"), *StartLocation.ToString(), *HitResult.Location.ToString()));

				// Check if the hit actor is a ChainLightningAbility actor
				AChainLightningAbility* HitChainLightning = Cast<AChainLightningAbility>(HitActor);
				if (HitChainLightning)
				{
					// Trigger the chain lightning effect recursively
					HitChainLightning->PerformSphereTrace(); // This will call the sphere trace on the hit actor
				}

				AMyEnemy* HitEnemy = Cast<AMyEnemy>(HitActor);
				if (HitEnemy)
				{
					HitEnemy->PerformSphereTrace();
				}
			}
		}
		else
		{
			// Debug Line (No Hit)
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 2.0f);

			// On-screen message for no hit
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Hit Detected."));
		}
	}
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
	GetMesh()->bPauseAnims = true;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		DisableInput(PC);
	}
	
	UGameplayStatics::SetGamePaused(GetWorld(), true);
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


