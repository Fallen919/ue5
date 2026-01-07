// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/BoxComponent.h>
#include "WarriorWeaponBase.generated.h"


UCLASS()
class ZHANSHI_API AWarriorWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWarriorWeaponBase();

protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadonly,Category="Weapons")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Weapons")
	UBoxComponent* WeaponCollisionBox;
	
public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox()const
	{
		return WeaponCollisionBox;
	}
};
