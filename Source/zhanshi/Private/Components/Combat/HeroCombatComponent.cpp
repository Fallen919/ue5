// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/HeroCombatComponent.h"
#include "Items/Weapons/WarriorHeroWeapon.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"

AWarriorHeroWeapon* UHeroCombatComponent::GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
    return Cast<AWarriorHeroWeapon>(GetCharacterCarriedWeaponByTag(InWeaponTag));
   
}

void UHeroCombatComponent::ClientAddInputMappingContext_Implementation(UInputMappingContext* MappingContext, int32 Priority)
{
	if (!MappingContext)
	{
		return;
	}

	APawn* OwningPawn = GetOwningPawn();
	if (!OwningPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem && !Subsystem->HasMappingContext(MappingContext))
	{
		Subsystem->AddMappingContext(MappingContext, Priority);
		UE_LOG(LogTemp, Log, TEXT("[Client] Added Input Mapping Context: %s"), *MappingContext->GetName());
	}
}

void UHeroCombatComponent::ClientRemoveInputMappingContext_Implementation(UInputMappingContext* MappingContext)
{
	if (!MappingContext)
	{
		return;
	}

	APawn* OwningPawn = GetOwningPawn();
	if (!OwningPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem && Subsystem->HasMappingContext(MappingContext))
	{
		Subsystem->RemoveMappingContext(MappingContext);
		UE_LOG(LogTemp, Log, TEXT("[Client] Removed Input Mapping Context: %s"), *MappingContext->GetName());
	}
}
