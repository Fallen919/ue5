// Fill out your copyright notice in the Description page of Project Settings.


#include "Animlnstances/Hero/WarriorHeroLinkedAnimlLayer.h"
#include "Animlnstances/Hero/WarriorHeroAnimInstance.h"

UWarriorHeroAnimInstance* UWarriorHeroLinkedAnimlLayer::GetWarriorHeroAnimInstance() const
{
	return Cast<UWarriorHeroAnimInstance>(GetOwningComponent()->GetAnimInstance());

}
