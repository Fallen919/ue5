// Steam 联机系统 - 重新编写版本

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

// 自定义委托声明 - 避免和引擎委托冲突
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>&, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);

/**
 * Steam 联机会话管理子系统
 * 专为Steam好友联机设计
 */
UCLASS()
class ZHANSHI_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMultiplayerSessionsSubsystem();

    /**
     * 创建会话房间
     * @param NumPublicConnections 最大玩家数
     * @param MatchType 匹配类型标识
     * @param bUseLAN 是否使用LAN模式（true=局域网, false=Steam）
     */
    void CreateSession(int32 NumPublicConnections, FString MatchType, bool bUseLAN = false);

    /**
     * 搜索会话
     * @param MaxSearchResults 最大搜索结果数
     * @param bUseLAN 是否使用LAN模式（true=局域网, false=Steam）
     */
    void FindSessions(int32 MaxSearchResults, bool bUseLAN = false);

    /**
     * 加入指定的会话
     * @param SessionResult 要加入的会话结果
     */
    void JoinSession(const FOnlineSessionSearchResult& SessionResult);

    /**
     * 销毁当前会话
     */
    void DestroySession();

    // 地图路径
    FString LobbyMapPath = TEXT("/Game/Maps/GameMap");

    // 自定义委托
    FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
    FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
    FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
    FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;

protected:
    // 回调函数
    void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void HandleFindSessionsComplete(bool bWasSuccessful);
    void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
    // 在线接口
    IOnlineSessionPtr SessionInterface;
    
    // 会话设置和搜索
    TSharedPtr<FOnlineSessionSettings> CurrentSessionSettings;
    TSharedPtr<FOnlineSessionSearch> CurrentSessionSearch;

    // 委托句柄
    FDelegateHandle CreateSessionDelegateHandle;
    FDelegateHandle FindSessionsDelegateHandle;
    FDelegateHandle JoinSessionDelegateHandle;
    FDelegateHandle DestroySessionDelegateHandle;

    // 委托对象
    FOnCreateSessionCompleteDelegate CreateSessionDelegate;
    FOnFindSessionsCompleteDelegate FindSessionsDelegate;
    FOnJoinSessionCompleteDelegate JoinSessionDelegate;
    FOnDestroySessionCompleteDelegate DestroySessionDelegate;

    // 状态标记
    bool bShouldRecreateSession;
    int32 PendingNumConnections;
    FString PendingMatchType;
    bool bIsLANMode;  // 记录当前是否使用LAN模式

    // 辅助函数
    void LogSessionInfo(const FString& Message, bool bIsError = false);
    void ShowDebugMessage(const FString& Message, FColor Color = FColor::White, float Duration = 5.0f);
};
