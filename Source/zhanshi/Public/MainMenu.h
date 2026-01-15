// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenu.generated.h"

/**
 * 主菜单UI - 开始游戏和退出游戏
 */
UCLASS()
class ZHANSHI_API UMainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void MenuSetup();

    UFUNCTION(BlueprintCallable)
    void MenuTeardown();

protected:
    virtual bool Initialize() override;

private:
    UPROPERTY(meta = (BindWidget))
    UButton *Button_StartGame;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_QuitGame;

    // 游戏菜单类（设置为WBP_Menu）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> GameMenuClass;

    UPROPERTY()
    UUserWidget *GameMenuWidget;

    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnQuitGameClicked();
};
