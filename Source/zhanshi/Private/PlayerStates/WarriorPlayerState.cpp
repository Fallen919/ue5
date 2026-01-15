// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/WarriorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/WarriorHeroCharacter.h"

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

void AWarriorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWarriorPlayerState, CharacterAppearance);
}
