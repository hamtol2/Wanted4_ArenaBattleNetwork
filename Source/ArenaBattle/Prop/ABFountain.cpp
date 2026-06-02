// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

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
	NetUpdateFrequency = 1.0f;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();

	//// 서버 로직.
	//if (HasAuthority())
	//{
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(
	//		Handle,
	//		FTimerDelegate::CreateLambda(
	//			[&]() {
	//				//ServerRotationYaw = 10.0f;
	//				ServerRotationYaw += 1.0f;
	//			}
	//		), 1.0f, true
	//	);
	//}
}

void AABFountain::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 리플리케이션(복제)할 속성을 매크로를 통해서 지정.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch,
	UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
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
