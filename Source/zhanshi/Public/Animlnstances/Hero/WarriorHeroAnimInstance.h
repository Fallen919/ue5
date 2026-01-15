// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animlnstances/WarriorCharactersAnimInstance.h"
#include "WarriorHeroAnimInstance.generated.h"

class AWarriorHeroCharacter;

/**
 * 战士英雄角色的动画实例
 */
UCLASS()
class ZHANSHI_API UWarriorHeroAnimInstance : public UWarriorCharactersAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	// 每帧更新动画用的数据
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 拥有本 AnimInstance 的英雄角色
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Refrences")
	AWarriorHeroCharacter* OwningHeroCharacter;

	// 是否该进入“放松/休闲”待机状态
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomtionData")
	bool bShouldEnterRelaxState;

	// Idle 多久后进入放松状态（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomtionData")
	float EnterRelaxStateThreshold = 5.f;

	// 当前 Idle 已持续时间（秒）
	float IdleElpasedTime;
};
