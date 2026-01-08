// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayTagContainer.h"
#include "HeroCombatComponent.generated.h"

class AWarriorHeroWeapon;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class ZHANSHI_API UHeroCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTag)const;

	// 在客户端添加武器输入映射
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Warrior|Combat")
	void ClientAddInputMappingContext(UInputMappingContext* MappingContext, int32 Priority);

	// 在客户端移除武器输入映射
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Warrior|Combat")
	void ClientRemoveInputMappingContext(UInputMappingContext* MappingContext);

};
