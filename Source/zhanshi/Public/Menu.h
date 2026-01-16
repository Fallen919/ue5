// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

// 前向声明
class UMultiplayerSessionsSubsystem;

/**
 * 游戏菜单UI - 创建游戏和加入游戏
 */
UCLASS()
class ZHANSHI_API UMenu : public UUserWidget
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
    UButton *Button_CreateGame;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_JoinGame;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_Back;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton *Button_SelectCharacter;

    // 测试用：直接连接按钮（可选）
    UPROPERTY(meta = (BindWidgetOptional))
    UButton *Button_DirectConnect;

    // 主菜单类（设置为WBP_MainMenu）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> MainMenuClass;

    // 角色选择UI类
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> CharacterSelectionClass;

    // 当前选中的角色索引
    UPROPERTY(BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
    int32 SelectedCharacterIndex = 0;

    UPROPERTY()
    UUserWidget *MainMenuWidget;

    UFUNCTION()
    void OnCreateGameClicked();

    UFUNCTION()
    void OnJoinGameClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnSelectCharacterClicked();

    UFUNCTION()
    void OnDirectConnectClicked();  // 直接连接功能

    // Subsystem回调
    UFUNCTION()
    void OnCreateSession(bool bWasSuccessful);
    void OnFindSessions(const TArray<FOnlineSessionSearchResult> &SessionResults, bool bWasSuccessful);
    void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
    // 多人游戏子系统
    UMultiplayerSessionsSubsystem *MultiplayerSessionsSubsystem;
};
