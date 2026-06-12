// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode 
	: public AGameModeBase, 
	public IABGameInterface
{
	GENERATED_BODY()
	
public:
	AABGameMode();

	//virtual void OnPlayerDead() override;


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

	//virtual void PreLogin(
	//	const FString& Options, 
	//	const FString& Address, 
	//	const FUniqueNetIdRepl& UniqueId, 
	//	FString& ErrorMessage) override;
	//
	//virtual void PostLogin(APlayerController* NewPlayer) override;
	//
	//virtual void StartPlay() override;
};
