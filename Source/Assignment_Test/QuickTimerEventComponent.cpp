
#include "QuickTimerEventComponent.h"
#include "Components/ActorComponent.h"
#include "MyCharacter.h"

// Sets default values for this component's properties
UQuickTimerEventComponent::UQuickTimerEventComponent() :
	QTE_TargetCount(5),
	QTE_Count(0),
	bQTEActive(false),
	QuicktimerTimeLimit(3.f)
{
	PrimaryComponentTick.bCanEverTick = false;

}

// Called when the game starts
void UQuickTimerEventComponent::BeginPlay()
{
	Super::BeginPlay();

	MyCharacter = Cast<AMyCharacter>(GetOwner());
}

void UQuickTimerEventComponent::StartQTE()
{
	bQTEActive = true;
	GetWorld()->GetTimerManager().SetTimer(QTE_TimerHandle, this, &UQuickTimerEventComponent::EndQTE, QuicktimerTimeLimit, false);

	MyCharacter->QTEWidgetDisplayText = FString::Printf(TEXT("Start Pressing E five times within 3 sec"));
}

void UQuickTimerEventComponent::HandleQTEKeyPress()
{
	if (bQTEActive)
	{
		QTE_Count++;

		if (QTE_Count >= QTE_TargetCount)
		{
			EndQTE();
		}
	}
}

void UQuickTimerEventComponent::EndQTE()
{
	bQTEActive = false;

	// Trigger win or lose events based on bSuccess
	if (QTE_Count >= QTE_TargetCount)
	{

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("QTE Succeeded!"));
		MyCharacter->QTEWidgetDisplayText = FString::Printf(TEXT("You Won!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("QTE Failed!"));
		MyCharacter->QTEWidgetDisplayText = FString::Printf(TEXT("You Loose"));
	}

	ResetQTE();
}

void UQuickTimerEventComponent::ResetQTE()
{
	GetWorld()->GetTimerManager().ClearTimer(QTE_TimerHandle);
	QTE_Count = 0;
	bQTEActive = false;
}