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
 *
 */
UCLASS()
class ONLINE_API UMenu : public UUserWidget
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

	// 主菜单类（设置为WBP_MainMenu）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> MainMenuClass;

	UPROPERTY()
	UUserWidget *MainMenuWidget;

	UFUNCTION()
	void OnCreateGameClicked();

	UFUNCTION()
	void OnJoinGameClicked();

	UFUNCTION()
	void OnBackClicked();

	// Subsystem回调
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult> &SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
	// 多人游戏子系统
	UMultiplayerSessionsSubsystem *MultiplayerSessionsSubsystem;
};
