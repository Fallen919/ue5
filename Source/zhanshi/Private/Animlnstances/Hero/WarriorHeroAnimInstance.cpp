// Fill out your copyright notice in the Description page of Project Settings.

#include "Animlnstances/Hero/WarriorHeroAnimInstance.h"
#include "Characters/WarriorHeroCharacter.h"

void UWarriorHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 从基类里的 OwningCharacter 转成具体的英雄角色类型
	if (OwningCharacter)
	{
		OwningHeroCharacter = Cast<AWarriorHeroCharacter>(OwningCharacter);
	}
}

void UWarriorHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// 有加速度：说明在移动或起步，算“不在发呆”
	if (bHasAcceleration)
	{
		IdleElpasedTime = 0.f;
		bShouldEnterRelaxState = false;
	}
	else
	{
		// 没有加速度：认为在 Idle，累加 Idle 时间
		IdleElpasedTime += DeltaSeconds;

		// Idle 时间超过阈值，则标记可以进入放松状态
		bShouldEnterRelaxState = (IdleElpasedTime >= EnterRelaxStateThreshold);
	}
}
