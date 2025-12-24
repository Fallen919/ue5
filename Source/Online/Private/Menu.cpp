// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (Button_CreateGame)
	{
		Button_CreateGame->OnClicked.AddDynamic(this, &UMenu::OnCreateGameClicked);
	}

	if (Button_JoinGame)
	{
		Button_JoinGame->OnClicked.AddDynamic(this, &UMenu::OnJoinGameClicked);
	}

	if (Button_Back)
	{
		Button_Back->OnClicked.AddDynamic(this, &UMenu::OnBackClicked);
	}

	// 获取多人游戏子系统
	UGameInstance *GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

		if (MultiplayerSessionsSubsystem)
		{
			// 绑定回调（只绑定一次）
			if (!MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.Contains(this, FName("OnCreateSession")))
			{
				MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);
				MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenu::OnFindSessions);
				MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
			}
		}
	}

	return true;
}

void UMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld *World = GetWorld();
	if (World)
	{
		APlayerController *PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void UMenu::MenuTeardown()
{
	RemoveFromParent();
	UWorld *World = GetWorld();
	if (World)
	{
		APlayerController *PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu::OnCreateGameClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("创造游戏按钮被点击！"));

	if (MultiplayerSessionsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("开始创建会话..."));
		MultiplayerSessionsSubsystem->CreateSession(4, FString("FreeForAll"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MultiplayerSessionsSubsystem 无效！"));
	}
}

void UMenu::OnJoinGameClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("加入游戏按钮被点击！"));

	if (MultiplayerSessionsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("开始查找会话..."));
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MultiplayerSessionsSubsystem 无效！"));
	}
}

void UMenu::OnBackClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("返回按钮被点击！"));

	// 移除当前菜单
	RemoveFromParent();

	// 显示主菜单
	if (MainMenuClass)
	{
		UWorld *World = GetWorld();
		if (World)
		{
			APlayerController *PlayerController = World->GetFirstPlayerController();
			if (PlayerController)
			{
				MainMenuWidget = CreateWidget<UUserWidget>(PlayerController, MainMenuClass);
				if (MainMenuWidget)
				{
					UE_LOG(LogTemp, Warning, TEXT("创建主菜单Widget成功"));
					MainMenuWidget->AddToViewport();
					MainMenuWidget->SetVisibility(ESlateVisibility::Visible);

					// 保持UI输入模式
					FInputModeUIOnly InputMode;
					InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					PlayerController->SetInputMode(InputMode);
					PlayerController->SetShowMouseCursor(true);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("创建主菜单Widget失败！"));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuClass未设置！请在WBP_Menu蓝图中设置"));
	}
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("创建会话成功！菜单将自动关闭"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, TEXT("创建会话成功！"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("创建会话失败！"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("创建会话失败！"));
		}
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult> &SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("未找到会话"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, TEXT("未找到游戏会话！"));
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("找到 %d 个会话"), SessionResults.Num());

	// 遍历所有会话，找到第一个匹配的加入
	for (auto Result : SessionResults)
	{
		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Cyan,
				FString::Printf(TEXT("找到会话: %s"), *Result.Session.OwningUserName));
		}

		if (MatchType == FString("FreeForAll"))
		{
			UE_LOG(LogTemp, Warning, TEXT("找到匹配的会话，正在加入..."));
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("加入会话成功！"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, TEXT("加入会话成功！"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("加入会话失败！"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("加入会话失败！"));
		}
	}
}
