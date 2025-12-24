// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OnlineGameMode.generated.h"

UCLASS(minimalapi)
class AOnlineGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOnlineGameMode();

	// 游戏数据
	UFUNCTION(BlueprintCallable)
	void AddTeamKill();

	UFUNCTION(BlueprintCallable)
	void AddEnemyKill();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// HUD
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameHUDClass;

	UPROPERTY()
	UUserWidget *GameHUDWidget;

	// 游戏数据
	int32 TeamKills;
	int32 EnemyKills;
	float MatchTimeRemaining; // 秒
	float MatchDuration;	  // 总时长（秒）

	void UpdateHUD();
};
