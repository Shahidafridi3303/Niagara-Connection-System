// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "MyEnemy.h"

void UEnemyAnimInstance::UpdateAnimationProperties(float DeltaSeconds)
{
	if (Enemy == nullptr)
	{
		Enemy = Cast<AMyEnemy>(TryGetPawnOwner());
	}

	if (Enemy)
	{
		FVector Velocity{ Enemy->GetVelocity() };
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
