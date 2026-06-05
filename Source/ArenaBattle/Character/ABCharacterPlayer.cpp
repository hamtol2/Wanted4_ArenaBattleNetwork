// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABCharacterControlData.h"
#include "UI/ABHUDWidget.h"
#include "CharacterStat/ABCharacterStatComponent.h"
#include "Interface/ABGameInterface.h"

#include "ArenaBattle.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Physics/ABCollision.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

AABCharacterPlayer::AABCharacterPlayer()
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeActionControlRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl'"));
	if (nullptr != InputChangeActionControlRef.Object)
	{
		ChangeControlAction = InputChangeActionControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove'"));
	if (nullptr != InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook'"));
	if (nullptr != InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove'"));
	if (nullptr != InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;

	// 시작할 때는 공격 가능한 상태로 설정.
	bCanAttack = true;

	// 액터 리플리케이션 활성화.
	bReplicates = true;
}

void AABCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}

	SetCharacterControl(CurrentCharacterControlType);
}

void AABCharacterPlayer::SetDead()
{
	Super::SetDead();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void AABCharacterPlayer::PossessedBy(AController* NewController)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// PossessedBy 함수가 호출되기 전에 액터의 소유 확인.
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		// 소유 정보가 있다면, 소유자의 이름 출력.
		AB_LOG(LogABNetwork, Log, TEXT("Onwer: %s"), 
			*OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	Super::PossessedBy(NewController);

	// PossessedBy 함수가 호출된 후에 액터의 소유 확인.
	OwnerActor = GetOwner();
	if (OwnerActor)
	{
		// 소유 정보가 있다면, 소유자의 이름 출력.
		AB_LOG(LogABNetwork, Log, TEXT("Onwer: %s"),
			*OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::SetupPlayerInputComponent(
	class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(ChangeControlAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ChangeCharacterControl);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(
		AttackAction, 
		ETriggerEvent::Triggered, 
		this, 
		&AABCharacterPlayer::Attack
	);
}

void AABCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		SetCharacterControl(ECharacterControlType::Shoulder);
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		SetCharacterControl(ECharacterControlType::Quater);
	}
}

void AABCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UABCharacterControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void AABCharacterPlayer::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = CharacterControlData->bDoCollisionTest;
}

void AABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	float InputSizeSquared = MovementVector.SquaredLength();
	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();
	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVector.Normalize();
		MovementVectorSizeSquared = 1.0f;
	}
	else
	{
		MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorSize);
}

void AABCharacterPlayer::Attack()
{
	//ProcessComboCommand();

	// 공격이 가능한 상태인지 확인.
	if (bCanAttack)
	{
		// 공격 입력이 들어오면 ServerRPC 호출.
		ServerRPCAttack();
	}
}

void AABCharacterPlayer::AttackHitCheck()
{
	// 공격 판정은 중요한 로직이기 때문에 서버에서 처리.
	if (HasAuthority())
	{
		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const float AttackDamage = Stat->GetTotalStat().Attack;
		const FVector Start
			= GetActorLocation()
			+ GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(
			OutHitResult,
			Start,
			End,
			FQuat::Identity,
			CCHANNEL_ABACTION,
			FCollisionShape::MakeSphere(AttackRadius),
			Params
		);

		if (HitDetected)
		{
			FDamageEvent DamageEvent;
			OutHitResult.GetActor()->TakeDamage(
				AttackDamage,
				DamageEvent,
				GetController(),
				this
			);
		}

#if ENABLE_DRAW_DEBUG

		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
		float CapsuleHalfHeight = AttackRange * 0.5f;
		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

		DrawDebugCapsule(
			GetWorld(),
			CapsuleOrigin,
			CapsuleHalfHeight,
			AttackRadius,
			FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(),
			DrawColor,
			false,
			5.0f
		);

#endif
	}
}

// OnRep_ 함수는 클라이언트에서만 호출.
// 서버에서 전달 받은 값과 자신(클라이언트)이 가진 값이 다를 때 호출됨.
void AABCharacterPlayer::OnRep_CanAttack()
{
	// 공격 중 상태.
	if (!bCanAttack)
	{
		// 공격 중일 때는 이동 모드를 None으로 설정.
		GetCharacterMovement()->SetMovementMode(
			EMovementMode::MOVE_None
		);
	}
	// 공격 중이 아닐 때.
	else
	{
		// 공격이 아닐 때는 이동 모드를 Walking으로 설정.
		GetCharacterMovement()->SetMovementMode(
			EMovementMode::MOVE_Walking
		);
	}
}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	// 클라에서 Attack 실행 -> ServerRPCAttack -> MulticastRPC -> 
	// 클라: bCanAttack ? true.
	// 서버: bCanAttack -> false로 변경.
	// 리플리케이션 틱 스케줄에 의해 bCanAttack이 클라로 전달.
	// 클라에서 bCanAttack이 false인 값을 받음. 이때 이전에 true였는데 false로 변경됨.
	// OnRep_

	// 서버 로직.
	if (HasAuthority())
	{
		// 공격이 시작됐으면, 재차 공격 입력이 되지 않도록 막기.
		bCanAttack = false;

		// 무브먼트 모드 none으로 설정 ( 공격할 때는 이동하지 않도록 ).
		//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		OnRep_CanAttack();

		// 애니메이션 재생 시간 만큼 타이머 설정.
		// 공격 종료 타이밍을 확인하기 위한 타이머
		// (기존에는 몽타주의 델리게이트를 사용했음).
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda(
				[this]() {
					// 공격 종료 로직.
					bCanAttack = true;

					// 무브먼트 모드 되돌리기.
					//GetCharacterMovement()->SetMovementMode(
					//	EMovementMode::MOVE_Walking
					//);

					OnRep_CanAttack();
				}
			), AttackTime, false
		);
	}

	// 공격 애님 몽타주 재생.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(ComboActionMontage);
	}
}

bool AABCharacterPlayer::ServerRPCAttack_Validate()
{
	return true;
}

void AABCharacterPlayer::ServerRPCAttack_Implementation()
{
	// 멀티캐스트 RPC 호출.
	MulticastRPCAttack();
}

void AABCharacterPlayer::SetupHUDWidget(UABHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateStat(Stat->GetBaseStat(), Stat->GetModifierStat());
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}
