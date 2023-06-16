// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "FPSGameState.h"
#include "FPSPlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet//GameplayStatics.h"

AFPSGameMode::AFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHUD::StaticClass();

	GameStateClass = AFPSGameState::StaticClass();

	PlayerControllerClass = AFPSPlayerController::StaticClass();
}

// 미션 성공 판별 함수.
void AFPSGameMode::CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess)
{
	if (InstigatorPawn)
	{
		if (SpectatingViewpointClass)
		{
			// SpectatingViewpointClass를 찾아 ReturnedActors에 저장한다.
			TArray<AActor*> ReturnedActors;
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, ReturnedActors);
			
			if (ReturnedActors.Num() > 0)
			{
				AActor* NewViewTarget = ReturnedActors[0];

				// 모든 플레이어의 시점을 SpectatingViewpointClass로 변경한다.
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
				{
					APlayerController* PC = It->Get();
					if (PC) 
					{
						PC->SetViewTargetWithBlend(NewViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
					}
				}			
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatingViewpointClass is nullptr."))
		}
	}
	AFPSGameState* GS = GetGameState<AFPSGameState>();
	if (GS)
	{
		GS->MulticastOnMissionComplete(InstigatorPawn, bMissionSuccess);
	}

	OnMissionCompleted(InstigatorPawn, bMissionSuccess);
}