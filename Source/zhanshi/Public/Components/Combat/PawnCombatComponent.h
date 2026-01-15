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
	// bRegisterAsEquippedWeapon�ڵ���ʱΪtrue��hero/���Ϊfalse
	//���������ɺ������װ������

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
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat|Block")
	void StartBlock(float PerfectBlockWindow = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat|Block")
	void EndBlock();

	UFUNCTION(BlueprintPure, Category = "Warrior|Combat|Block")
	bool IsBlocking() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Combat|Block")
	bool IsPerfectBlockWindowActive() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Combat|Block")
	float GetBlockDamageMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat|Block")
	void ConsumePerfectBlockWindow();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	//ͨ�����TMap���ں�������������
	TMap<FGameplayTag, AWarriorWeaponBase*>CharacterCarriedWeaponMap;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true"))
	bool bIsBlocking = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true"))
	bool bPerfectBlockWindowActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float BlockDamageMultiplier = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warrior|Combat|Block", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DefaultPerfectBlockWindow = 0.2f;

	FTimerHandle PerfectBlockWindowTimer;
};






