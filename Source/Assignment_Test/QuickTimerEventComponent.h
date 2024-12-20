// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuickTimerEventComponent.generated.h"

class AMyCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASSIGNMENT_TEST_API UQuickTimerEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuickTimerEventComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Event, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	int32 QTE_TargetCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Event, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float QuicktimerTimeLimit;
private:
	AMyCharacter* MyCharacter;

	FTimerHandle QTE_TimerHandle;
	int32 QTE_Count;
	
	bool bQTEActive;

	void EndQTE();
	void ResetQTE();

public:
	void StartQTE();
	void HandleQTEKeyPress();

	FORCEINLINE bool IsQTEActive() const { return bQTEActive; }
};
