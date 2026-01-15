// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/WarriorAttributeSet.h"
#include "Characters/WarriorBaseCharacter.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGameplayTags.h"

UWarriorAttributeSet::UWarriorAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitIncomingDamage(0.f);
}

void UWarriorAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UWarriorAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarriorAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UWarriorAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

void UWarriorAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = GetIncomingDamage();
		if (Damage > 0.f)
		{
			// Ignore damage if character is already dead
			if (AWarriorBaseCharacter* OwnerCharacter = Cast<AWarriorBaseCharacter>(GetOwningActor()))
			{
				if (OwnerCharacter->IsDead())
				{
					SetIncomingDamage(0.f);
					return;
				}
			}

			float AdjustedDamage = Damage;
			bool bBlocked = false;

			if (AWarriorBaseCharacter* OwnerCharacter = Cast<AWarriorBaseCharacter>(GetOwningActor()))
			{
				if (UPawnCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UPawnCombatComponent>())
				{
					if (CombatComp->IsBlocking())
					{
						bBlocked = true;
						AdjustedDamage = Damage * CombatComp->GetBlockDamageMultiplier();
					}
				}
			}

			const FGameplayEffectContextHandle& ContextHandle = Data.EffectSpec.GetEffectContext();
			if (bBlocked)
			{
				FGameplayEventData BlockEvent;
				BlockEvent.EventTag = WarriorGameplayTags::Player_Event_Block;
				BlockEvent.Instigator = ContextHandle.GetOriginalInstigator();
				if (!BlockEvent.Instigator)
				{
					BlockEvent.Instigator = ContextHandle.GetInstigator();
				}
				BlockEvent.Target = GetOwningActor();
				BlockEvent.EventMagnitude = Damage;
				BlockEvent.OptionalObject = ContextHandle.GetSourceObject();
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningActor(), BlockEvent.EventTag, BlockEvent);
			}

			const float NewHealth = FMath::Clamp(GetHealth() - AdjustedDamage, 0.f, GetMaxHealth());
			SetHealth(NewHealth);
			SetIncomingDamage(0.f);
			UE_LOG(LogTemp, Warning, TEXT("[Damage] %s Health: %.1f / %.1f (Damage: %.1f)"),
				*GetOwningActor()->GetName(), NewHealth, GetMaxHealth(), AdjustedDamage);

			if (NewHealth <= 0.f)
			{
				FGameplayEventData EventData;
				EventData.EventTag = WarriorGameplayTags::Player_Event_Death;
				EventData.Instigator = ContextHandle.GetOriginalInstigator();
				if (!EventData.Instigator)
				{
					EventData.Instigator = ContextHandle.GetInstigator();
				}
				EventData.Target = GetOwningActor();
				EventData.EventMagnitude = AdjustedDamage;
				EventData.OptionalObject = ContextHandle.GetSourceObject();
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningActor(), WarriorGameplayTags::Player_Event_Death, EventData);

				if (AWarriorBaseCharacter* OwnerCharacter = Cast<AWarriorBaseCharacter>(GetOwningActor()))
				{
					OwnerCharacter->HandleDeath(Data.EffectSpec, AdjustedDamage);
				}
			}
			else if (!bBlocked && AdjustedDamage > 0.f)
			{
				FGameplayEventData EventData;
				EventData.EventTag = WarriorGameplayTags::Player_Event_HitReact;
				EventData.Instigator = ContextHandle.GetOriginalInstigator();
				if (!EventData.Instigator)
				{
					EventData.Instigator = ContextHandle.GetInstigator();
				}
				EventData.Target = GetOwningActor();
				EventData.EventMagnitude = AdjustedDamage;
				EventData.OptionalObject = ContextHandle.GetSourceObject();
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningActor(), WarriorGameplayTags::Player_Event_HitReact, EventData);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UWarriorAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarriorAttributeSet, Health, OldHealth);
}

void UWarriorAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarriorAttributeSet, MaxHealth, OldMaxHealth);
}


