// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 * 主菜单关卡的GameMode
 */
UCLASS()
class ONLINE_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMainMenuGameMode();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuClass;

    UPROPERTY()
    UUserWidget *MainMenuWidget;
};
