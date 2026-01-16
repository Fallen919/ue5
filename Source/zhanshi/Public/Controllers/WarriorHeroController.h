// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WarriorHeroController.generated.h"

class UGameHUD;
class UPauseMenu;

/**
 * 
 */
UCLASS()
class ZHANSHI_API AWarriorHeroController : public APlayerController
{
	GENERATED_BODY()

public:
	AWarriorHeroController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 创建并显示HUD
	void CreateGameHUD();

	// 显示暂停菜单
	void ShowPauseMenu();

	// 更新HUD显示
	UFUNCTION()
	void UpdateHUDKills(const FWarriorKillEvent& Event);

	// 更新血条显示
	void UpdateHUDHealth(float CurrentHealth, float MaxHealth);

	// 获取游戏HUD引用
	UGameHUD* GetGameHUD() const { return GameHUDWidget; }

protected:
	// HUD Widget类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UGameHUD> GameHUDClass;

	// 暂停菜单Widget类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPauseMenu> PauseMenuClass;

	// HUD实例
	UPROPERTY()
	UGameHUD* GameHUDWidget;

	// 暂停菜单实例
	UPROPERTY()
	UPauseMenu* PauseMenuWidget;

	// 游戏时间
	float GameTime = 0.0f;

	// 游戏总时长（秒）
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float TotalGameTime = 1200.0f; // 默认20分钟
};

