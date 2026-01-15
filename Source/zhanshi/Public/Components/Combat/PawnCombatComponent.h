// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "PawnCombatComponent.generated.h"

class AWarriorWeaponBase;
class FLifetimeProperty;

/**
 * 
 */
UCLASS()
class ZHANSHI_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()
public:
	UPawnCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void RegisterSpawnWeapon(FGameplayTag InWeaponTagToRegister, AWarriorWeaponBase* InWeaponToRegister, bool bRegisterAsEquippedWeapon = false);
	// bRegisterAsEquippedWeapon在敌人时为true，hero/玩家为false
	// 完成武器生成后调用此函数注册

	// Client RPC to replicate weapon registration  
	UFUNCTION(Client, Reliable)
	void ClientRegisterWeapon(FGameplayTag InWeaponTag, AWarriorWeaponBase* InWeapon, bool bAsEquipped);

	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	AWarriorWeaponBase* GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagGet)const;

	UPROPERTY(BlueprintReadWrite, Category = "Warrior|Combat")
	FGameplayTag CurrentEquipedWeaponTag;

	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	AWarriorWeaponBase* GetCharacterCurrentEquippedWeapon()const;


	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void DestroyCarriedWeapons();
	
	// === 格挡系统 ===
	// 开始格挡，可选指定完美格挡窗口时长（秒）
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat|Block")
	void StartBlock(float PerfectBlockWindow = -1.f);

	// 结束格挡
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat|Block")
	void EndBlock();

	// 查询是否正在格挡
	UFUNCTION(BlueprintPure, Category = "Warrior|Combat|Block")
	bool IsBlocking() const;

	// 查询完美格挡窗口是否激活
	UFUNCTION(BlueprintPure, Category = "Warrior|Combat|Block")
	bool IsPerfectBlockWindowActive() const;

	// 获取格挡减伤倍率（0.3 = 减伤70%）
	UFUNCTION(BlueprintPure, Category = "Warrior|Combat|Block")
	float GetBlockDamageMultiplier() const;

	// 消耗完美格挡窗口（完美格挡成功后调用）
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat|Block")
	void ConsumePerfectBlockWindow();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 武器存储Map：用Gameplay Tag作为键，快速查找角色携带的武器
	TMap<FGameplayTag, AWarriorWeaponBase*>CharacterCarriedWeaponMap;

	// 格挡状态标记（网络复制）
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true"))
	bool bIsBlocking = false;

	// 完美格挡窗口激活标记（网络复制）
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true"))
	bool bPerfectBlockWindowActive = false;

	// 格挡减伤倍率：0.3表示只受到30%伤害（减伤70%）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float BlockDamageMultiplier = 0.3f;

	// 默认完美格挡窗口时长（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DefaultPerfectBlockWindow = 0.2f;

	// 完美格挡窗口计时器句柄
	FTimerHandle PerfectBlockWindowTimer;
};






