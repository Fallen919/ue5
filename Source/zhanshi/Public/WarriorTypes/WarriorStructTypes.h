// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include"GameplayTagContainer.h"
#include "WarriorStructTypes.generated.h"

class UWarriorHeroLinkedAnimlLayer;
class UWarriorHeroGameplayAbility;
class UInputMappingContext;
class UAnimInstance;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct FWarriorHeroAbilitySet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag; //  ‰»Î±Í«©

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UWarriorHeroGameplayAbility>AbilityToGrant;

	bool IsValid()const;


};

USTRUCT(BlueprintType)
struct FWarriorHeroWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<UWarriorHeroLinkedAnimlLayer>WeaponAnimLayerToLink;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,meta=(TitleProperty="InputTag"))
	TArray<FWarriorHeroAbilitySet> DefaultWeaponAbilities;

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
