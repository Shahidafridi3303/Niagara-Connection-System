#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChainLightningAbility.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UStaticMeshComponent;

UCLASS()
class ASSIGNMENT_TEST_API AChainLightningAbility : public AActor
{
    GENERATED_BODY()

public:
    AChainLightningAbility();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

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
    float TraceRadius = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StaticMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
    UNiagaraSystem* NiagaraEffect;

    FTimerHandle TimerHandle_ResetCollision;

    // Declare these variables in the class header
private:
    FVector StartLocation;
    FVector TargetLocation;
    FVector InterpolatedEnd;
    float InterpolationTime;
    float ElapsedTime;
    UNiagaraComponent* NiagaraComponent;
    bool bIsInterpolating;

};
