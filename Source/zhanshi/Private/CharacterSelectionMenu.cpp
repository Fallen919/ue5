// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterSelectionMenu.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerStates/WarriorPlayerState.h"
#include "WarriorGameInstance.h"

bool UCharacterSelectionMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (Button_Character0)
    {
        Button_Character0->OnClicked.AddDynamic(this, &UCharacterSelectionMenu::OnCharacter0Clicked);
    }

    if (Button_Character1)
    {
        Button_Character1->OnClicked.AddDynamic(this, &UCharacterSelectionMenu::OnCharacter1Clicked);
    }

    if (Button_Confirm)
    {
        Button_Confirm->OnClicked.AddDynamic(this, &UCharacterSelectionMenu::OnConfirmClicked);
    }

    if (Button_Cancel)
    {
        Button_Cancel->OnClicked.AddDynamic(this, &UCharacterSelectionMenu::OnCancelClicked);
    }

    return true;
}

void UCharacterSelectionMenu::Setup(int32 CurrentSelection)
{
    SelectedCharacterIndex = CurrentSelection;

    AddToViewport(10);
    SetVisibility(ESlateVisibility::Visible);

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }
}

void UCharacterSelectionMenu::OnCharacter0Clicked()
{
    SelectedCharacterIndex = 0;
    UE_LOG(LogTemp, Warning, TEXT("Selected Character 0"));
}

void UCharacterSelectionMenu::OnCharacter1Clicked()
{
    SelectedCharacterIndex = 1;
    UE_LOG(LogTemp, Warning, TEXT("Selected Character 1"));
}

void UCharacterSelectionMenu::OnConfirmClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Confirmed Character Selection: %d"), SelectedCharacterIndex);
    
	// 保存选择到PlayerState（会复制到服务器）
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AWarriorPlayerState* WarriorPS = PC->GetPlayerState<AWarriorPlayerState>();
		if (WarriorPS)
		{
			WarriorPS->SetSelectedCharacterIndex(SelectedCharacterIndex);
			UE_LOG(LogTemp, Warning, TEXT("Saved character selection to PlayerState: %d"), SelectedCharacterIndex);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get WarriorPlayerState!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get PlayerController!"));
	}

	// 也保存到GameInstance作为后备
	UGameInstance* GameInst = GetGameInstance();
	if (GameInst)
	{
		UWarriorGameInstance* WarriorGameInst = Cast<UWarriorGameInstance>(GameInst);
		if (WarriorGameInst)
		{
			WarriorGameInst->SetSelectedCharacterIndex(SelectedCharacterIndex);
			UE_LOG(LogTemp, Warning, TEXT("Saved character selection to WarriorGameInstance: %d"), SelectedCharacterIndex);
		}
	}
	
	// 关闭UI
	RemoveFromParent();
	
	UWorld *World = GetWorld();
	if (World)
	{
		APlayerController *PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
		}
	}
}

void UCharacterSelectionMenu::OnCancelClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Cancelled Character Selection"));
    
    // 关闭UI
    RemoveFromParent();

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
        }
    }
}
