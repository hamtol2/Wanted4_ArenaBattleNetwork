// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "ABGameMode.h"
#include "Player/ABPlayerController.h"

#include "ArenaBattle.h"
#include "ABGameState.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "ABPlayerState.h"

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

	// 플레이어 스테이트 클래스 설정.
	PlayerStateClass = AABPlayerState::StaticClass();
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
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 점수 처리 진행.
	// 플레이어 스테이트가 기본 제공하는 점수 사용.
	APlayerState* KillerPlayerState = Killer->PlayerState;
	if (KillerPlayerState)
	{
		// Kill을 올린 플레이어에 1점 추가.
		KillerPlayerState->SetScore(KillerPlayerState->GetScore() + 1);

		// kill 수가 2보다 큰 플레이어가 있으면 게임 종료.
		if (KillerPlayerState->GetScore() > 2)
		{
			// 경기 종료 처리.
			// 5초를 더 대기 후 ServerTravel.
			FinishMatch();
		}
	}
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

void AABGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 타이머 설정 ( 게임 시간 계산 ).
	GetWorldTimerManager().SetTimer(
		GameTimerHandle,
		this,
		&AABGameMode::DefaultGameTimer,
		GetWorldSettings()->GetEffectiveTimeDilation(),
		true
	);
}

void AABGameMode::DefaultGameTimer()
{
	// 남은 시간 계산을 위해 게임 스테이트 가져오기.
	AABGameState* const ABGameState = Cast<AABGameState>(GameState);
	if (ABGameState)
	{
		// 게임의 남은 시간 1초 감소 처리 ( 카운트 다운 )
		ABGameState->RemainingTime -= 1;

		// 남은 시간 출력.
		AB_LOG(
			LogABNetwork, 
			Log, 
			TEXT("Remaining Time: %d"), 
			ABGameState->RemainingTime
		);

		// 시간이 모두 경과했는지 확인.
		if (ABGameState->RemainingTime <= 0)
		{
			// 매치 상태에 따라 처리.

			// 경기 시간이 모두 지났는데 경기가 진행 중이면, 
			// 경기 종료 처리.
			if (GetMatchState() == MatchState::InProgress)
			{
				// 게임 종료 함수 호출.
				FinishMatch();
			}
			// 다음 경기를 기다리고 있는 상태라면,
			// 서버 트래블을 사용해 재경기.
			else if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				// 클라이언트를 다른 맵으로 이동.
				// 이 때 로드할 맵 정보를 전달.
				// URL: 웹페이지 주소에서 많이 사용.
				// URL 구조: https://www.naver.com/?id=ronniej&password=1234
				GetWorld()->ServerTravel(
					TEXT("/Game/ArenaBattle/Maps/Part3Step2?Listen")
				);
			}
		}
	}
}

void AABGameMode::FinishMatch()
{
	// 이 함수는 경기를 종료시킬 때 호출.

	AABGameState* const ABGameState
		= Cast<AABGameState>(GameState);

	// 경기 상태 확인.
	// 경기가 진행 중이라면 경기 종료 처리.
	if (ABGameState && IsMatchInProgress())
	{
		// 게임 모드의 엔드 매치 호출.
		EndMatch();

		// 경기 종료 후 잠깐 대기를 위해 타이머 시간 재설정.
		ABGameState->RemainingTime = ABGameState->ShowResultWaitingTime;
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
