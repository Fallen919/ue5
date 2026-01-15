// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorAbilitySystemComponent.generated.h"

/**
 * 自定义的Ability System Component
 * 扩展了GAS的功能，实现输入与技能的绑定，以及动态技能的授予和移除
 */
UCLASS()
class ZHANSHI_API UWarriorAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	// 处理输入按下事件，根据输入标签激活对应的技能
	void OnAbilityInputPressed(const FGameplayTag& InInputTag);
	
	// 处理输入释放事件，通知技能输入已释放
	void OnAbilityInputReleased(const FGameplayTag& InInputTag);

	// 动态授予武器技能列表，返回技能句柄数组用于后续移除
	UFUNCTION(BlueprintCallable, Category = "Warrior|Ability", meta = (ApplyLevel = "1"))
	void GrantHeroWeaponAbilities(const TArray<FWarriorHeroAbilitySet>& IndefaultWeaponAbilities,int32 ApplyLevel,TArray<FGameplayAbilitySpecHandle>&OutGrantedAbilitySpecHandles);

	// 移除之前授予的武器技能，清空技能句柄数组
	UFUNCTION(BlueprintCallable, Category = "Warrior|Ability")
	void RemovedGrantedHeroWeaponAbilities(UPARAM(ref) TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove);
};
