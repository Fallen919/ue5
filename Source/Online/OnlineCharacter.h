// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Blueprint/UserWidget.h"

#include "OnlineCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class AOnlineCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent *CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent *FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext *DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *LookAction;

	/** Pause Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction *PauseAction;

public:
	AOnlineCharacter();

	/** 暂停菜单Widget类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue &Value);

	/** Called when pressing pause/ESC key */

	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu();

private:
	/** 暂停菜单实例 */
	UPROPERTY()
	UUserWidget *PauseMenuWidget;

protected:
	/** Called for looking input */
	void Look(const FInputActionValue &Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	// To add mapping context
	virtual void BeginPlay();

	// Client RPC to setup input on client
	UFUNCTION(Client, Reliable)
	void ClientSetupInput();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent *GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent *GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void OpenLobby();

	UFUNCTION(BlueprintCallable)
	void CallOpenLevel(const FString &Address);

	UFUNCTION(BlueprintCallable)
	void CallClientTravel(const FString &Address);

public:
	IOnlineSessionPtr OnlineSessionInterface;

	UFUNCTION(BlueprintCallable)
	void CreateGameSession();

	UFUNCTION(BlueprintCallable)
	void JoinGameSession();

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
};
