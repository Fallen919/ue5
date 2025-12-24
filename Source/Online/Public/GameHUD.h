// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameHUD.generated.h"

/**
 * 游戏内HUD - 显示击杀数和剩余时间
 */
UCLASS()
class ONLINE_API UGameHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    // 更新击杀数
    UFUNCTION(BlueprintCallable)
    void UpdateKills(int32 TeamKills, int32 EnemyKills);

    // 更新剩余时间
    UFUNCTION(BlueprintCallable)
    void UpdateRemainingTime(int32 Minutes, int32 Seconds);

protected:
    virtual bool Initialize() override;

private:
    // UI组件（从蓝图绑定）
    UPROPERTY(meta = (BindWidget))
    UTextBlock *Text_TeamKills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock *Text_EnemyKills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock *Text_RemainingTime;
};
