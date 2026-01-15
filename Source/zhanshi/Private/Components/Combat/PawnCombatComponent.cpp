// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "Net/UnrealNetwork.h"
#include"WarriorDebugHelper.h"

UPawnCombatComponent::UPawnCombatComponent()
{
    SetIsReplicatedByDefault(true);
}

void UPawnCombatComponent::RegisterSpawnWeapon(
    FGameplayTag InWeaponTagToRegister,  // 要注册的武器标签，用于唯一标识武器类型，如"Weapon.Axe"
    AWarriorWeaponBase* InWeaponToRegister, // 要注册的武器的指针
    bool bRegisterAsEquippedWeapon // 是否将此武器设置为当前装备的武器
)
{
    if (CharacterCarriedWeaponMap.Contains(InWeaponTagToRegister))
    {
        if (bRegisterAsEquippedWeapon)
        {
            CurrentEquipedWeaponTag = InWeaponTagToRegister;
        }
        UE_LOG(LogTemp, Warning, TEXT("[Weapon] Already registered: %s for %s"), 
            *InWeaponTagToRegister.ToString(), 
            *GetNameSafe(GetOwner()));
        return;
    }
    // 确保武器的指针有效
    check(InWeaponToRegister);

    UE_LOG(LogTemp, Warning, TEXT("[Weapon] Registering: %s for %s | Weapon=%s (Equipped=%d)"), 
        *InWeaponTagToRegister.ToString(), 
        *GetNameSafe(GetOwner()),
        *GetNameSafe(InWeaponToRegister),
        bRegisterAsEquippedWeapon);

    // 将武器添加到角色携带的武器字典中
    // - 键：InWeaponTagToRegister
    // - 值：InWeaponToRegister
    CharacterCarriedWeaponMap.Emplace(InWeaponTagToRegister, InWeaponToRegister);

    // 如果参数指定要装备此武器，则更新当前装备武器的标签
    if (bRegisterAsEquippedWeapon)
    {
        // 设置当前装备的武器标签
        CurrentEquipedWeaponTag = InWeaponTagToRegister;
    }

    // 通知客户端注册武器
    if (AActor* Owner = GetOwner())
    {
        if (Owner->HasAuthority())
        {
            ClientRegisterWeapon(InWeaponTagToRegister, InWeaponToRegister, bRegisterAsEquippedWeapon);
        }
    }
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagGet) const
{
    if (CharacterCarriedWeaponMap.Contains(InWeaponTagGet))
    {
        if (AWarriorWeaponBase* const* FoundWeapon = CharacterCarriedWeaponMap.Find(InWeaponTagGet))
        {
            AWarriorWeaponBase* Weapon = *FoundWeapon;
            
            // 检查武器是否有效
            if (!IsValid(Weapon))
            {
                UE_LOG(LogTemp, Error, TEXT("[Weapon] Found weapon in map but it's INVALID! Tag=%s Owner=%s"), 
                    *InWeaponTagGet.ToString(), 
                    *GetNameSafe(GetOwner()));
                return nullptr;
            }
            
            return Weapon;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("[Weapon] Weapon NOT FOUND in map! Tag=%s Owner=%s MapSize=%d"), 
        *InWeaponTagGet.ToString(), 
        *GetNameSafe(GetOwner()),
        CharacterCarriedWeaponMap.Num());
    
    return nullptr;
}

void UPawnCombatComponent::ClientRegisterWeapon_Implementation(FGameplayTag InWeaponTag, AWarriorWeaponBase* InWeapon, bool bAsEquipped)
{
    if (!InWeapon || !InWeaponTag.IsValid())
    {
        return;
    }

    // Only register on clients
    if (AActor* Owner = GetOwner())
    {
        if (Owner->HasAuthority())
        {
            return; // Server already registered in the main function
        }
    }

    if (!CharacterCarriedWeaponMap.Contains(InWeaponTag))
    {
        CharacterCarriedWeaponMap.Emplace(InWeaponTag, InWeapon);
        
        if (bAsEquipped)
        {
            CurrentEquipedWeaponTag = InWeaponTag;
        }

        UE_LOG(LogTemp, Warning, TEXT("[Weapon] CLIENT registered: %s for %s | Weapon=%s"), 
            *InWeaponTag.ToString(), 
            *GetNameSafe(GetOwner()),
            *GetNameSafe(InWeapon));
    }
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
    if (!CurrentEquipedWeaponTag.IsValid())
    {
        return nullptr;
    }
    return GetCharacterCarriedWeaponByTag(CurrentEquipedWeaponTag);
}

void UPawnCombatComponent::DestroyCarriedWeapons()
{
    if (AActor* Owner = GetOwner())
    {
        if (!Owner->HasAuthority())
        {
            return;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("[Weapon] ===== DESTROYING ALL WEAPONS ===== Owner=%s MapSize=%d"), 
        *GetNameSafe(GetOwner()),
        CharacterCarriedWeaponMap.Num());

    for (TPair<FGameplayTag, AWarriorWeaponBase*>& Pair : CharacterCarriedWeaponMap)
    {
        if (IsValid(Pair.Value))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Weapon] Destroying weapon: %s"), *GetNameSafe(Pair.Value));
            
            // Reset collision before destroying
            if (UBoxComponent* CollisionBox = Pair.Value->GetWeaponCollisionBox())
            {
                CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            
            Pair.Value->Destroy();
        }
    }

    CharacterCarriedWeaponMap.Empty();
    CurrentEquipedWeaponTag = FGameplayTag();
    
    UE_LOG(LogTemp, Error, TEXT("[Weapon] ===== ALL WEAPONS DESTROYED ===== Owner=%s"), *GetNameSafe(GetOwner()));
}


void UPawnCombatComponent::StartBlock(float PerfectBlockWindow)
{
    if (AActor* Owner = GetOwner())
    {
        if (!Owner->HasAuthority())
        {
            return;
        }
    }

    bIsBlocking = true;

    float Window = PerfectBlockWindow;
    if (Window < 0.f)
    {
        Window = DefaultPerfectBlockWindow;
    }

    if (Window > 0.f)
    {
        bPerfectBlockWindowActive = true;
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(PerfectBlockWindowTimer);
            World->GetTimerManager().SetTimer(PerfectBlockWindowTimer, this, &ThisClass::ConsumePerfectBlockWindow, Window, false);
        }
    }
    else
    {
        bPerfectBlockWindowActive = false;
    }
}

void UPawnCombatComponent::EndBlock()
{
    if (AActor* Owner = GetOwner())
    {
        if (!Owner->HasAuthority())
        {
            return;
        }
    }

    bIsBlocking = false;
    ConsumePerfectBlockWindow();
}

bool UPawnCombatComponent::IsBlocking() const
{
    return bIsBlocking;
}

bool UPawnCombatComponent::IsPerfectBlockWindowActive() const
{
    return bPerfectBlockWindowActive;
}

float UPawnCombatComponent::GetBlockDamageMultiplier() const
{
    return BlockDamageMultiplier;
}

void UPawnCombatComponent::ConsumePerfectBlockWindow()
{
    bPerfectBlockWindowActive = false;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerfectBlockWindowTimer);
    }
}

void UPawnCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPawnCombatComponent, bIsBlocking);
    DOREPLIFETIME(UPawnCombatComponent, bPerfectBlockWindowActive);
}


