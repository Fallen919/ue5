// Steam 联机系统实现 - 重新编写版本

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
    CreateSessionDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleCreateSessionComplete)),
    FindSessionsDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::HandleFindSessionsComplete)),
    JoinSessionDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleJoinSessionComplete)),
    DestroySessionDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleDestroySessionComplete)),
    bShouldRecreateSession(false),
    PendingNumConnections(0),
    bIsLANMode(false)
{
    // 获取Steam在线子系统
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        
        FString SubsystemName = OnlineSubsystem->GetSubsystemName().ToString();
        LogSessionInfo(FString::Printf(TEXT("初始化联机子系统: %s"), *SubsystemName));
        
        if (SubsystemName != TEXT("STEAM"))
        {
            UE_LOG(LogTemp, Warning, TEXT("[联机系统] 当前子系统不是STEAM（%s），如果使用LAN模式则正常"), *SubsystemName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[联机系统] 无法获取在线子系统，联机功能将不可用"));
    }
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType, bool bUseLAN)
{
    bIsLANMode = bUseLAN;  // 保存当前模式
    
    if (!SessionInterface.IsValid())
    {
        LogSessionInfo(TEXT("创建会话失败: SessionInterface无效"), true);
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }

    // 检查是否存在旧会话
    FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession)
    {
        LogSessionInfo(TEXT("检测到已存在会话，先销毁旧会话..."));
        bShouldRecreateSession = true;
        PendingNumConnections = NumPublicConnections;
        PendingMatchType = MatchType;
        DestroySession();
        return;
    }

    // 注册创建会话委托
    CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegate);

    // 配置会话设置
    CurrentSessionSettings = MakeShareable(new FOnlineSessionSettings());
    
    // 基本设置
    CurrentSessionSettings->NumPublicConnections = NumPublicConnections;
    CurrentSessionSettings->bShouldAdvertise = true;  // 广播会话
    CurrentSessionSettings->bAllowJoinInProgress = true;
    CurrentSessionSettings->bIsDedicated = false;
    
    // 根据模式配置：LAN 或 Steam
    CurrentSessionSettings->bIsLANMatch = bUseLAN;
    
    if (bUseLAN)
    {
        // LAN局域网模式
        LogSessionInfo(TEXT("使用LAN局域网模式"));
        CurrentSessionSettings->bUsesPresence = false;
        CurrentSessionSettings->bUseLobbiesIfAvailable = false;
        CurrentSessionSettings->bAllowJoinViaPresence = false;
    }
    else
    {
        // Steam模式
        LogSessionInfo(TEXT("使用Steam联机模式"));
        CurrentSessionSettings->bAllowJoinViaPresence = true;  // 允许通过Steam状态加入
        CurrentSessionSettings->bUsesPresence = true;  // 使用Steam状态功能
        CurrentSessionSettings->bUseLobbiesIfAvailable = true;  // 使用Steam大厅
        CurrentSessionSettings->bAllowInvites = true;  // 允许Steam邀请
    }
    
    // 其他设置
    CurrentSessionSettings->bUsesStats = false;
    CurrentSessionSettings->bAntiCheatProtected = false;
    
    // 设置自定义搜索参数
    CurrentSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    CurrentSessionSettings->BuildUniqueId = 1;  // 确保版本匹配

    FString ModeText = bUseLAN ? TEXT("LAN局域网") : TEXT("Steam");
    LogSessionInfo(FString::Printf(TEXT("开始创建%s会话 [玩家数: %d, 类型: %s]"), *ModeText, NumPublicConnections, *MatchType));
    ShowDebugMessage(FString::Printf(TEXT("正在创建%s房间 (%d人)..."), *ModeText, NumPublicConnections), FColor::Cyan);

    // 获取本地玩家
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        LogSessionInfo(TEXT("创建会话失败: 无法获取本地玩家"), true);
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }

    // 创建会话
    bool bCreateSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *CurrentSessionSettings);
    
    if (!bCreateSuccess)
    {
        LogSessionInfo(TEXT("CreateSession调用失败"), true);
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        ShowDebugMessage(TEXT("创建房间失败！"), FColor::Red);
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults, bool bUseLAN)
{
    bIsLANMode = bUseLAN;  // 保存当前模式
    
    if (!SessionInterface.IsValid())
    {
        LogSessionInfo(TEXT("查找会话失败: SessionInterface无效"), true);
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }

    // 注册查找会话委托
    FindSessionsDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegate);

    // 配置会话搜索
    CurrentSessionSearch = MakeShareable(new FOnlineSessionSearch());
    CurrentSessionSearch->MaxSearchResults = MaxSearchResults;
    CurrentSessionSearch->bIsLanQuery = bUseLAN;  // 根据参数选择模式
    CurrentSessionSearch->TimeoutInSeconds = 10.0f;  // 搜索超时时间
    
    if (!bUseLAN)
    {
        // Steam会话搜索的关键设置
        CurrentSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        CurrentSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    }
    
    CurrentSessionSearch->QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, false, EOnlineComparisonOp::Equals);
    CurrentSessionSearch->QuerySettings.Set(SEARCH_SECURE_SERVERS_ONLY, false, EOnlineComparisonOp::Equals);

    FString ModeText = bUseLAN ? TEXT("LAN局域网") : TEXT("Steam");
    LogSessionInfo(FString::Printf(TEXT("开始搜索%s会话 [最大结果: %d]"), *ModeText, MaxSearchResults));
    ShowDebugMessage(FString::Printf(TEXT("正在搜索%s房间..."), *ModeText), FColor::Cyan);

    // 获取本地玩家
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        LogSessionInfo(TEXT("查找会话失败: 无法获取本地玩家"), true);
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }

    // 开始搜索
    bool bFindSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), CurrentSessionSearch.ToSharedRef());
    
    if (!bFindSuccess)
    {
        LogSessionInfo(TEXT("FindSessions调用失败"), true);
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        ShowDebugMessage(TEXT("搜索房间失败！"), FColor::Red);
    }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (!SessionInterface.IsValid())
    {
        LogSessionInfo(TEXT("加入会话失败: SessionInterface无效"), true);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    // 注册加入会话委托
    JoinSessionDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegate);

    LogSessionInfo(FString::Printf(TEXT("尝试加入会话: %s"), *SessionResult.Session.OwningUserName));
    ShowDebugMessage(FString::Printf(TEXT("正在加入 %s 的房间..."), *SessionResult.Session.OwningUserName), FColor::Cyan);

    // 获取本地玩家
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        LogSessionInfo(TEXT("加入会话失败: 无法获取本地玩家"), true);
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    // 加入会话
    bool bJoinSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult);
    
    if (!bJoinSuccess)
    {
        LogSessionInfo(TEXT("JoinSession调用失败"), true);
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        ShowDebugMessage(TEXT("加入房间失败！"), FColor::Red);
    }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (!SessionInterface.IsValid())
    {
        LogSessionInfo(TEXT("销毁会话失败: SessionInterface无效"), true);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }

    // 注册销毁会话委托
    DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegate);

    LogSessionInfo(TEXT("开始销毁会话..."));

    bool bDestroySuccess = SessionInterface->DestroySession(NAME_GameSession);
    
    if (!bDestroySuccess)
    {
        LogSessionInfo(TEXT("DestroySession调用失败"), true);
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
    }
}

