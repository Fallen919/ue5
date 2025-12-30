// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include"WarriorDebugHelper.h"

void UPawnCombatComponent::RegisterSpawnWeapon(
    FGameplayTag InWeaponTagToRegister,  // 要注册的武器标签，用于唯一标识武器类型（如"Weapon.Sword"）
    AWarriorWeaponBase* InWeaponToRegister, // 要注册的武器对象指针
    bool bRegisterAsEquippedWeapon // 是否将此武器设置为当前装备的武器
)
{
    // 确保当前没有同标签的武器已被注册
    // 使用checkf在开发时提供详细错误信息，防止重复添加武器
    checkf(!CharacterCarriedWeaponMap.Contains(InWeaponTagToRegister),
        TEXT("名为 %s 的武器已经作为携带武器添加过了"), *InWeaponTagToRegister.ToString());
    // 确保传入的武器指针有效
    check(InWeaponToRegister);

    // 将武器添加到角色的武器字典中
    // 使用Emplace高效地直接在字典内存中构造键值对：
    // - 键：InWeaponTagToRegister
    // - 值：InWeaponToRegister
    CharacterCarriedWeaponMap.Emplace(InWeaponTagToRegister, InWeaponToRegister);

    // 如果参数指定要装备此武器，则更新当前装备武器的标签
    if (bRegisterAsEquippedWeapon)
    {
        // 设置当前装备武器标签
        CurrentEquipedWeaponTag = InWeaponTagToRegister;
    }
   
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagGet) const
{
    if (CharacterCarriedWeaponMap.Contains(InWeaponTagGet))
    {
        if (AWarriorWeaponBase* const* FoundWeapon = CharacterCarriedWeaponMap.Find(InWeaponTagGet))
        {
            return *FoundWeapon;
        }
    }
    return nullptr;
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
    if (!CurrentEquipedWeaponTag.IsValid())
    {
        return nullptr;
    }
    return GetCharacterCarriedWeaponByTag(CurrentEquipedWeaponTag);
}
