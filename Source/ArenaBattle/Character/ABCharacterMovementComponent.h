// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterMovementComponent.generated.h"

// FNetworkPredictionData_Client 클래스를 관리할 별도의 클래스 선언.
class FABNetworkPredictionData_Client_Character
	: public FNetworkPredictionData_Client_Character
{
	// 편의 목적. 부모 클래스를 Super 키워드로 재정의.
	using Super = FNetworkPredictionData_Client_Character;

public:
	FABNetworkPredictionData_Client_Character(
		const UCharacterMovementComponent& ClientMovement
	);

	virtual FSavedMovePtr AllocateNewMove() override;
};

// 움직임 데이터를 변경하기 위한 클래스 선언.
class FABSavedMove_Character : public FSavedMove_Character
{
	// 편의 목적. 부모 클래스를 Super 키워드로 재정의.
	using Super = FSavedMove_Character;

public:
	virtual void Clear() override;
	virtual void SetInitialPosition(ACharacter* Character) override;
	virtual uint8 GetCompressedFlags() const override;

	// 상태 값.
	uint8 bPressedTeleport : 1;
	uint8 bDidTeleport : 1;
};

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UABCharacterMovementComponent();

	// 캐릭터에서 텔레포트 입력이 들어왔을 때 실행해줄 함수.
	void SetTeleportCommand();

protected:
	// 실제로 텔레포트를 수행하는 함수.
	virtual void ABTeleport();

	// PerformMovement 함수의 로직이 완료되면 끝 부분에서 호출되는 이벤트 함수.
	virtual void OnMovementUpdated(
		float DeltaSeconds, 
		const FVector& OldLocation, 
		const FVector& OldVelocity) override;

	// 클라이언트에서 서버로 텔레포트 명령을 보낼 때 사용.
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() 
		const override;

	// 클라이언트에서 압축해서 보낸 플래그를 서버에서 받아서 처리하는 함수.
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:
	// 텔레포트 상태를 확인할 부울 변수.
	// 텔레포트 입력이 들어왔는지 확인.
	uint8 bPressedTeleport : 1;

	// 쿨타임이 있기 때문에 텔레포트가 진행 중인지 파악할 부울 변수.
	uint8 bDidTeleport : 1;

protected:
	// 텔레포트 이동 거리 값.
	float TeleportOffset;

	// 텔레포트 쿨타임 시간.
	float TeleportCooltime;
};
