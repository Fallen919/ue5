// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/WarriorBaseMode.h"
#include "PlayerStates/WarriorPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "WarriorGameInstance.h"

AWarriorBaseMode::AWarriorBaseMode()
{
	PlayerStateClass = AWarriorPlayerState::StaticClass();
}

void AWarriorBaseMode::HandleKillEvent(const FWarriorKillEvent& Event)
{
	// 先处理击杀者的分数和击杀统计
	if (Event.KillerPS && Event.KillerPS != Event.VictimPS)
	{
		AWarriorPlayerState* KillerWarriorPS = Cast<AWarriorPlayerState>(Event.KillerPS);
		
		// 检查是否忽略统计（只有显式设置了MaxStatsPlayers且超出范围才忽略）
		bool bShouldIgnore = KillerWarriorPS && KillerWarriorPS->ShouldIgnoreStats();
		
		UE_LOG(LogTemp, Warning, TEXT("[Kill Debug] Killer=%s, KillerWarriorPS=%s, bShouldIgnore=%d, OldScore=%.0f"), 
			*Event.KillerPS->GetPlayerName(),
			KillerWarriorPS ? TEXT("Valid") : TEXT("NULL"),
			bShouldIgnore,
			Event.KillerPS->GetScore());
		
		if (!bShouldIgnore)
		{
			// 所有击杀都计分（包括首次）
			Event.KillerPS->SetScore(Event.KillerPS->GetScore() + KillScoreValue);
			UE_LOG(LogTemp, Warning, TEXT("[Kill] %s 击杀计分 +%.0f，当前分数: %.0f"), 
				*Event.KillerPS->GetPlayerName(), KillScoreValue, Event.KillerPS->GetScore());

			if (!bMatchEnded && Event.KillerPS->GetScore() >= KillWinThreshold)
			{
				EndMatchAndQuit(Event.KillerPS);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Kill] %s 被设置为忽略统计，不计分"), *Event.KillerPS->GetPlayerName());
		}
	}

	// 先处理受害者的死亡统计
	if (Event.VictimPS)
	{
		AWarriorPlayerState* VictimWarriorPS = Cast<AWarriorPlayerState>(Event.VictimPS);
		
		// 检查是否忽略统计（只有显式设置了MaxStatsPlayers且超出范围才忽略）
		bool bShouldIgnore = VictimWarriorPS && VictimWarriorPS->ShouldIgnoreStats();
		
		UE_LOG(LogTemp, Warning, TEXT("[Death Debug] Victim=%s, VictimWarriorPS=%s, bShouldIgnore=%d, OldDeaths=%d"), 
			*Event.VictimPS->GetPlayerName(),
			VictimWarriorPS ? TEXT("Valid") : TEXT("NULL"),
			bShouldIgnore,
			VictimWarriorPS ? VictimWarriorPS->GetDeaths() : 0);
		
		if (!bShouldIgnore)
		{
			// 所有死亡都计数（包括首次）
			Event.VictimPS->SetScore(Event.VictimPS->GetScore() - DeathScorePenalty);
			if (VictimWarriorPS)
			{
				VictimWarriorPS->IncrementDeaths();
			}
			UE_LOG(LogTemp, Warning, TEXT("[Death] %s 死亡计数，当前死亡数: %d"), 
				*Event.VictimPS->GetPlayerName(), 
				VictimWarriorPS ? VictimWarriorPS->GetDeaths() : 0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Death] %s 被设置为忽略统计，不计数"), *Event.VictimPS->GetPlayerName());
		}

		AController* VictimController = Cast<AController>(Event.VictimPS->GetOwner());
		RequestRespawn(VictimController, RespawnDelay);
	}

	// 在更新完分数和死亡数后再广播事件，这样HUD能获取到最新数据
	OnKill.Broadcast(Event);
	OnDeath.Broadcast(Event);
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

	// 检查是否为后加入的玩家（不参与统计）
	// 只有在明确设置MaxStatsPlayers > 0时才启用玩家限制
	if (MaxStatsPlayers > 0 && PlayerIndex >= MaxStatsPlayers)
	{
		WarriorPS->SetIgnoreStats(true);
		UE_LOG(LogTemp, Error, TEXT("[PostLogin] %s 为后加入玩家（索引%d >= 最大统计玩家数%d），不参与击杀和死亡统计"), 
			*NewPlayer->GetPlayerState<APlayerState>()->GetPlayerName(), PlayerIndex, MaxStatsPlayers);
	}
	else
	{
		// 明确设置为false，确保参与统计
		WarriorPS->SetIgnoreStats(false);
		UE_LOG(LogTemp, Warning, TEXT("[PostLogin] %s 参与击杀和死亡统计（索引%d, MaxStatsPlayers=%d, bIgnoreStats=%d）"), 
			*NewPlayer->GetPlayerState<APlayerState>()->GetPlayerName(), PlayerIndex, MaxStatsPlayers, WarriorPS->ShouldIgnoreStats());
	}

	// 提示操作与胜利条件
	const FString ShortHint = TEXT("WSAD移动，Shift格挡，左键轻击，右键重击");
	const FString WinHint = FString::Printf(TEXT("击杀%d人获胜，获胜后返回主菜单"), KillWinThreshold);
	NewPlayer->ClientMessage(ShortHint);
	NewPlayer->ClientMessage(WinHint);

	// 右上角提示 20 秒
	if (GEngine)
	{
		const FString Hint = FString::Printf(TEXT("WSAD移动 | Shift格挡 | 左键轻击 | 右键重击 | 击杀%d人获胜后返回主菜单"), KillWinThreshold);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Hint);
		GEngine->AddOnScreenDebugMessage(
			/*Key*/ 12345,
			/*TimeToDisplay*/ 20.f,
			/*Color*/ FColor::Cyan,
			Hint,
			/*bNewerOnTop*/ true);
	}
}

