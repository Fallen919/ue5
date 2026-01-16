// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "UObject/ScriptDelegates.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorBaseMode.generated.h"

class APlayerState;
class AWarriorPlayerState;
class APawn;

USTRUCT(BlueprintType)
struct FWarriorKillEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	APlayerState* KillerPS = nullptr;

	UPROPERTY(BlueprintReadOnly)
	APlayerState* VictimPS = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DamageTag;

	UPROPERTY(BlueprintReadOnly)
	bool bFriendlyFire = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWarriorKillSignature, const FWarriorKillEvent&, Event);

/**
 * 
 */
UCLASS()
class ZHANSHI_API AWarriorBaseMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWarriorBaseMode();

	UPROPERTY(BlueprintAssignable, Category = "Warrior|Match")
	FWarriorKillSignature OnKill;

	UPROPERTY(BlueprintAssignable, Category = "Warrior|Match")
	FWarriorKillSignature OnDeath;

	void HandleKillEvent(const FWarriorKillEvent& Event);

	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Character")
	TArray<FWarriorCharacterAppearance> AvailableAppearances;

	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Character")
	TArray<TSubclassOf<APawn>> AvailablePawnClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Respawn")
	float RespawnDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Score")
	float KillScoreValue = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Score")
	float DeathScorePenalty = 0.0f;

	// 击杀数达标获胜
	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Win")
	int32 KillWinThreshold = 20;

	// 最大参与统计的玩家数量（超过此数量加入的玩家不计入击杀和死亡统计）
	// 设为0表示所有玩家都参与统计
	UPROPERTY(EditDefaultsOnly, Category = "Warrior|Stats")
	int32 MaxStatsPlayers = 0;

protected:
	// 是否已经结束比赛
	bool bMatchEnded = false;

	void EndMatchAndQuit(APlayerState* WinnerPS);

	void RequestRespawn(AController* Controller, float DelaySeconds);
};
