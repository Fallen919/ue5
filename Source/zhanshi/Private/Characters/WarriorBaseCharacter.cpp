// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorBaseCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameModes/WarriorBaseMode.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"



AWarriorBaseCharacter::AWarriorBaseCharacter()
{

    // 完全禁用Actor的Tick系统，提升性能
    PrimaryActorTick.bCanEverTick = false;

    // 禁止在Tick启用时自动启用
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 禁用网格体贴花效果（血迹、弹孔等），减少渲染开销
    GetMesh()->bReceivesDecals = false;

    WarriorAbilitySystemComponent=CreateDefaultSubobject<UWarriorAbilitySystemComponent>(TEXT("WarriorAbilitySystemComponent"));
    // 启用组件复制
    WarriorAbilitySystemComponent->SetIsReplicated(true);
    // 设置复制模式为 Mixed，确保多人游戏中能力能正确复制到客户端
    WarriorAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    WarriorAttributeSet = CreateDefaultSubobject<UWarriorAttributeSet>(TEXT("WarriorAttributeSet"));
}

void AWarriorBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		DefaultCapsuleCollisionEnabled = Capsule->GetCollisionEnabled();
		DefaultCapsuleResponses = Capsule->GetCollisionResponseToChannels();
		bCachedCollisionDefaults = true;
	}
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

// 死亡处理函数：处理角色死亡的完整流程
void AWarriorBaseCharacter::HandleDeath(const FGameplayEffectSpec& EffectSpec, float Damage)
{
    // 权威性检查：只在服务器执行死亡逻辑，防止客户端作弊
    if (bIsDead || !HasAuthority())
    {
        return;
    }

    if (Damage <= 0.f)
    {
        return;
    }

    bIsDead = true;
    UE_LOG(LogTemp, Warning, TEXT("[Death] Victim=%s Damage=%.1f"), *GetName(), Damage);

    // 第一步：禁用碰撞，防止尸体被重复攻击
    SetActorEnableCollision(false);
    
    // 第二步：停止角色移动
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->SetMovementMode(EMovementMode::MOVE_None);
    }

    // 第三步：提取伤害来源信息，用于击杀事件
    const FGameplayEffectContextHandle& ContextHandle = EffectSpec.GetEffectContext();
    AActor* InstigatorActor = ContextHandle.GetOriginalInstigator();
    if (!InstigatorActor)
    {
        InstigatorActor = ContextHandle.GetInstigator();
    }

    // 获取击杀者的Controller（用于记录击杀者的PlayerState）
    AController* InstigatorController = nullptr;
    if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
    {
        InstigatorController = InstigatorPawn->GetController();
    }
    if (!InstigatorController && InstigatorActor)
    {
        InstigatorController = InstigatorActor->GetInstigatorController();
    }

    // 提取伤害标签（如近战伤害、远程伤害等）
    FGameplayTag DamageTag;
    FGameplayTagContainer AssetTags;
    EffectSpec.GetAllAssetTags(AssetTags);
    if (!AssetTags.IsEmpty())
    {
        TArray<FGameplayTag> TagArray;
        AssetTags.GetGameplayTagArray(TagArray);
        if (TagArray.Num() > 0)
        {
            DamageTag = TagArray[0];
        }
    }

    // 第四步：向GameMode发送击杀事件，让规则系统处理计分、UI等
    if (AWarriorBaseMode* GameMode = GetWorld()->GetAuthGameMode<AWarriorBaseMode>())
    {
        FWarriorKillEvent Event;
        Event.KillerPS = InstigatorController ? InstigatorController->PlayerState : nullptr;
        Event.VictimPS = GetPlayerState();
        Event.DamageTag = DamageTag;
        Event.bFriendlyFire = true;

        GameMode->HandleKillEvent(Event); // 通知GameMode处理击杀
    }
}


// 重生重置函数：恢复角色到可用状态
void AWarriorBaseCharacter::ResetForRespawn()
{
	// 第一步：清除死亡标记
	bIsDead = false;

	// 第二步：恢复碰撞（使角色可以被攻击和交互）
	SetActorEnableCollision(true);

	// 第三步：恢复碰撞配置（使用BeginPlay时缓存的默认配置）
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		if (bCachedCollisionDefaults)
		{
			// 使用缓存的默认碰撞设置
			Capsule->SetCollisionEnabled(DefaultCapsuleCollisionEnabled);
			Capsule->SetCollisionResponseToChannels(DefaultCapsuleResponses);
		}
		else
		{
			// 降级方案：使用默认值
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}

	// 第四步：恢复移动能力
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	// 第五步：恢复满血（只在服务器执行）
	if (HasAuthority() && WarriorAttributeSet)
	{
		const float MaxHealth = WarriorAttributeSet->GetMaxHealth();
		WarriorAttributeSet->SetHealth(MaxHealth);
		WarriorAttributeSet->SetIncomingDamage(0.f);
	}

	// Clear all loose gameplay tags to reset state
	if (WarriorAbilitySystemComponent)
	{
		FGameplayTagContainer TagsToRemove;
		WarriorAbilitySystemComponent->GetOwnedGameplayTags(TagsToRemove);
		
		// Remove all loose tags (tags not granted by abilities or effects)
		for (const FGameplayTag& Tag : TagsToRemove)
		{
			WarriorAbilitySystemComponent->RemoveLooseGameplayTag(Tag);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[Respawn] Cleared %d gameplay tags for %s"), 
			TagsToRemove.Num(), *GetName());
	}
}
