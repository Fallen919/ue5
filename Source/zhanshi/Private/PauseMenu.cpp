// Fill out your copyright notice in the Description page of Project Settings.

#include "PauseMenu.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

bool UPauseMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (Button_Resume)
    {
        Button_Resume->OnClicked.AddDynamic(this, &UPauseMenu::OnResumeClicked);
    }

    if (Button_Quit)
    {
        Button_Quit->OnClicked.AddDynamic(this, &UPauseMenu::OnQuitClicked);
    }

    return true;
}

void UPauseMenu::MenuSetup()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);

            // 不暂停游戏，让游戏继续运行
        }
    }
}

void UPauseMenu::MenuTeardown()
{
    RemoveFromParent();
    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);

            // 游戏本来就没暂停，无需恢复
        }
    }
}

void UPauseMenu::OnResumeClicked()
{
    MenuTeardown();
}

void UPauseMenu::OnQuitClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("暂停菜单：退出游戏，返回主菜单"));

    MenuTeardown();

    UWorld *World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController *PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // 检查是否在多人游戏中
    if (World->GetNetMode() != NM_Standalone)
    {
        // 多人游戏：先断开连接再返回主菜单
        UE_LOG(LogTemp, Warning, TEXT("多人游戏模式，断开连接并返回主菜单"));
        
        // 使用 ClientTravel 返回主菜单（会自动断开连接）
        PlayerController->ClientTravel(TEXT("/Game/Maps/MenuLevel"), ETravelType::TRAVEL_Absolute);
    }
    else
    {
        // 单机模式：直接切换关卡
        UGameplayStatics::OpenLevel(World, FName("MenuLevel"));
    }
}
