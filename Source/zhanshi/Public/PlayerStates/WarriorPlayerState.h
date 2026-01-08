// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorPlayerState.generated.h"

UCLASS()
class ZHANSHI_API AWarriorPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void SetCharacterAppearance(const FWarriorCharacterAppearance& InAppearance);
	const FWarriorCharacterAppearance& GetCharacterAppearance() const { return CharacterAppearance; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterAppearance)
	FWarriorCharacterAppearance CharacterAppearance;

	UFUNCTION()
	void OnRep_CharacterAppearance();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
