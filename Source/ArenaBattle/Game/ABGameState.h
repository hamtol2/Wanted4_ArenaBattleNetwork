// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ABGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameState : public AGameState
{
	GENERATED_BODY()

	//// GameMode의 StartPlay 함수에서 호출하는 함수.
	//virtual void HandleBeginPlay() override;
	//
	//// bReplicatedHasBegunPlay 값이 변경되면 
	//// 서버/클라이언트 모두에서 실행되는 함수.
	//virtual void OnRep_ReplicatedHasBegunPlay() override;

public:
	AABGameState();

	virtual void GetLifetimeReplicatedProps(
		TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	// 게임 진행에 남은 시간.
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	// 플레이 시간 (단위: 초).
	int32 MatchPlayTime = 20;

	// 게임이 종료되었을 때 잠시 대기할 시간(단위: 초).
	int32 ShowResultWaitingTime = 5;
};
