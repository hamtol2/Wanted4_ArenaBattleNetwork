// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

#include "Components/PointLightComponent.h"

// Sets default values
AABFountain::AABFountain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	// 리플리케이션 활성화.
	bReplicates = true;

	// 네트워크 전송 빈도 설정 ( 1초에 1번으로 )
	//NetUpdateFrequency = 1.0f;
	SetNetUpdateFrequency(1.0f);

	// 거리 기반 연관성 판정에 사용할 거리 값 (제곱 값: 20미터).
	//NetCullDistanceSquared = 4000000.0f;
	SetNetCullDistanceSquared(4000000.0f);

	// 휴면 상태 설정.
	// 프로퍼티 리플리케이션의 경우에는 DORM_Initial 값만 사용 가능.
	//NetDormancy = DORM_Initial;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();

	// 서버 로직.
	if (HasAuthority())
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda(
				[&]() {

					// 큰 데이터 설정 ( 400 바이트 크기 ).
					//BigData.Init(BigDataElement, 1000);

					// 지속적인 전속을 위한 데이터 변경.
					//BigDataElement += 1.0f;

					// 색상 값 변경.
					//ServerLightColor = FLinearColor(
					//	FMath::RandRange(0.0f, 1.0f),
					//	FMath::RandRange(0.0f, 1.0f),
					//	FMath::RandRange(0.0f, 1.0f),
					//	1.0f
					//);

					// OnRep_ 함수는 서버에서 호출되지 않기 때문에 명시적으로 호출.
					//OnRep_ServerLightColor();

					//const FLinearColor NewLightColor = FLinearColor(
					//	FMath::RandRange(0.0f, 1.0f),
					//	FMath::RandRange(0.0f, 1.0f),
					//	FMath::RandRange(0.0f, 1.0f),
					//	1.0f
					//);

					//// 멀티캐스트 RPC 호출.
					//MulticastRPCChangeLightColor(NewLightColor);
				}
			), 1.0f, true
		);


		// 두 번째 타이머 - 휴면 상태를 깨우기 위함.
		FTimerHandle Handle2;
		GetWorld()->GetTimerManager().SetTimer(
			Handle2,
			FTimerDelegate::CreateLambda([&]()
				{
					// 10초 경과 후에 휴면 상태 깨우기.
					//FlushNetDormancy();

					// 플레이어 컨트롤러를 순회해서 클라이언트의
					// 플레이어 컨트롤러를 오너로 설정.
					for (auto Iterator
						= GetWorld()->GetPlayerControllerIterator();
						Iterator;
						++Iterator)
					{
						// 플레이어 컨트롤러 가져오기.
						APlayerController* PlayerController = Iterator->Get();

						// 클라이언트의 PC인지 확인.
						// 서버 입장에서 IsLocalPlayerController()가 true라면,
						// 리슨 서버에 있는 플레이어 컨트롤러이기 때문에
						// 이 값이 false여야 클라이언트라는 게 확인됨.
						if (PlayerController
							&& !PlayerController->IsLocalPlayerController())
						{
							// 첫 번째 플레이어 컨트롤러를 오너로 설정.
							SetOwner(PlayerController);
							break;
						}
					}

				}
			), 10.0f, false
		);

	}

	// 클라이언트 로직.
	else
	{
		// 오너십 설정.
		// 이 분수대 액터의 소유권을 클라이언트로 설정.
		//SetOwner(GetWorld()->GetFirstPlayerController());

		// 타이머 활용해서 서버 RPC 호출.
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda(
				[&]()
				{
					// 서버 RPC 호출.
					ServerRPCChangeLightColor();
				}
			), 1.0f, true
		);
	}
}

void AABFountain::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 리플리케이션(복제)할 속성을 매크로를 통해서 지정.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);

	// 데이터 전송 테스트를 위한 변수를 리플리케이션에 등록.
	//DOREPLIFETIME(AABFountain, BigData);
	DOREPLIFETIME(AABFountain, ServerLightColor);
	//DOREPLIFETIME_CONDITION(AABFountain, ServerLightColor, COND_InitialOnly);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch,
	UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

bool AABFountain::IsNetRelevantFor(
	const AActor* RealViewer,
	const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	bool NetRelevantResult
		= Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	// 연관성이 없다고 판단된 경우에는 뷰어의 위치 출력.
	if (!NetRelevantResult)
	{
		AB_LOG(
			LogABNetwork,
			Log,
			TEXT("Not Relevant: [%s] %s"),
			*RealViewer->GetName(),
			*SrcLocation.ToString()
		);
	}

	return NetRelevantResult;
}

