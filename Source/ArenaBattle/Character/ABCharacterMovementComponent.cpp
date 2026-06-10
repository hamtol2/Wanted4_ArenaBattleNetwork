// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterMovementComponent.h"
#include "ArenaBattle.h"
#include "GameFramework/Character.h"

UABCharacterMovementComponent::UABCharacterMovementComponent()
{
	// 기본값 설정.
	bPressedTeleport = false;
	bDidTeleport = false;

	// 6미터 텔레포트 거리.
	TeleportOffset = 600.0f;

	// 쿨타임 3초.
	TeleportCooltime = 3.0f;
}

void UABCharacterMovementComponent::SetTeleportCommand()
{
	// 텔레포트 입력이 들어왔음을 표시.
	bPressedTeleport = true;
}

void UABCharacterMovementComponent::ABTeleport()
{
	// 캐릭터(오너)의 기능을 활용할 예정.
	if (CharacterOwner)
	{
		AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));

		// 텔레포트 목적지 계산.
		// 캐릭터의 현재 위치 + 캐릭터의 앞방향으로 6미터 위치를 계산.
		// 동차좌표계 -> 아핀변환.
		FVector TargetLocation
			= CharacterOwner->GetActorLocation()
			+ CharacterOwner->GetActorForwardVector() * TeleportOffset;

		// 언리얼이 기본으로 제공하는 기능을 활용.
		CharacterOwner->TeleportTo(
			TargetLocation,
			CharacterOwner->GetActorRotation(),
			false,
			false
		);

		// 텔레포트 수행한 후에 상태 변경.
		bDidTeleport = true;

		// 쿨타임 (타이머 활용).
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda(
				[this]()
				{
					// 텔레포트가 끝났음을 표시.
					bDidTeleport = false;

					// 로그 출력.
					AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport End"));
				}
			), TeleportCooltime, false
		);
	}
}
