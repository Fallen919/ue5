// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CharacterSelectionMenu.generated.h"

/**
 * 角色选择菜单UI
 */
UCLASS()
class ZHANSHI_API UCharacterSelectionMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void Setup(int32 CurrentSelection);

protected:
    virtual bool Initialize() override;

private:
    UPROPERTY(meta = (BindWidget))
    UButton *Button_Character0;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_Character1;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_Confirm;

    UPROPERTY(meta = (BindWidget))
    UButton *Button_Cancel;

    UFUNCTION()
    void OnCharacter0Clicked();

    UFUNCTION()
    void OnCharacter1Clicked();

    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();

    // 当前选中的角色索引
    int32 SelectedCharacterIndex = 0;
};