void AABFountain::PreReplication(
	IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PreReplication(ChangedPropertyTracker);
}

void AABFountain::OnRep_ServerRotationYaw()
{
	AB_LOG(LogABNetwork, Log, TEXT("Yaw: %f"), ServerRotationYaw);

	// 서버에서 전달 받은 회전 값을 설정할 회전 값 생성.
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;

	// 회전 값 설정.
	RootComponent->SetWorldRotation(NewRotator);

	// 이전 서버의 업데이트로부터 이번 업데이트까지 걸린 시간 저장.
	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;

	// 서버로부터 데이터를 받으면 0으로 초기화.
	ClientTimeSinceUpdate = 0.0f;
}

void AABFountain::OnRep_ServerLightColor()
{
	// 클라이언트인 경우 전달 받은 값 출력.
	if (!HasAuthority())
	{
		AB_LOG(
			LogABNetwork,
			Log,
			TEXT("ServerLightColor: %s"),
			*ServerLightColor.ToString()
		);
	}

	// 서버-클라이언트 모두에서 실행.
	// 컴포넌트 검색 - BP에서 추가한 컴포넌트 가져오기.
	UPointLightComponent* PointLight
		= GetComponentByClass<UPointLightComponent>();
	if (PointLight)
	{
		// 서버에서 전달 받은 색상을 라이트 색상으로 설정.
		PointLight->SetLightColor(ServerLightColor);
	}
}

void AABFountain::ServerRPCChangeLightColor_Implementation()
{
	// 서버 RPC를 클라이언트에서 호출하면,
	// 서버에서 이 함수가 호출됨. Client -> Server.
	// 이를 인지한 서버에서는 랜덤으로 색상을 만들고,
	// 자신을 포함해 모든 클라이언트에게 새로 만든 색상을 
	// 멀티캐스트 RPC를 통해 전달.
	const FLinearColor NewLightColor = FLinearColor(
		FMath::RandRange(0.0f, 1.0f),
		FMath::RandRange(0.0f, 1.0f),
		FMath::RandRange(0.0f, 1.0f),
		1.0f
	);

	// 멀티캐스트 RPC 호출.
	MulticastRPCChangeLightColor(NewLightColor);
}

void AABFountain::MulticastRPCChangeLightColor_Implementation(
	const FLinearColor& NewLightColor)
{
	AB_LOG(
		LogABNetwork,
		Log,
		TEXT("LightColor: %s"),
		*NewLightColor.ToString()
	);

	// 컴포넌트 검색 후 라이트 색상 설정.
	UPointLightComponent* PointLight
		= GetComponentByClass<UPointLightComponent>();
	if (PointLight)
	{
		PointLight->SetLightColor(NewLightColor);
	}
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 분수대 회전 처리.
	// 서버와 클라이언트 로직을 분리해서 작성.

	// 서버
	if (HasAuthority())
	{
		// 회전 적용.
		AddActorLocalRotation(
			FRotator(0.0f, RotationRate * DeltaTime, 0.0f)
		);

		// 변경된 회전 값을 프로퍼티에 저장.
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}

	// 클라이언트.
	else
	{
		// 서버로부터 데이터를 받은 후에 경과한 시간 계산.
		ClientTimeSinceUpdate += DeltaTime;

		// 보간처리.

		// 너무 작은 시간이 경과했을 때는 의미가 없음.
		if (ClientTimeSinceUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		// 다음 네트워크 패킷 전송 때 전달될 회전 값 예측.
		const float EstimateRotationYaw
			= ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;

		// 보간할 비율(alpha) 구하기.
		const float LerpRatio
			= ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		// 보간(Lerp).
		const float ClientNewYaw
			= FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);

		// 회전 값 설정 및 적용.
		FRotator ClientRotator = RootComponent->GetComponentRotation();
		ClientRotator.Yaw = ClientNewYaw;

		RootComponent->SetWorldRotation(ClientRotator);

		//// 서버에서 전달된 값 출력.
		//AB_LOG(
		//	LogABNetwork, 
		//	Log, 
		//	TEXT("ServerRotationYaw: %f"),
		//	ServerRotationYaw
		//);

		//// 서버에서 전달 받은 회전 값을 설정할 회전 값 생성.
		//FRotator NewRotator = RootComponent->GetComponentRotation();
		//NewRotator.Yaw = ServerRotationYaw;

		//// 회전 값 설정.
		//RootComponent->SetWorldRotation(NewRotator);
	}
}
