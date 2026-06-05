// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ABCharacterBase.h"
#include "InputActionValue.h"
#include "Interface/ABCharacterHUDInterface.h"
#include "ABCharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABCharacterPlayer : public AABCharacterBase, public IABCharacterHUDInterface
{
	GENERATED_BODY()
	
public:
	AABCharacterPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetDead() override;
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Character Control Section
protected:
	void ChangeCharacterControl();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UABCharacterControlData* CharacterControlData) override;

// Camera Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

// Input Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeControlAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuaterMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	void ShoulderMove(const FInputActionValue& Value);
	void ShoulderLook(const FInputActionValue& Value);

	void QuaterMove(const FInputActionValue& Value);

	ECharacterControlType CurrentCharacterControlType;

	void Attack();

	// 공격 애니메이션 재생 함수.
	void PlayAttackAnimation();

	// 공격 처리 관련 인터페이스 함수 오버라이드.
	virtual void AttackHitCheck() override;

	// 공격 명령 처리를 위한 ServerRPC.
	// 클라이언트가 서버로 요청할 때 요청한 시간을 보내도록.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCAttack(float AttackStartTime);

	// 서버 포함, 클라이언트에 공격 명령 전달을 위한 멀티캐스트 RPC.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCAttack();

	// 클라이언트에서 공격 판정을 했을 때 충돌한 경우 실행.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyHit(
		const FHitResult& HitResult, float HitCheckTime
	);

	// 클라이언트에서 공격 판정을 했을 때 충돌하지 않은 경우 실행.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyMiss(
		FVector TraceStart, 
		FVector TraceEnd,
		FVector TraceDir,
		float HitCheckTime
	);


	UFUNCTION()
	void OnRep_CanAttack();

	// 공격 중인지 여부를 나타내는 플래그 (부울).
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	// 애니메이션 재생 길이 값 ( 타이머에 시간 값으로 활용 ).
	float AttackTime = 1.4667f;

	// 이전에 공격한 시간을 기록하는 변수.
	float LastAttackStartTime = 0.0f;

	// 클라이언트와 서버의 시간 차이를 기록하기 위한 변수.
	float AttackTimeDifference = 0.0f;

// UI Section
protected:
	virtual void SetupHUDWidget(class UABHUDWidget* InHUDWidget) override;
};
