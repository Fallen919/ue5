// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() : CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
                                                                 FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
                                                                 JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
                                                                 DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
                                                                 StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    IOnlineSubsystem *Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Blue,
                FString::Printf(TEXT("找到子系统: %s"), *Subsystem->GetSubsystemName().ToString()));
        }
    }
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface无效！"));
        return;
    }

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("已存在会话，先销毁..."));
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MatchType;

        DestroySession();
        return;
    }

    // 保存设置
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSettings->NumPublicConnections = NumPublicConnections;
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->bUseLobbiesIfAvailable = true;
    LastSessionSettings->bAllowInvites = true;
    LastSessionSettings->bIsDedicated = false;
    LastSessionSettings->bUsesStats = false;
    LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->BuildUniqueId = 1;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, 
            FString::Printf(TEXT("创建会话: 玩家=%d, 类型=%s, LAN=%d"), NumPublicConnections, *MatchType, LastSessionSettings->bIsLANMatch));
    }

    UE_LOG(LogTemp, Warning, TEXT("会话设置: LAN=%d, 最大玩家=%d, 匹配类型=%s"),
           LastSessionSettings->bIsLANMatch, NumPublicConnections, *MatchType);

    const ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

        // 广播失败
        MultiplayerOnCreateSessionComplete.Broadcast(false);

        UE_LOG(LogTemp, Error, TEXT("创建会话失败！"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("开始创建会话..."));
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface无效！"));
        return;
    }

    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

    FString SubsystemName = IOnlineSubsystem::Get()->GetSubsystemName().ToString();
    UE_LOG(LogTemp, Warning, TEXT("搜索设置: LAN=%d, 最大结果=%d, 子系统=%s"),
           LastSessionSearch->bIsLanQuery, MaxSearchResults, *SubsystemName);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, 
            FString::Printf(TEXT("开始查找会话 (子系统: %s)"), *SubsystemName));
    }

    const ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);

        UE_LOG(LogTemp, Error, TEXT("查找会话失败！"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("开始查找会话..."));
    }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult &SessionResult)
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    const ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);

        UE_LOG(LogTemp, Error, TEXT("加入会话失败！"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("开始加入会话..."));
    }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }

    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    if (!SessionInterface->DestroySession(NAME_GameSession))
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }

    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("创建会话成功！"));

        // 切换到配置的地图
        UWorld *World = GetWorld();
        if (World)
        {
            FString TravelURL = LobbyMapPath + "?listen";
            UE_LOG(LogTemp, Warning, TEXT("切换到地图: %s"), *TravelURL);
            World->ServerTravel(TravelURL);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("创建会话失败！"));
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }

    int32 NumResults = LastSessionSearch.IsValid() ? LastSessionSearch->SearchResults.Num() : 0;
    UE_LOG(LogTemp, Warning, TEXT("查找会话完成: 成功=%d, 找到数量=%d"),
           bWasSuccessful, NumResults);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, bWasSuccessful ? FColor::Green : FColor::Red, 
            FString::Printf(TEXT("查找完成: 找到 %d 个会话"), NumResults));
    }

    if (NumResults <= 0)
    {
        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        UE_LOG(LogTemp, Warning, TEXT("未找到任何会话"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, TEXT("未找到会话！检查:"));
            GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, TEXT("1. 是否Steam好友"));
            GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, TEXT("2. 对方是否已创建房间"));
            GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, TEXT("3. 防火墙设置"));
        }
        return;
    }

    // 打印所有找到的会话
    for (int32 i = 0; i < LastSessionSearch->SearchResults.Num(); i++)
    {
        auto &Result = LastSessionSearch->SearchResults[i];
        FString MatchType;
        Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

        int32 CurrentPlayers = Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections;
        int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;

        UE_LOG(LogTemp, Warning, TEXT("会话 %d: 主机=%s, Ping=%d, 匹配类型=%s, 玩家=%d/%d"),
               i,
               *Result.Session.OwningUserName,
               Result.PingInMs,
               *MatchType,
               CurrentPlayers,
               MaxPlayers);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, 
                FString::Printf(TEXT("会话 %d: %s [%d/%d] - %s"), 
                    i + 1, *Result.Session.OwningUserName, CurrentPlayers, MaxPlayers, *MatchType));
        }
    }

    MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);

    UE_LOG(LogTemp, Warning, TEXT("找到 %d 个会话"), LastSessionSearch->SearchResults.Num());
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }

    MultiplayerOnJoinSessionComplete.Broadcast(Result);

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("加入会话成功！"));

        // 获取连接地址并旅行
        FString Address;
        SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Yellow,
                FString::Printf(TEXT("连接地址: %s"), *Address));
        }

        APlayerController *PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
        if (PlayerController)
        {
            PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("加入会话失败！"));
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    if (bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(LastNumPublicConnections, LastMatchType);
    }

    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
