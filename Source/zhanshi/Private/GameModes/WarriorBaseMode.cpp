// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/WarriorBaseMode.h"
#include "PlayerStates/WarriorPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "AbilitySystemComponent.h"

AWarriorBaseMode::AWarriorBaseMode()
{
	PlayerStateClass = AWarriorPlayerState::StaticClass();
}

void AWarriorBaseMode::HandleKillEvent(const FWarriorKillEvent& Event)
{
	OnKill.Broadcast(Event);
	OnDeath.Broadcast(Event);

	if (Event.KillerPS && Event.KillerPS != Event.VictimPS)
	{
		Event.KillerPS->SetScore(Event.KillerPS->GetScore() + KillScoreValue);
	}

	if (Event.VictimPS)
	{
		Event.VictimPS->SetScore(Event.VictimPS->GetScore() - DeathScorePenalty);

		AController* VictimController = Cast<AController>(Event.VictimPS->GetOwner());
		RequestRespawn(VictimController, RespawnDelay);
	}
}

void AWarriorBaseMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Respawn] Controller=%s Pawn=%s"),
		*GetNameSafe(NewPlayer),
		*GetNameSafe(NewPlayer->GetPawn()));
}


void AWarriorBaseMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	if (AvailableAppearances.IsEmpty())
	{
		return;
	}

	AWarriorPlayerState* WarriorPS = NewPlayer->GetPlayerState<AWarriorPlayerState>();
	if (!WarriorPS)
	{
		return;
	}

	int32 PlayerIndex = 0;
	if (GameState && GameState->PlayerArray.Num() > 0)
	{
		PlayerIndex = GameState->PlayerArray.Num() - 1;
	}

	const int32 AppearanceIndex = PlayerIndex % AvailableAppearances.Num();
	WarriorPS->SetCharacterAppearance(AvailableAppearances[AppearanceIndex]);
}

UClass* AWarriorBaseMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (AvailablePawnClasses.IsEmpty())
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	int32 PlayerIndex = 0;
	if (InController)
	{
		if (APlayerState* PS = InController->PlayerState)
		{
			PlayerIndex = PS->GetPlayerId();
		}
	}

	const int32 PawnIndex = FMath::Abs(PlayerIndex) % AvailablePawnClasses.Num();
	if (AvailablePawnClasses.IsValidIndex(PawnIndex) && AvailablePawnClasses[PawnIndex])
	{
		return AvailablePawnClasses[PawnIndex];
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AWarriorBaseMode::RequestRespawn(AController* Controller, float DelaySeconds)
{
	if (!Controller)
	{
		return;
	}

	TWeakObjectPtr<AController> WeakController = Controller;
	TWeakObjectPtr<APawn> WeakPawn = Controller->GetPawn();

	auto RespawnBody = [this, WeakController, WeakPawn]()
	{
		if (WeakPawn.IsValid())
		{
			if (UPawnCombatComponent* CombatComp = WeakPawn->FindComponentByClass<UPawnCombatComponent>())
			{
				CombatComp->DestroyCarriedWeapons();
			}
			
			// Clear AbilitySystemComponent before destroying pawn
			if (UAbilitySystemComponent* ASC = WeakPawn->FindComponentByClass<UAbilitySystemComponent>())
			{
				ASC->ClearAllAbilities();
				ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer());
			}
			
			WeakPawn->DetachFromControllerPendingDestroy();
			WeakPawn->Destroy();
		}

		if (WeakController.IsValid())
		{
			RestartPlayer(WeakController.Get());
		}
	};

	if (DelaySeconds <= 0.0f)
	{
		RespawnBody();
		return;
	}

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindLambda(RespawnBody);

	FTimerHandle RespawnHandle;
	GetWorldTimerManager().SetTimer(RespawnHandle, RespawnDelegate, DelaySeconds, false);
}
