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

    return true;
}

void UGameHUD::UpdateKills(int32 TeamKills, int32 EnemyKills)
{
    if (Text_TeamKills)
    {
        Text_TeamKills->SetText(FText::FromString(FString::Printf(TEXT("我方击杀: %d"), TeamKills)));
    }

    if (Text_EnemyKills)
    {
        Text_EnemyKills->SetText(FText::FromString(FString::Printf(TEXT("敌方击杀: %d"), EnemyKills)));
    }
}

void UGameHUD::UpdateRemainingTime(int32 Minutes, int32 Seconds)
{
    if (Text_RemainingTime)
    {
        Text_RemainingTime->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
    }
}
