// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CreatLevelGameMode.generated.h"

/**
 * CreatLevel关卡的GameMode - 显示创造游戏/加入游戏菜单
 */
UCLASS()
class ONLINE_API ACreatLevelGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACreatLevelGameMode();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> GameMenuClass;

    UPROPERTY()
    UUserWidget *GameMenuWidget;
};
