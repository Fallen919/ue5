// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorBaseCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameModes/WarriorBaseMode.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AWarriorBaseCharacter::AWarriorBaseCharacter()
{
 	
    PrimaryActorTick.bCanEverTick = false;

   
    PrimaryActorTick.bStartWithTickEnabled = false;
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

void AWarriorBaseCharacter::HandleDeath(const FGameplayEffectSpec& EffectSpec, float Damage)
{
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

    // Disable collision so dead character cannot be attacked
    SetActorEnableCollision(false);
    
    // Disable movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->SetMovementMode(EMovementMode::MOVE_None);
    }

    const FGameplayEffectContextHandle& ContextHandle = EffectSpec.GetEffectContext();
    AActor* InstigatorActor = ContextHandle.GetOriginalInstigator();
    if (!InstigatorActor)
    {
        InstigatorActor = ContextHandle.GetInstigator();
    }

    AController* InstigatorController = nullptr;
    if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
    {
        InstigatorController = InstigatorPawn->GetController();
    }
    if (!InstigatorController && InstigatorActor)
    {
        InstigatorController = InstigatorActor->GetInstigatorController();
    }

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

    if (AWarriorBaseMode* GameMode = GetWorld()->GetAuthGameMode<AWarriorBaseMode>())
    {
        FWarriorKillEvent Event;
        Event.KillerPS = InstigatorController ? InstigatorController->PlayerState : nullptr;
        Event.VictimPS = GetPlayerState();
        Event.DamageTag = DamageTag;
        Event.bFriendlyFire = true;

        GameMode->HandleKillEvent(Event);
    }
}


void AWarriorBaseCharacter::ResetForRespawn()
{
	bIsDead = false;

	SetActorEnableCollision(true);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		if (bCachedCollisionDefaults)
		{
			Capsule->SetCollisionEnabled(DefaultCapsuleCollisionEnabled);
			Capsule->SetCollisionResponseToChannels(DefaultCapsuleResponses);
		}
		else
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
	}

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
