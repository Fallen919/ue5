// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/WarriorHeroController.h"
#include "GameHUD.h"
#include "PauseMenu.h"
#include "GameModes/WarriorBaseMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "PlayerStates/WarriorPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "AbilitySystemComponent.h"

AWarriorHeroController::AWarriorHeroController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWarriorHeroController::BeginPlay()
{
	Super::BeginPlay();

	// 延迟创建HUD，确保PlayerState已经初始化
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AWarriorHeroController::CreateGameHUD, 0.5f, false);

	// 设置定时器定期更新HUD（用于客户端，因为客户端无法访问GameMode事件）
	FTimerHandle HUDUpdateTimerHandle;
	GetWorldTimerManager().SetTimer(HUDUpdateTimerHandle, [this]()
	{
		if (GameHUDWidget && PlayerState)
		{
			FWarriorKillEvent DummyEvent;
			UpdateHUDKills(DummyEvent);
		}
	}, 0.2f, true); // 每0.2秒更新一次HUD
}

void AWarriorHeroController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新游戏时间
	if (GameHUDWidget)
	{
		GameTime += DeltaTime;
		float RemainingTime = FMath::Max(0.0f, TotalGameTime - GameTime);
		int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
		int32 Seconds = FMath::FloorToInt(FMath::Fmod(RemainingTime, 60.0f));
		GameHUDWidget->UpdateRemainingTime(Minutes, Seconds);

		// 更新血条
		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn)
		{
			if (UAbilitySystemComponent* ASC = ControlledPawn->FindComponentByClass<UAbilitySystemComponent>())
			{
				if (const UAttributeSet* AttributeSet = ASC->GetAttributeSet(UWarriorAttributeSet::StaticClass()))
				{
					const UWarriorAttributeSet* WarriorAttr = Cast<UWarriorAttributeSet>(AttributeSet);
					if (WarriorAttr)
					{
						float CurrentHealth = WarriorAttr->GetHealth();
						float MaxHealth = WarriorAttr->GetMaxHealth();
						UpdateHUDHealth(CurrentHealth, MaxHealth);
					}
				}
			}
		}
	}
}

void AWarriorHeroController::CreateGameHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!GameHUDClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameHUDClass is not set in WarriorHeroController!"));
		return;
	}

	if (!GameHUDWidget)
	{
		GameHUDWidget = CreateWidget<UGameHUD>(this, GameHUDClass);
		if (GameHUDWidget)
		{
			GameHUDWidget->AddToViewport();
			
			// 绑定击杀事件（只在服务器端有GameMode）
			if (AWarriorBaseMode* GameMode = GetWorld()->GetAuthGameMode<AWarriorBaseMode>())
			{
				GameMode->OnKill.AddDynamic(this, &AWarriorHeroController::UpdateHUDKills);
				GameMode->OnDeath.AddDynamic(this, &AWarriorHeroController::UpdateHUDKills);
				UE_LOG(LogTemp, Warning, TEXT("[HUD] %s 成功绑定击杀和死亡事件"), *GetNameSafe(this));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[HUD] %s 无法获取GameMode，可能是客户端"), *GetNameSafe(this));
			}
			
			// 初始显示
			FWarriorKillEvent DummyEvent;
			UpdateHUDKills(DummyEvent);

			// 初始化血条
			APawn* ControlledPawn = GetPawn();
			if (ControlledPawn)
			{
				if (UAbilitySystemComponent* ASC = ControlledPawn->FindComponentByClass<UAbilitySystemComponent>())
				{
					if (const UAttributeSet* AttributeSet = ASC->GetAttributeSet(UWarriorAttributeSet::StaticClass()))
					{
						const UWarriorAttributeSet* WarriorAttr = Cast<UWarriorAttributeSet>(AttributeSet);
						if (WarriorAttr)
						{
							float CurrentHealth = WarriorAttr->GetHealth();
							float MaxHealth = WarriorAttr->GetMaxHealth();
							UpdateHUDHealth(CurrentHealth, MaxHealth);
						}
					}
				}
			}

			// 20秒后隐藏控制提示
			FTimerHandle HintTimerHandle;
			GetWorldTimerManager().SetTimer(HintTimerHandle, [this]()
			{
				if (GameHUDWidget)
				{
					GameHUDWidget->HideControlsHint();
				}
			}, 20.0f, false);
		}
	}
}

void AWarriorHeroController::ShowPauseMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!PauseMenuClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PauseMenuClass is not set in WarriorHeroController!"));
		return;
	}

	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UPauseMenu>(this, PauseMenuClass);
	}

	if (PauseMenuWidget)
	{
		PauseMenuWidget->MenuSetup();
	}
}

void AWarriorHeroController::UpdateHUDKills(const FWarriorKillEvent& Event)
{
	if (!GameHUDWidget || !GetWorld())
	{
		return;
	}

	if (!PlayerState)
	{
		return;
	}

	// 击杀数 = 玩家分数（每次击杀增加分数）
	int32 TeamKills = FMath::Max(0, FMath::FloorToInt(PlayerState->GetScore()));
	
	// 死亡数 = PlayerState中的Deaths计数
	int32 TeamDeaths = 0;
	if (AWarriorPlayerState* WarriorPS = Cast<AWarriorPlayerState>(PlayerState))
	{
		TeamDeaths = WarriorPS->GetDeaths();
	}

	UE_LOG(LogTemp, Warning, TEXT("[HUD Update] Controller=%s, PlayerName=%s, TeamKills=%d, TeamDeaths=%d, Score=%.0f"), 
		*GetNameSafe(this),
		*PlayerState->GetPlayerName(),
		TeamKills, 
		TeamDeaths,
		PlayerState->GetScore());

	GameHUDWidget->UpdateKills(TeamKills, TeamDeaths);
}

void AWarriorHeroController::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	if (GameHUDWidget)
	{
		GameHUDWidget->UpdateHealthBar(CurrentHealth, MaxHealth);
	}
}

