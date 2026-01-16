// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animlnstances/WarriorAnimInstance.h"
#include "WarriorCharactersAnimInstance.generated.h"
class AWarriorBaseCharacter;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class ZHANSHI_API UWarriorCharactersAnimInstance : public UWarriorAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation()override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds)override;
	
protected:
	UPROPERTY()
	AWarriorBaseCharacter* OwningCharacter;

	UPROPERTY()
	UCharacterMovementComponent* OwningMovementComponent;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category = "AnimData|LocomtionData")
	float GroundSpeed;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomtionData")
	bool bHasAcceleration;
};
