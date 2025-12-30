// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"

void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InInputTag)
{
    UE_LOG(LogTemp, Error, TEXT("[ASC] ActivatableAbilities Num = %d"), GetActivatableAbilities().Num());

    if (!InInputTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ASC] Invalid Input Tag"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ASC] Handle Input Tag = %s"), *InInputTag.ToString());

    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ASC] Check Ability = %s"), *AbilitySpec.Ability->GetName());

        if (!AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
        {
            continue;
        }

        TryActivateAbility(AbilitySpec.Handle);
    }
}


void UWarriorAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InInputTag)
{
}

void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(const TArray<FWarriorHeroAbilitySet>& IndefaultWeaponAbilities, int32 ApplyLevel, TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	if (IndefaultWeaponAbilities.IsEmpty())
	{
		return;
	}
	for (const FWarriorHeroAbilitySet& AbilitySet : IndefaultWeaponAbilities)
	{
		if (!AbilitySet.IsValid())continue;
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilitySet.InputTag);

	
		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}
}

void UWarriorAbilitySystemComponent::RemovedGrantedHeroWeaponAbilities(UPARAM(ref)TArray<FGameplayAbilitySpecHandle>& InSpecHandlesToRemove)
{
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
