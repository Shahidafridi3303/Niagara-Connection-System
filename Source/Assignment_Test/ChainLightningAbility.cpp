#include "ChainLightningAbility.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "Components/StaticMeshComponent.h"

AChainLightningAbility::AChainLightningAbility()
{
    PrimaryActorTick.bCanEverTick = true;
    bHasPerformedSphereTrace = false;
    bIsProcessing = false;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    RootComponent = StaticMeshComponent;
}

void AChainLightningAbility::BeginPlay()
{
    Super::BeginPlay();

    Tags.Add(FName("ChainLightningTarget"));
}

void AChainLightningAbility::PerformSphereTrace()
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

                for (auto& Result : OverlapResults)
                {
                    AActor* OverlappedActor = Result.GetActor();
                    if (OverlappedActor && OverlappedActor != this && !OverlappedActor->IsPendingKill())
                    {
                        // Check if the actor has the "ChainLightningTarget" tag
                        if (OverlappedActor->ActorHasTag(FName("ChainLightningTarget")))
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
            StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable collision to avoid further traces
        }, 1.0f, false); // Delay for 1 second before performing the sphere trace
}

void AChainLightningAbility::HandleOverlappingActors(const TArray<AActor*>& ValidTargets)
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

void AChainLightningAbility::Tick(float DeltaTime)
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

void AChainLightningAbility::PerformNiagaraEffect(AActor* TargetActor)
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

    // If the target is another ChainLightningAbility actor, call PerformSphereTrace on that actor
    if (TargetActor && TargetActor->IsA(AChainLightningAbility::StaticClass()))
    {
        AChainLightningAbility* ChainLightningActor = Cast<AChainLightningAbility>(TargetActor);
        if (ChainLightningActor && !ChainLightningActor->bHasPerformedSphereTrace)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("Triggering next chain lightning"));
            ChainLightningActor->PerformSphereTrace(); // Trigger next trace
        }
    }

    bIsProcessing = false; // Reset processing flag after completing the chain lightning
}


