// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include"GameplayTagContainer.h"
#include "WarriorStructTypes.generated.h"

class UWarriorHeroLinkedAnimlLayer;
class UWarriorHeroGameplayAbility;
class UInputMappingContext;
class UAnimInstance;
class USkeletalMesh;

/**
 * 技能配置结构：定义单个技能的输入标签和技能类
 * 用于配置武器的技能列表
 */
USTRUCT(BlueprintType)
struct FWarriorHeroAbilitySet
{
	GENERATED_BODY()

	// 输入标签：用于将输入事件绑定到技能（如InputTag_LightAttack_Axe）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	// 要授予的技能类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UWarriorHeroGameplayAbility>AbilityToGrant;

	bool IsValid()const;


};

/**
 * 武器数据结构：配置一把武器的所有内容
 * 包含动画层、技能列表、输入映射
 * 数据驱动设计：添加新武器只需配置数据，无需改代码
 */
USTRUCT(BlueprintType)
struct FWarriorHeroWeaponData
{
	GENERATED_BODY()

	// 武器动画层：链接到角色动画蓝图，提供武器特定的动画
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<UWarriorHeroLinkedAnimlLayer>WeaponAnimLayerToLink;

	// 武器技能列表：配置这把武器的所有技能（轻击、重击、格挡等）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,meta=(TitleProperty="InputTag"))
	TArray<FWarriorHeroAbilitySet> DefaultWeaponAbilities;

	// 武器输入映射：定义这把武器的输入按键配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputMappingContext* WeaponInputMapingContext;
};
USTRUCT(BlueprintType)
struct FWarriorCharacterAppearance
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMesh* SkeletalMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> AnimClass;

	bool IsValid() const
	{
		return SkeletalMesh != nullptr;
	}
};
