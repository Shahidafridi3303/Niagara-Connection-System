// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaarcterAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

UChaarcterAnimInstance::UChaarcterAnimInstance() :
	Speed(0.f)
{
}

void UChaarcterAnimInstance::NativeInitializeAnimation()
{
	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
}

void UChaarcterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (MyCharacter == nullptr)
	{
		MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	}
	if (MyCharacter)
	{
		FVector Velocity{ MyCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		bIsFalling = MyCharacter->GetMovementComponent()->IsFalling();
	}
}
