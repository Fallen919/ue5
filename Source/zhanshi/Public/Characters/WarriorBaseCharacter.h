// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Engine/EngineTypes.h"
#include "WarriorBaseCharacter.generated.h"

class UWarriorAbilitySystemComponent;
class UWarriorAttributeSet;
class UDataAsset_StartUpDataBase;

UCLASS()
class ZHANSHI_API AWarriorBaseCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWarriorBaseCharacter();

protected:
	//~ Begin IAbilitySystemInterface Interface.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface Interface

	//~ Begin APawn Interface.
    virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UWarriorAbilitySystemComponent* WarriorAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UWarriorAttributeSet* WarriorAttributeSet;
	//����������
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData")
	TSoftObjectPtr< UDataAsset_StartUpDataBase>CharacterStartUpData;

public:
	// Called by attribute set when health reaches zero.
	void HandleDeath(const FGameplayEffectSpec& EffectSpec, float Damage);

	// Reset death-related state for respawn or re-possess.
	void ResetForRespawn();

	FORCEINLINE UWarriorAbilitySystemComponent* GetWarriorAbilitySystemComponent()const {
		return WarriorAbilitySystemComponent;
	}
	FORCEINLINE UWarriorAttributeSet* GetWarriorAttributeSet()const 
	{
		return WarriorAttributeSet;
	}

	FORCEINLINE bool IsDead() const { return bIsDead; }

protected:
	bool bIsDead = false;

	bool bCachedCollisionDefaults = false;
	ECollisionEnabled::Type DefaultCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	FCollisionResponseContainer DefaultCapsuleResponses;

};
