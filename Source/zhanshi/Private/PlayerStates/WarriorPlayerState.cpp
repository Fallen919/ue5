// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/WarriorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/WarriorHeroCharacter.h"

void AWarriorPlayerState::IncrementDeaths()
{
	Deaths++;
}

void AWarriorPlayerState::MarkFirstKill()
{
	bHasCompletedFirstKill = true;
}

void AWarriorPlayerState::MarkFirstDeath()
{
	bHasCompletedFirstDeath = true;
}

void AWarriorPlayerState::SetSelectedCharacterIndex(int32 Index)
{
	// 如果是客户端，通过Server RPC设置
	if (!HasAuthority())
	{
		ServerSetSelectedCharacterIndex(Index);
		UE_LOG(LogTemp, Warning, TEXT("[SetSelectedCharacterIndex] Client requesting server to set index: %d"), Index);
	}
	else
	{
		// 服务器直接设置
		SelectedCharacterIndex = Index;
		UE_LOG(LogTemp, Warning, TEXT("[SetSelectedCharacterIndex] Server set index: %d"), Index);
	}
}

void AWarriorPlayerState::ServerSetSelectedCharacterIndex_Implementation(int32 Index)
{
	SelectedCharacterIndex = Index;
	UE_LOG(LogTemp, Warning, TEXT("[ServerSetSelectedCharacterIndex] Server received and set index: %d for %s"), Index, *GetPlayerName());
}

void AWarriorPlayerState::SetCharacterAppearance(const FWarriorCharacterAppearance& InAppearance)
{
	CharacterAppearance = InAppearance;
	OnRep_CharacterAppearance();
}

void AWarriorPlayerState::OnRep_CharacterAppearance()
{
	if (APawn* Pawn = GetPawn())
	{
		if (AWarriorHeroCharacter* HeroCharacter = Cast<AWarriorHeroCharacter>(Pawn))
		{
			HeroCharacter->ApplyCharacterAppearance(CharacterAppearance);
		}
	}
}

void AWarriorPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	// Score变化时，通知Controller更新HUD
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnRep_Score] %s Score changed to %.0f"), *GetPlayerName(), GetScore());
	}
}

void AWarriorPlayerState::OnRep_Deaths()
{
	// Deaths变化时，通知Controller更新HUD
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnRep_Deaths] %s Deaths changed to %d"), *GetPlayerName(), Deaths);
	}
}

void AWarriorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWarriorPlayerState, CharacterAppearance);
	DOREPLIFETIME(AWarriorPlayerState, Deaths);
	DOREPLIFETIME(AWarriorPlayerState, SelectedCharacterIndex);
	DOREPLIFETIME(AWarriorPlayerState, bHasCompletedFirstKill);
	DOREPLIFETIME(AWarriorPlayerState, bHasCompletedFirstDeath);
	DOREPLIFETIME(AWarriorPlayerState, bIgnoreStats);
}
