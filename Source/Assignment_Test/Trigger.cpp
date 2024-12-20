// Fill out your copyright notice in the Description page of Project Settings.


#include "Trigger.h"
#include "MyCharacter.h"
#include "QuickTimerEventComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ATrigger::ATrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(StaticMesh);
	SphereCollision->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ATrigger::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ATrigger::OnOverlap);
}

void ATrigger::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* Character = Cast<AMyCharacter>(OtherActor);
	if (Character && Character->GetQuickTimerEventComponent())
	{
		Character->GetQuickTimerEventComponent()->StartQTE();
		Character->CreateQTE_Widget();

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Start pressing E"));
	}
}

// Called every frame
void ATrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

