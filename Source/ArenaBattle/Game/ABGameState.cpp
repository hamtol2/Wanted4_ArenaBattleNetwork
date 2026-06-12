// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameState.h"
#include "Net/UnrealNetwork.h"

//void AABGameState::HandleBeginPlay()
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::HandleBeginPlay();
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//void AABGameState::OnRep_ReplicatedHasBegunPlay()
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::OnRep_ReplicatedHasBegunPlay();
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}

AABGameState::AABGameState()
{
	// 시작될 때는 게임 시간을 초기화.
	RemainingTime = MatchPlayTime;
}

void AABGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABGameState, RemainingTime);
}
