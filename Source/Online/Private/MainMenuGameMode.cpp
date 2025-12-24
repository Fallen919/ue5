// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuGameMode.h"
#include "MainMenu.h"
#include "Blueprint/UserWidget.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    // 不需要Pawn
    DefaultPawnClass = nullptr;
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (MainMenuClass)
    {
        APlayerController *PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            UMainMenu *MainMenu = CreateWidget<UMainMenu>(PlayerController, MainMenuClass);
            if (MainMenu)
            {
                MainMenuWidget = MainMenu;
                MainMenu->MenuSetup();
            }
        }
    }
}
