// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorPlayerState.generated.h"

UCLASS()
class ZHANSHI_API AWarriorPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void SetCharacterAppearance(const FWarriorCharacterAppearance& InAppearance);
	const FWarriorCharacterAppearance& GetCharacterAppearance() const { return CharacterAppearance; }

	// 增加死亡次数
	void IncrementDeaths();
	
	// 获取死亡次数
	int32 GetDeaths() const { return Deaths; }

	// 设置和获取选择的角色索引
	void SetSelectedCharacterIndex(int32 Index);
	int32 GetSelectedCharacterIndex() const { return SelectedCharacterIndex; }

	// 客户端请求服务器设置角色选择
	UFUNCTION(Server, Reliable)
	void ServerSetSelectedCharacterIndex(int32 Index);

	// 击杀相关方法
	void MarkFirstKill();
	bool HasCompletedFirstKill() const { return bHasCompletedFirstKill; }

	// 死亡相关方法
	void MarkFirstDeath();
	bool HasCompletedFirstDeath() const { return bHasCompletedFirstDeath; }

	// 忽略统计（用于后加入的玩家/AI）
	void SetIgnoreStats(bool bIgnore) { bIgnoreStats = bIgnore; }
	bool ShouldIgnoreStats() const { return bIgnoreStats; }

protected:
	// 覆盖Score的复制通知，用于更新HUD
	virtual void OnRep_Score() override;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterAppearance)
	FWarriorCharacterAppearance CharacterAppearance;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths = 0;

	UPROPERTY(Replicated)
	int32 SelectedCharacterIndex = -1;

	// 是否已完成首次击杀（首次击杀不计分，第二次才计分）
	UPROPERTY(Replicated)
	bool bHasCompletedFirstKill = false;

	// 是否已完成首次死亡（首次死亡不计分，第二次才计分）
	UPROPERTY(Replicated)
	bool bHasCompletedFirstDeath = false;

	// 是否忽略统计（用于后加入的玩家/AI）
	UPROPERTY(Replicated)
	bool bIgnoreStats = false;

	UFUNCTION()
	void OnRep_CharacterAppearance();

	UFUNCTION()
	void OnRep_Deaths();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
