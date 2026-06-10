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

void UABCharacterMovementComponent::OnMovementUpdated(
	float DeltaSeconds,
	const FVector& OldLocation,
	const FVector& OldVelocity)
{
	//Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	// 조건을 확인해서 텔레포트 수행.
	if (bPressedTeleport && !bDidTeleport)
	{
		ABTeleport();
	}

	// 위에서 입력 처리를 했기 때문에 입력 상태 초기화.
	if (bPressedTeleport)
	{
		bPressedTeleport = false;
	}
}

FNetworkPredictionData_Client* UABCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UABCharacterMovementComponent* MutableThis 
			= const_cast<UABCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FABNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

void UABCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// 클라이언트에서 인코딩한 값을 디코딩.
	bPressedTeleport = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bDidTeleport = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	// 조건 확인 후 텔레포트를 수행할지 결정.
	if (CharacterOwner && CharacterOwner->HasAuthority())
	{
		// 텔레포트가 수행이 안된 상태라면 서버에서 수행.
		if (bPressedTeleport && !bDidTeleport)
		{
			AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));

			ABTeleport();
		}
	}
}

FABNetworkPredictionData_Client_Character::FABNetworkPredictionData_Client_Character(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FABNetworkPredictionData_Client_Character::AllocateNewMove()
{
	// 새로운 타입으로 데이터 바꾸기.
	return FSavedMovePtr(new FABSavedMove_Character());
}

void FABSavedMove_Character::Clear()
{
	Super::Clear();

	// 프로퍼티 초기화.
	bPressedTeleport = false;
	bDidTeleport = false;
}

void FABSavedMove_Character::SetInitialPosition(ACharacter * Character)
{
	Super::SetInitialPosition(Character);

	// 초기 위치 설정. 캐릭터 무브먼트 컴포넌트를 활용.
	UABCharacterMovementComponent* ABMovement
		= Cast<UABCharacterMovementComponent>(Character->GetCharacterMovement());
	if (ABMovement)
	{
		// 텔레포트 관련 상태 값을 컴포넌트에서 읽어서 설정.
		bPressedTeleport = ABMovement->bPressedTeleport;
		bDidTeleport = ABMovement->bDidTeleport;

		// 이렇게 설정한 값은 RPC를 통해서 서버로 전달해야함.
	}
}

uint8 FABSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	// 전달할 값을 Result에 추가.
	if (bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	if (bDidTeleport)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}
