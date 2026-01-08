// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "WarriorGameplayTags.h"

// 输入按下时的处理函数：遍历所有可激活技能，找到匹配输入标签的技能并激活
void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InInputTag)
{
    if (!InInputTag.IsValid())
    {
        return;
    }

    // 遍历角色当前拥有的所有技能
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        // 检查技能的动态标签是否包含当前输入标签
        if (!AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
        {
            continue; // 不匹配则跳过
        }

        // 标记技能输入已按下
        AbilitySpecInputPressed(AbilitySpec);
        // 如果技能已激活，通知网络复制
        if (AbilitySpec.IsActive())
        {
            InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
        }

        // 尝试激活技能
        TryActivateAbility(AbilitySpec.Handle);
    }
}


void UWarriorAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InInputTag)
{
    if (!InInputTag.IsValid())
    {
        return;
    }

    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (!AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
        {
            continue;
        }

        AbilitySpecInputReleased(AbilitySpec);
        if (AbilitySpec.IsActive())
        {
            InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
            if (InInputTag == WarriorGameplayTags::InputTag_Block)
            {
                CancelAbilityHandle(AbilitySpec.Handle);
            }
        }
    }
}

// 动态授予武器技能：装备武器时调用，批量授予技能并保存句柄
void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(const TArray<FWarriorHeroAbilitySet>& IndefaultWeaponAbilities, int32 ApplyLevel, TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	// 只在服务器执行，确保权威性
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}
	if (IndefaultWeaponAbilities.IsEmpty())
	{
		return;
	}
	// 遍历武器的技能列表（如轻击、重击、格挡等）
	for (const FWarriorHeroAbilitySet& AbilitySet : IndefaultWeaponAbilities)
	{
		if (!AbilitySet.IsValid())continue;
		// 创建技能规格（Spec）
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		// 关键：添加输入标签到动态标签，用于输入匹配
		AbilitySpec.DynamicAbilityTags.AddTag(AbilitySet.InputTag);

		// 授予技能并保存句柄，卸下武器时用这个句柄移除技能
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}
}

void UWarriorAbilitySystemComponent::RemovedGrantedHeroWeaponAbilities(UPARAM(ref)TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}
    if (InSpecHandlesToRemove.IsEmpty())
    {
        return;
    }
    for (const FGameplayAbilitySpecHandle& SpecHandle : InSpecHandlesToRemove)
    {
        if (SpecHandle.IsValid())
        {
            ClearAbility(SpecHandle);//清除能力，使能力不再生效
        }
    }
    InSpecHandlesToRemove.Empty();//清除能力的记录数组，防止下次使用后导致错误
}
