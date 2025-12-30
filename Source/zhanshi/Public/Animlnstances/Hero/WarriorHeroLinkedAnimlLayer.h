// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animlnstances/WarriorAnimInstance.h"
#include "WarriorHeroLinkedAnimlLayer.generated.h"

class UWarriorHeroAnimInstance;
/**
 * 
 */
UCLASS()
class ZHANSHI_API UWarriorHeroLinkedAnimlLayer : public UWarriorAnimInstance
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure,meta=(BlueprintThreadSafe))
	UWarriorHeroAnimInstance* GetWarriorHeroAnimInstance()const;
};
