// Fill out your copyright notice in the Description page of Project Settings.

#include "CreatLevelGameMode.h"
#include "Menu.h"
#include "Blueprint/UserWidget.h"

ACreatLevelGameMode::ACreatLevelGameMode()
{
    // 不需要Pawn
    DefaultPawnClass = nullptr;
}

void ACreatLevelGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("CreatLevelGameMode BeginPlay called!"));

    if (GameMenuClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMenuClass is valid!"));
        APlayerController *PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerController found!"));
            GameMenuWidget = CreateWidget<UUserWidget>(PlayerController, GameMenuClass);
            if (GameMenuWidget)
            {
                UE_LOG(LogTemp, Warning, TEXT("GameMenu Widget created successfully!"));

                // 尝试调用 MenuSetup（如果是 UMenu 类型）
                UMenu *GameMenu = Cast<UMenu>(GameMenuWidget);
                if (GameMenu)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Calling MenuSetup..."));
                    GameMenu->MenuSetup();
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Widget is not UMenu, using manual setup..."));
                    GameMenuWidget->AddToViewport();
                    GameMenuWidget->SetVisibility(ESlateVisibility::Visible);

                    // 设置输入模式为UI
                    FInputModeUIOnly InputMode;
                    InputMode.SetWidgetToFocus(GameMenuWidget->TakeWidget());
                    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                    PlayerController->SetInputMode(InputMode);
                    PlayerController->SetShowMouseCursor(true);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create game menu widget!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMenuClass is NULL! Please set it in the blueprint."));
    }
}
