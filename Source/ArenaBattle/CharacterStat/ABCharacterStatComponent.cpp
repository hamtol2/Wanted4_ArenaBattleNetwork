// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStat/ABCharacterStatComponent.h"
#include "GameData/ABGameSingleton.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	CurrentLevel = 1;
	AttackRadius = 50.0f;

	bWantsInitializeComponent = true;

	// 리플리케이션 활성화.
	SetIsReplicated(true);
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetLevelStat(CurrentLevel);

	// MaxHp 설정.
	MaxHp = BaseStat.MaxHp;

	SetHp(MaxHp);

	// 스탯이 변경되면 발행되는 델리게이트에 함수 등록.
	OnStatChanged.AddUObject(
		this, 
		&UABCharacterStatComponent::SetNewMaxHp
	);
}

void UABCharacterStatComponent::BeginPlay()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::BeginPlay();
}

void UABCharacterStatComponent::ReadyForReplication()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::ReadyForReplication();
}

void UABCharacterStatComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UABCharacterStatComponent, CurrentHp);
	DOREPLIFETIME(UABCharacterStatComponent, MaxHp);
	
	DOREPLIFETIME_CONDITION(UABCharacterStatComponent, BaseStat, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UABCharacterStatComponent, ModifierStat, COND_OwnerOnly);
}

void UABCharacterStatComponent::SetNewMaxHp(
	const FABCharacterStat& InBaseStat, 
	const FABCharacterStat& InModifierStat)
{
	// MaxHp 설정 및 MaxHp가 변경되면 이벤트 발행.
	float PrevMaxHp = MaxHp;
	MaxHp = GetTotalStat().MaxHp;

	// 기존에 가진 값과 새로 설정한 값이 다르면 이벤트 발행.
	if (PrevMaxHp != MaxHp)
	{
		// HP 정보 변경 델리게이트 실행.
		OnHpChanged.Broadcast(CurrentHp, MaxHp);
	}
}

// OnRep_ 함수는 클라이언트에서만 호출.
// 서버로부터 값을 받았을 때 기존과 다를 때 호출됨 ( 콜백 ).
// 따라서 Hp가 변경되었을 때 필요한 로직 수행.
void UABCharacterStatComponent::OnRep_CurrentHp()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 이미 서버로부터 갱신된 CurrentHp 값을 받았기 때문에
	// 델리게이트를 통해서 전달.
	OnHpChanged.Broadcast(CurrentHp, MaxHp);

	// 죽음 판정.
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}
}

void UABCharacterStatComponent::OnRep_MaxHp()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 이미 서버로부터 갱신된 CurrentHp/MaxHp 값을 받았기 때문에
	// 델리게이트를 통해서 전달.
	OnHpChanged.Broadcast(CurrentHp, MaxHp);
}

void UABCharacterStatComponent::OnRep_BaseStat()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 스탯 변경 이벤트 발행.
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
}

void UABCharacterStatComponent::OnRep_ModifierStat()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 스탯 변경 이벤트 발행.
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
}

void UABCharacterStatComponent::SetLevelStat(int32 InNewLevel)
{
	CurrentLevel = FMath::Clamp(InNewLevel, 1, UABGameSingleton::Get().CharacterMaxLevel);
	SetBaseStat(UABGameSingleton::Get().GetCharacterStat(CurrentLevel));
	check(BaseStat.MaxHp > 0.0f);
}

float UABCharacterStatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	SetHp(PrevHp - ActualDamage);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}

	return ActualDamage;
}

void UABCharacterStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);
	
	OnHpChanged.Broadcast(CurrentHp, MaxHp);
}

