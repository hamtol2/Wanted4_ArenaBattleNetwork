// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ABGameInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UABGameInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class ARENABATTLE_API IABGameInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 리스폰을 할 때 생성 위치/회전 값을 반환하기 위한 함수.
	virtual FTransform GetRandomStartTransform() const = 0;

	// 누가 누구를 죽였는지에 대한 정보를 받을 수 있도록 함수 선언.
	virtual void OnPlayerKilled(
		AController* Killer, AController* KilledPlayer, APawn* KilledPawn) = 0;

	//virtual void OnPlayerDead() = 0;
};
