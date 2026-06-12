// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "ABGameMode.h"
#include "Player/ABPlayerController.h"

#include "ArenaBattle.h"
#include "ABGameState.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AABGameMode::AABGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/Engine.Blueprint'/Game/ArenaBattle/Blueprint/BP_ABCharacterPlayer.BP_ABCharacterPlayer_C'"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ArenaBattle.ABPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}

	// 게임 스테이트 클래스 설정.
	GameStateClass = AABGameState::StaticClass();
}

FTransform AABGameMode::GetRandomStartTransform() const
{
	// PlayerStartArray 배열 정보가 설정되지 않았다면 기본 트랜스폼 반환.
	if (PlayerStartArray.Num() == 0)
	{
		return FTransform(FVector(0.0f, 0.0f, 230.0f));
	}

	// 배열 정보가 설정되었다면, 랜덤으로 위치 선택 후 반환.
	int32 RandIndex = FMath::RandRange(0, PlayerStartArray.Num() - 1);
	return PlayerStartArray[RandIndex]->GetActorTransform();
}

void AABGameMode::OnPlayerKilled(
	AController* Killer, 
	AController* KilledPlayer, 
	APawn* KilledPawn)
{

}

void AABGameMode::StartPlay()
{
	Super::StartPlay();

	// 월드에 있는 플레이어 스타트 액터를 배열에 저장.
	for (APlayerStart* PlayerStart 
		: TActorRange<APlayerStart>(GetWorld()))
	{
		// 배열에 추가.
		PlayerStartArray.Add(PlayerStart);
	}
}

//void AABGameMode::OnPlayerDead()
//{
//
//}

//void AABGameMode::PreLogin(
//	const FString& Options,
//	const FString& Address,
//	const FUniqueNetIdRepl& UniqueId,
//	FString& ErrorMessage)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT//("=========================="));
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
//
//	// ErrorMessage에 아무런 값을 입력하지 않으면 로그인을 통과시킴.
//	// ErrorMessage에 값이 입력되면 오류로 간주 -> 접속 차단.
//	//ErrorMessage = TEXT("Server is full");
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//void AABGameMode::PostLogin(APlayerController* NewPlayer)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::PostLogin(NewPlayer);
//
//	// 클라이언트 정보 출력.
//	UNetDriver* NetDriver = GetNetDriver();
//	if (NetDriver)
//	{
//		// 클라이언트의 접속이 없는 경우.
//		if (NetDriver->ClientConnections.Num() == 0)
//		{
//			AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Client //Connection"));
//		}
//		// 클라이언트의 접속이 있는 경우.
//		else
//		{
//			for (const auto& Connection : NetDriver->ClientConnections)
//			{
//				AB_LOG(LogABNetwork, Log, TEXT("Client Connection : %s"),
//					*Connection->GetName()
//				);
//			}
//		}
//	}
//	else
//	{
//		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No NetDriver"));
//	}
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//void AABGameMode::StartPlay()
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::StartPlay();
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
