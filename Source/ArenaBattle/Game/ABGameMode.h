// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode 
	: public AGameMode, 
	public IABGameInterface
{
	GENERATED_BODY()
	
public:
	AABGameMode();

	// IABGameInterface을(를) 통해 상속됨
	virtual FTransform GetRandomStartTransform() const override;

	virtual void OnPlayerKilled(
		AController* Killer, 
		AController* KilledPlayer, 
		APawn* KilledPawn) override;

	// 게임이 시작되면 호출되는 함수.
	virtual void StartPlay() override;

protected:
	// 시작 위치로 사용할 PlayerStart 액터 배열.
	TArray<TObjectPtr<class APlayerStart>> PlayerStartArray;

	// 컴포넌트 초기화가 끝나면 호출되는 이벤트 함수.
	virtual void PostInitializeComponents() override;

	// 타이머로 사용할 함수.
	virtual void DefaultGameTimer();

	// 경기가 종료되면 호출할 함수.
	virtual void FinishMatch();

	// 타이머 핸들.
	FTimerHandle GameTimerHandle;
};
