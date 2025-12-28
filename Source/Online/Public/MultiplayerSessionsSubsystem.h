// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

// 自定义委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult> &SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 多人游戏会话子系统 - 处理所有联机功能
 */
UCLASS()
class ONLINE_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMultiplayerSessionsSubsystem();

    // 公开API
    void CreateSession(int32 NumPublicConnections, FString MatchType);
    void FindSessions(int32 MaxSearchResults);
    void JoinSession(const FOnlineSessionSearchResult &SessionResult);
    void DestroySession();
    void StartSession();

    // 设置创建会话后要切换的地图路径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
    FString LobbyMapPath = "/Game/ThirdPerson/Maps/GameMap";

    // 自定义委托
    FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
    FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
    FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
    FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
    FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

protected:
    // 内部回调
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
    TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

    // 委托句柄
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;

    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegateHandle;

    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;

    FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
    FDelegateHandle DestroySessionCompleteDelegateHandle;

    FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
    FDelegateHandle StartSessionCompleteDelegateHandle;

    bool bCreateSessionOnDestroy{false};
    int32 LastNumPublicConnections;
    FString LastMatchType;
};
