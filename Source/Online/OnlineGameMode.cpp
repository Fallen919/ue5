// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineGameMode.h"
#include "OnlineCharacter.h"
#include "GameHUD.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

AOnlineGameMode::AOnlineGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// 启用Tick
	PrimaryActorTick.bCanEverTick = true;

	// 初始化游戏数据
	TeamKills = 0;
	EnemyKills = 0;
	MatchDuration = 300.0f; // 5分钟
	MatchTimeRemaining = MatchDuration;
}

void AOnlineGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("OnlineGameMode BeginPlay - 设置游戏输入模式"));

	// 确保所有玩家控制器都设置为游戏输入模式
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController *PlayerController = Iterator->Get();
		if (PlayerController)
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);

			UE_LOG(LogTemp, Warning, TEXT("玩家控制器设置为游戏模式"));

			// 创建HUD
			if (GameHUDClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("GameHUDClass 已设置: %s"), *GameHUDClass->GetName());

				if (!GameHUDWidget)
				{
					GameHUDWidget = CreateWidget<UUserWidget>(PlayerController, GameHUDClass);
					if (GameHUDWidget)
					{
						GameHUDWidget->AddToViewport();
						UE_LOG(LogTemp, Warning, TEXT("游戏HUD创建成功并添加到视口"));
						UpdateHUD();
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("游戏HUD创建失败！"));
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameHUDClass 未设置！请在GameMode蓝图中设置。当前GameMode: %s"), *GetClass()->GetName());
			}
		}
	}
}

void AOnlineGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新剩余时间
	if (MatchTimeRemaining > 0.0f)
	{
		MatchTimeRemaining -= DeltaTime;
		if (MatchTimeRemaining < 0.0f)
		{
			MatchTimeRemaining = 0.0f;
			UE_LOG(LogTemp, Warning, TEXT("比赛时间结束！"));
		}
		UpdateHUD();
	}
}

void AOnlineGameMode::AddTeamKill()
{
	TeamKills++;
	UE_LOG(LogTemp, Warning, TEXT("我方击杀 +1，当前: %d"), TeamKills);
	UpdateHUD();
}

void AOnlineGameMode::AddEnemyKill()
{
	EnemyKills++;
	UE_LOG(LogTemp, Warning, TEXT("敌方击杀 +1，当前: %d"), EnemyKills);
	UpdateHUD();
}

void AOnlineGameMode::UpdateHUD()
{
	if (GameHUDWidget)
	{
		if (UGameHUD *HUD = Cast<UGameHUD>(GameHUDWidget))
		{
			// 更新击杀数
			HUD->UpdateKills(TeamKills, EnemyKills);

			// 更新剩余时间
			int32 Minutes = FMath::FloorToInt(MatchTimeRemaining / 60.0f);
			int32 Seconds = FMath::FloorToInt(MatchTimeRemaining) % 60;
			HUD->UpdateRemainingTime(Minutes, Seconds);
		}
	}
}
