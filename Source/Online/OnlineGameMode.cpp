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

	UE_LOG(LogTemp, Warning, TEXT("OnlineGameMode BeginPlay"));
}

void AOnlineGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		bool bIsLocal = NewPlayer->IsLocalPlayerController();
		APawn* ControlledPawn = NewPlayer->GetPawn();
		
		UE_LOG(LogTemp, Warning, TEXT("玩家加入: %s (本地=%d, Pawn=%s)"), 
			*NewPlayer->GetName(), bIsLocal, ControlledPawn ? TEXT("有") : TEXT("无"));

		// 强制设置为游戏输入模式
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		NewPlayer->SetInputMode(InputMode);
		NewPlayer->SetShowMouseCursor(false);
		NewPlayer->bShowMouseCursor = false;
		
		// 确保输入启用
		NewPlayer->SetIgnoreMoveInput(false);
		NewPlayer->SetIgnoreLookInput(false);

		if (GEngine)
		{
			FString StatusMsg = FString::Printf(TEXT("玩家加入: %s [本地:%s]"), 
				*NewPlayer->GetName(), bIsLocal ? TEXT("是") : TEXT("否"));
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, StatusMsg);
			
			if (ControlledPawn)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
					FString::Printf(TEXT("已占有角色: %s"), *ControlledPawn->GetName()));
			}
		}

		// 如果是本地玩家，创建HUD
		if (bIsLocal)
		{
			if (GameHUDClass && !GameHUDWidget)
			{
				UE_LOG(LogTemp, Warning, TEXT("为本地玩家创建HUD: %s"), *GameHUDClass->GetName());

				GameHUDWidget = CreateWidget<UUserWidget>(NewPlayer, GameHUDClass);
				if (GameHUDWidget)
				{
					GameHUDWidget->AddToViewport();
					UE_LOG(LogTemp, Warning, TEXT("游戏HUD创建成功"));
					UpdateHUD();
				}
			}
			else if (!GameHUDClass)
			{
				UE_LOG(LogTemp, Error, TEXT("GameHUDClass未设置！"));
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
