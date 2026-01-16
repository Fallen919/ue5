// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "GameHUD.generated.h"

/**
 * 游戏内HUD - 显示击杀数和剩余时间
 */
UCLASS()
class ZHANSHI_API UGameHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    // 更新击杀数和死亡数
    UFUNCTION(BlueprintCallable)
    void UpdateKills(int32 TeamKills, int32 TeamDeaths);

    // 更新剩余时间
    UFUNCTION(BlueprintCallable)
    void UpdateRemainingTime(int32 Minutes, int32 Seconds);

    // 更新血条
    UFUNCTION(BlueprintCallable)
    void UpdateHealthBar(float CurrentHealth, float MaxHealth);

    // 显示控制提示
    UFUNCTION(BlueprintCallable)
    void ShowControlsHint();

    // 隐藏控制提示
    UFUNCTION(BlueprintCallable)
    void HideControlsHint();

protected:
    virtual bool Initialize() override;

private:
    // UI组件（从蓝图绑定）
    UPROPERTY(meta = (BindWidget))
    UTextBlock *Text_TeamKills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock *Text_TeamDeaths;

    UPROPERTY(meta = (BindWidget))
    UTextBlock *Text_RemainingTime;

    UPROPERTY(meta = (BindWidget))
    UProgressBar *ProgressBar_Health;

    // 控制提示文本（可选，从蓝图绑定）
    UPROPERTY(meta = (BindWidget), meta = (BindWidgetOptional))
    UTextBlock *Text_ControlsHint;
};
