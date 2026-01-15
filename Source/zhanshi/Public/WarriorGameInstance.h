// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WarriorGameInstance.generated.h"

/**
 * 游戏实例 - 保存跨关卡的数据
 */
UCLASS()
class ZHANSHI_API UWarriorGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 设置和获取选择的角色索引
	void SetSelectedCharacterIndex(int32 Index) { SelectedCharacterIndex = Index; }
	int32 GetSelectedCharacterIndex() const { return SelectedCharacterIndex; }

private:
	// 选择的角色索引
	UPROPERTY()
	int32 SelectedCharacterIndex = 0;
};
