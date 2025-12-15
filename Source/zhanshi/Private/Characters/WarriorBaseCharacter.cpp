// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorBaseCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"


// Sets default values
AWarriorBaseCharacter::AWarriorBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    // 完全禁用Actor的Tick系统，提升性能
    PrimaryActorTick.bCanEverTick = false;

    // 初始禁用Tick但保留启用能力
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 禁用网格体接收贴花效果（血迹、弹孔等），提升渲染性能
    GetMesh()->bReceivesDecals = false;

    WarriorAbilitySystemComponent=CreateDefaultSubobject<UWarriorAbilitySystemComponent>(TEXT("WarriorAbilitySystemComponent"));

    WarriorAttributeSet = CreateDefaultSubobject<UWarriorAttributeSet>(TEXT("WarriorAttributeSet"));
}

UAbilitySystemComponent* AWarriorBaseCharacter::GetAbilitySystemComponent() const
{
    return GetWarriorAbilitySystemComponent();
}

void AWarriorBaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if (WarriorAbilitySystemComponent)
    {
        WarriorAbilitySystemComponent->InitAbilityActorInfo(this, this);
        ensureMsgf(!CharacterStartUpData.IsNull(), TEXT("ForGot to assign start up data to %s"), *GetName());

        
    }
}



