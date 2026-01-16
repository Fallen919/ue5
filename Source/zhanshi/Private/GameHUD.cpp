// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"

bool UGameHUD::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    // 初始化显示
    UpdateKills(0, 0);
    UpdateRemainingTime(5, 0); // 默认5分钟

    // 显示控制提示
    ShowControlsHint();

    return true;
}

void UGameHUD::UpdateKills(int32 TeamKills, int32 TeamDeaths)
{
    if (Text_TeamKills)
    {
        Text_TeamKills->SetText(FText::FromString(FString::Printf(TEXT("我方击杀: %d"), TeamKills)));
    }

    if (Text_TeamDeaths)
    {
        Text_TeamDeaths->SetText(FText::FromString(FString::Printf(TEXT("我方死亡: %d"), TeamDeaths)));
    }
}

void UGameHUD::UpdateRemainingTime(int32 Minutes, int32 Seconds)
{
    if (Text_RemainingTime)
    {
        Text_RemainingTime->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
    }
}

void UGameHUD::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
    if (ProgressBar_Health)
    {
        float HealthPercent = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
        ProgressBar_Health->SetPercent(HealthPercent);
    }
}

void UGameHUD::ShowControlsHint()
{
    if (Text_ControlsHint)
    {
        FString HintText = TEXT("WSAD移动\nShift格挡\n左键轻击\n右键重击\nQ取武器\nZ收回武器\n击杀20个胜利");
        Text_ControlsHint->SetText(FText::FromString(HintText));
        Text_ControlsHint->SetVisibility(ESlateVisibility::Visible);
    }
}

void UGameHUD::HideControlsHint()
{
    if (Text_ControlsHint)
    {
        Text_ControlsHint->SetVisibility(ESlateVisibility::Collapsed);
    }
}
