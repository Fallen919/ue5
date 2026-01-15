// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "Menu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

bool UMainMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (Button_StartGame)
    {
        Button_StartGame->OnClicked.AddDynamic(this, &UMainMenu::OnStartGameClicked);
    }

    if (Button_QuitGame)
    {
        Button_QuitGame->OnClicked.AddDynamic(this, &UMainMenu::OnQuitGameClicked);
    }

    return true;
}

void UMainMenu::MenuSetup()
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
        }
    }
}

void UMainMenu::MenuTeardown()
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
        }
    }
}

void UMainMenu::OnStartGameClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("开始游戏按钮被点击！"));

    // 隐藏主菜单
    MenuTeardown();

    // 显示游戏菜单（创造游戏/加入游戏界面）
    if (GameMenuClass)
    {
        UWorld *World = GetWorld();
        if (World)
        {
            APlayerController *PlayerController = World->GetFirstPlayerController();
            if (PlayerController)
            {
                GameMenuWidget = CreateWidget<UUserWidget>(PlayerController, GameMenuClass);
                if (GameMenuWidget)
                {
                    UE_LOG(LogTemp, Warning, TEXT("创建游戏菜单Widget成功，类型: %s"), *GameMenuWidget->GetClass()->GetName());

                    // 尝试调用MenuSetup（如果是UMenu类型）
                    if (UMenu *Menu = Cast<UMenu>(GameMenuWidget))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Cast成功，调用Menu::MenuSetup"));
                        Menu->MenuSetup();
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Cast失败！WBP_Menu的父类不是Menu C++类！"));
                        UE_LOG(LogTemp, Warning, TEXT("使用手动设置..."));
                        // 手动设置
                        GameMenuWidget->AddToViewport();
                        GameMenuWidget->SetVisibility(ESlateVisibility::Visible);

                        FInputModeUIOnly InputMode;
                        InputMode.SetWidgetToFocus(GameMenuWidget->TakeWidget());
                        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                        PlayerController->SetInputMode(InputMode);
                        PlayerController->SetShowMouseCursor(true);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("创建游戏菜单Widget失败！"));
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMenuClass未设置！请在WBP_MainMenu蓝图中设置"));
    }
}

void UMainMenu::OnQuitGameClicked()
{
    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
        }
    }
}
