// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WarriorAttributeSet.generated.h"

// 宏定义：自动生成属性的访问器函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 角色属性集：定义角色的核心属性（生命值等）
 * PostGameplayEffectExecute是伤害系统的核心，处理格挡、死亡等逻辑
 */
UCLASS()
class ZHANSHI_API UWarriorAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWarriorAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 属性改变前的钳制处理（确保数值在合理范围）
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// GameplayEffect应用后的处理：伤害计算、格挡判定、死亡触发都在这里
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 当前生命值（网络复制）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UWarriorAttributeSet, Health);

	// 最大生命值（网络复制）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UWarriorAttributeSet, MaxHealth);

	// 进入伤害（临时属性，不复制，用于伤害计算）
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UWarriorAttributeSet, IncomingDamage);

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
};
