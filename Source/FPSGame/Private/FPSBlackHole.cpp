// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSBlackHole.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AFPSBlackHole::AFPSBlackHole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	InnerSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InnerComp"));
	InnerSphereComp->SetSphereRadius(100.0f);
	InnerSphereComp->SetupAttachment(MeshComp);

	// Bind to event
	InnerSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSBlackHole::OverlapInnerSphere);

	OuterSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("OuterComp"));
	OuterSphereComp->SetSphereRadius(3000.0f);
	OuterSphereComp->SetupAttachment(MeshComp);
}


void AFPSBlackHole::OverlapInnerSphere(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor)
	{
		OtherActor->Destroy();
	}
}

// Called every frame
void AFPSBlackHole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<UPrimitiveComponent*>  OverlappingComps; //�迭 ����
	OuterSphereComp->GetOverlappingComponents(OverlappingComps); //�ܺ� ���� ������ ������Ʈ ����Ʈ ����

	for (int32 i = 0; i < OverlappingComps.Num(); i++)
	{
		UPrimitiveComponent* PrimComp = OverlappingComps[i];
		if (PrimComp && PrimComp->IsSimulatingPhysics())
		{
			const float SphereRadius = OuterSphereComp->GetScaledSphereRadius();
			const float ForceStrenth = -2000; //�߾����� ���Ƶ��̱� ���� ������

			PrimComp->AddRadialForce(GetActorLocation(), SphereRadius, ForceStrenth, ERadialImpulseFalloff::RIF_Constant, true);
		}
	}
}

