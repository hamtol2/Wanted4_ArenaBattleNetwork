// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterMovementComponent.generated.h"

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
