// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "PauseMenu.generated.h"

/**
 * 暂停菜单UI - 继续游戏和退出游戏
 */
UCLASS()
class ONLINE_API UPauseMenu : public UUserWidget
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
    UButton *Button_Resume;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_Quit;

    UFUNCTION()
    void OnResumeClicked();

    UFUNCTION()
    void OnQuitClicked();
};
