// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABFountain.generated.h"

UCLASS()
class ARENABATTLE_API AABFountain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABFountain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//// 액터 채널이 열릴 때 호출되는 함수.
	//virtual void OnActorChannelOpen(
	//	class FInBunch& InBunch, 
	//	class UNetConnection* Connection) override;

	//// 연관성 검사 함수.
	//virtual bool IsNetRelevantFor(
	//	const AActor* RealViewer, 
	//	const AActor* ViewTarget, 
	//	const FVector& SrcLocation) const override;

	//// 리플리케이션이 처리되기 바로 직전에 호출되는 함수
	//// 서버에서만 호출됨.
	//virtual void PreReplication(
	//	IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	// 속성이 변경됐을 때 호출될 함수 지정.
	UFUNCTION()
	void OnRep_ServerRotationYaw();

	UFUNCTION()
	void OnRep_ServerLightColor();

	// 멀티캐스트 RPC 함수.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeLightColor(
		const FLinearColor& NewLightColor);

	// 서버 RPC 함수.
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRPCChangeLightColor();

	// 클라이언트 RPC 함수.
	UFUNCTION(Client, Unreliable)
	void ClientRPCChangeLightColor(
		const FLinearColor& NewLightColor);

	// 리플리케이션 옵션을 지정한 속성 추가.
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	// 회전 속도 변수.
	float RotationRate = 30.0f;

	// 서버로부터 패킷을 받은 후에 경과한 시간을 계산하기 위한 변수.
	float ClientTimeSinceUpdate = 0.0f;

	// 서버로부터 데이터를 받고 그 다음 데이터를 받았을 때까지 걸린 시간을 
	// 기록할 변수.
	float ClientTimeBetweenLastUpdate = 0.0f;

	// 의도적으로 네트워크 포화상태를 만들기 위한 변수.
	//UPROPERTY(Replicated)
	//TArray<float> BigData;
	//
	//// 값 변경에 사용할 변수.
	//float BigDataElement = 0.0f;
	
	// 라이트 색상 속성. ( 휴면 상태 테스트 )
	UPROPERTY(ReplicatedUsing = OnRep_ServerLightColor)
	FLinearColor ServerLightColor;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Water;

};