UClass* AWarriorBaseMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (AvailablePawnClasses.IsEmpty())
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	int32 PawnIndex = 0;
	bool bFoundSelection = false;
	
	// 优先从 PlayerState 获取角色选择（每个玩家有自己的选择）
	if (InController)
	{
		if (AWarriorPlayerState* WarriorPS = InController->GetPlayerState<AWarriorPlayerState>())
		{
			const int32 PSIndex = WarriorPS->GetSelectedCharacterIndex();
			if (PSIndex >= 0)
			{
				PawnIndex = PSIndex;
				bFoundSelection = true;
				UE_LOG(LogTemp, Warning, TEXT("[GetDefaultPawn] Controller=%s, PlayerName=%s, Using character from PlayerState: %d"), 
					*GetNameSafe(InController), *WarriorPS->GetPlayerName(), PawnIndex);
			}
		}
	}
	
	// 如果 PlayerState 没有选择，再尝试从 GameInstance 获取（只适用于本地主机）
	if (!bFoundSelection)
	{
		if (UWarriorGameInstance* GameInst = Cast<UWarriorGameInstance>(GetGameInstance()))
		{
			PawnIndex = GameInst->GetSelectedCharacterIndex();
			UE_LOG(LogTemp, Warning, TEXT("[GetDefaultPawn] Using character from GameInstance: %d"), PawnIndex);
		}
		// 如果都没有，使用PlayerID
		else if (InController && InController->PlayerState)
		{
			PawnIndex = InController->PlayerState->GetPlayerId();
			UE_LOG(LogTemp, Warning, TEXT("[GetDefaultPawn] Using PlayerID as character index: %d"), PawnIndex);
		}
	}

	const int32 FinalIndex = FMath::Abs(PawnIndex) % AvailablePawnClasses.Num();
	if (AvailablePawnClasses.IsValidIndex(FinalIndex) && AvailablePawnClasses[FinalIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[GetDefaultPawn] Spawning character at index: %d (Class: %s)"), FinalIndex, *AvailablePawnClasses[FinalIndex]->GetName());
		return AvailablePawnClasses[FinalIndex];
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AWarriorBaseMode::EndMatchAndQuit(APlayerState* WinnerPS)
{
	bMatchEnded = true;

	const FString WinnerName = WinnerPS ? WinnerPS->GetPlayerName() : TEXT("未知玩家");
	const FString Msg = FString::Printf(TEXT("%s 达到 %d 击杀，比赛结束，返回主菜单..."), *WinnerName, KillWinThreshold);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->ClientMessage(Msg);
		}
	}

	// 服务器切到主菜单关卡，客户端自动跟随
	const FString MenuMap = TEXT("/Game/Maps/MenuLevel");
	GetWorld()->ServerTravel(MenuMap + TEXT("?listen"));
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
