// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	GuardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();

	// 정찰이 켜져있을 경우 다음 정찰 포인트로 이동.
	if (bPatrol)
	{
		MoveToNextPatrolPoint();
	}
}

// 가드가 플레이어를 본 경우.
void AFPSAIGuard::OnPawnSeen(APawn * SeenPawn)
{
	if (SeenPawn == nullptr)
	{
		return;
	}
	// 플레이어 위치에 빨간 구체를 그린다.
	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{	
		// 미션 실패.
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);

	AController* Controller = GetController();
	if (Controller)
	{
		// 움직임 정지.
		Controller->StopMovement();
	}
}

// 소음을 들은 경우.
void AFPSAIGuard::OnNoiseHeard(APawn * NoiseInstigator, const FVector & Location, float Volume)
{
	if (GuardState == EAIState::Alerted)
	{
		return;
	}

	// 소음 위치에 초록 구체 표시.
	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;

	SetActorRotation(NewLookAt);
	SetGuardState(EAIState::Suspicious);

	// 3초 후 원래 상태로 복귀.
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);	

	AController* Controller = GetController();
	if (Controller)
	{
		// 움직임 정지.
		Controller->StopMovement();
	}
}

void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted)
	{
		return;
	}
	SetActorRotation(OriginalRotation);

	SetGuardState(EAIState::Idle);
	if (bPatrol)
	{
		MoveToNextPatrolPoint();
	}
}

void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState)
	{
		return;
	}

	GuardState = NewState;
	OnRep_GuardState();

	OnStateChanged(GuardState);
}

void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentPatrolPoint)
	{
		FVector Delta = GetActorLocation() - CurrentPatrolPoint->GetActorLocation();
		float DistanceToGoal = Delta.Size();

		// 다음 지점까지의 거리가 50 미만이면 다음 정찰 지점으로 이동.
		if (DistanceToGoal < 50)
		{
			MoveToNextPatrolPoint();
		}
	}
}

void AFPSAIGuard::MoveToNextPatrolPoint()
{
	if (CurrentPatrolPoint == nullptr || CurrentPatrolPoint == SecondPatrolPoint)
	{
		CurrentPatrolPoint = FirstPatrolPoint;
	}
	else
	{
		CurrentPatrolPoint = SecondPatrolPoint;
	}
	UNavigationSystem::SimpleMoveToActor(GetController(), CurrentPatrolPoint);
}

/*Replicated 속성을 사용하기 위해서는 GetLifetimeReplicatedProps() 함수에 
Replicated할 속성값을 등록해줘야만 정상적으로 빌드를 할 수 있다.*/
void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}