// ========== 回调函数实现 ==========

void UMultiplayerSessionsSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    // 清理委托
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
    }

    if (bWasSuccessful)
    {
        FString ModeText = bIsLANMode ? TEXT("LAN局域网") : TEXT("Steam");
        LogSessionInfo(FString::Printf(TEXT("✓ %s会话创建成功！"), *ModeText));
        ShowDebugMessage(FString::Printf(TEXT("✓ %s房间创建成功！"), *ModeText), FColor::Green, 10.0f);
        
        // 切换到游戏地图（作为主机）
        UWorld* World = GetWorld();
        if (World)
        {
            FString TravelURL = FString::Printf(TEXT("%s?listen"), *LobbyMapPath);
            LogSessionInfo(FString::Printf(TEXT("切换到游戏地图: %s"), *TravelURL));
            World->ServerTravel(TravelURL);
        }
    }
    else
    {
        LogSessionInfo(TEXT("✗ 会话创建失败！"), true);
        ShowDebugMessage(TEXT("✗ 房间创建失败！"), FColor::Red, 10.0f);
    }

    // 广播结果
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
    // 清理委托
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
    }

    int32 NumResults = CurrentSessionSearch.IsValid() ? CurrentSessionSearch->SearchResults.Num() : 0;
    
    FString ModeText = bIsLANMode ? TEXT("LAN局域网") : TEXT("Steam");
    
    LogSessionInfo(TEXT("========================================"));
    LogSessionInfo(FString::Printf(TEXT("查找%s会话完成 [成功: %s, 结果数: %d]"), 
        *ModeText, bWasSuccessful ? TEXT("是") : TEXT("否"), NumResults));

    if (NumResults == 0)
    {
        LogSessionInfo(TEXT("未找到任何会话"));
        ShowDebugMessage(FString::Printf(TEXT("未找到%s会话！"), *ModeText), FColor::Yellow, 10.0f);
        
        if (bIsLANMode)
        {
            ShowDebugMessage(TEXT("请确认："), FColor::Yellow, 10.0f);
            ShowDebugMessage(TEXT("1. 两台电脑在同一局域网"), FColor::Yellow, 10.0f);
            ShowDebugMessage(TEXT("2. 对方已创建房间"), FColor::Yellow, 10.0f);
            ShowDebugMessage(TEXT("3. 防火墙设置"), FColor::Yellow, 10.0f);
        }
        else
        {
            ShowDebugMessage(TEXT("请确认："), FColor::Yellow, 10.0f);
            ShowDebugMessage(TEXT("1. 是否Steam好友"), FColor::Yellow, 10.0f);
            ShowDebugMessage(TEXT("2. 对方是否已创建房间"), FColor::Yellow, 10.0f);
            ShowDebugMessage(TEXT("3. 防火墙设置"), FColor::Yellow, 10.0f);
        }
        
        IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
        if (OnlineSubsystem)
        {
            FString SubsystemName = OnlineSubsystem->GetSubsystemName().ToString();
            ShowDebugMessage(FString::Printf(TEXT("当前子系统: %s"), *SubsystemName), FColor::Cyan, 10.0f);
        }
        
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        LogSessionInfo(TEXT("========================================"));
        return;
    }

    // 打印所有找到的会话
    for (int32 i = 0; i < CurrentSessionSearch->SearchResults.Num(); i++)
    {
        const FOnlineSessionSearchResult& Result = CurrentSessionSearch->SearchResults[i];
        
        FString MatchType;
        Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
        
        int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
        int32 CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;
        
        LogSessionInfo(FString::Printf(TEXT("[房间%d] 主机: %s | 玩家: %d/%d | Ping: %dms | 类型: %s"), 
            i + 1, 
            *Result.Session.OwningUserName, 
            CurrentPlayers, 
            MaxPlayers, 
            Result.PingInMs,
            *MatchType));
        
        ShowDebugMessage(FString::Printf(TEXT("房间%d: %s [%d/%d人]"), 
            i + 1, *Result.Session.OwningUserName, CurrentPlayers, MaxPlayers), 
            FColor::Green, 10.0f);
    }
    
    LogSessionInfo(FString::Printf(TEXT("查找完成: 找到 %d 个会话"), NumResults));
    ShowDebugMessage(FString::Printf(TEXT("查找完成: 找到 %d 个会话"), NumResults), FColor::Green, 10.0f);
    LogSessionInfo(TEXT("========================================"));

    // 广播结果
    MultiplayerOnFindSessionsComplete.Broadcast(CurrentSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    // 清理委托
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
    }

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        LogSessionInfo(TEXT("✓ 成功加入会话！"));
        ShowDebugMessage(TEXT("✓ 加入成功！正在连接..."), FColor::Green, 10.0f);

        // 获取连接地址
        FString ConnectAddress;
        if (SessionInterface.IsValid() && SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectAddress))
        {
            LogSessionInfo(FString::Printf(TEXT("连接地址: %s"), *ConnectAddress));

            // 开始旅行到主机
            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
            if (PlayerController)
            {
                PlayerController->ClientTravel(ConnectAddress, ETravelType::TRAVEL_Absolute);
            }
            else
            {
                LogSessionInfo(TEXT("错误: 无法获取玩家控制器"), true);
            }
        }
        else
        {
            LogSessionInfo(TEXT("错误: 无法获取连接地址"), true);
            ShowDebugMessage(TEXT("✗ 获取连接地址失败！"), FColor::Red, 10.0f);
        }
    }
    else
    {
        FString ErrorMsg;
        switch (Result)
        {
        case EOnJoinSessionCompleteResult::SessionIsFull:
            ErrorMsg = TEXT("房间已满");
            break;
        case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            ErrorMsg = TEXT("会话不存在");
            break;
        case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
            ErrorMsg = TEXT("无法获取地址");
            break;
        default:
            ErrorMsg = TEXT("未知错误");
            break;
        }
        
        LogSessionInfo(FString::Printf(TEXT("✗ 加入会话失败: %s"), *ErrorMsg), true);
        ShowDebugMessage(FString::Printf(TEXT("✗ 加入失败: %s"), *ErrorMsg), FColor::Red, 10.0f);
    }

    // 广播结果
    MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    // 清理委托
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
    }

    if (bWasSuccessful)
    {
        LogSessionInfo(TEXT("✓ 会话销毁成功"));
        
        // 如果需要重新创建会话
        if (bShouldRecreateSession)
        {
            bShouldRecreateSession = false;
            LogSessionInfo(TEXT("准备重新创建会话..."));
            CreateSession(PendingNumConnections, PendingMatchType);
        }
    }
    else
    {
        LogSessionInfo(TEXT("✗ 会话销毁失败"), true);
    }

    // 广播结果
    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

// ========== 辅助函数实现 ==========

void UMultiplayerSessionsSubsystem::LogSessionInfo(const FString& Message, bool bIsError)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Warning, TEXT("[联机系统] %s"), *Message);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[联机系统] %s"), *Message);
    }
}

void UMultiplayerSessionsSubsystem::ShowDebugMessage(const FString& Message, FColor Color, float Duration)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
    }
}
