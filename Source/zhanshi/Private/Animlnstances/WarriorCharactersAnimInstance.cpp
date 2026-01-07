// Fill out your copyright notice in the Description page of Project Settings.


#include "Animlnstances/WarriorCharactersAnimInstance.h"
#include "Characters/WarriorBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UWarriorCharactersAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter=Cast<AWarriorBaseCharacter>(TryGetPawnOwner());
	if (OwningCharacter) {
		OwningMovementComponent=OwningCharacter->GetCharacterMovement();
	}
}

void UWarriorCharactersAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if(!OwningCharacter||!OwningMovementComponent)
	{
		return;
	}
	GroundSpeed= OwningCharacter->GetVelocity().Size2D();
	bHasAcceleration=OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;

}
