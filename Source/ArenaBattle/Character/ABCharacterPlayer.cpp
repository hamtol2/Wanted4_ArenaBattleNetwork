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

#include "GameFramework/GameStateBase.h"

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
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

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
		// 클라이언트 로직.
		if (!HasAuthority())
		{
			// 공격 시작되면 공격 불가하도록 설정.
			bCanAttack = false;

			// 공격 중에는 이동하지 못하도록 설정.
			GetCharacterMovement()->SetMovementMode(
				EMovementMode::MOVE_None
			);

			// 타이머 설정 (공격 종료 처리).
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(
				Handle,
				FTimerDelegate::CreateLambda(
					[&]()
					{
						// 공격 종료 처리.
						bCanAttack = true;

						// 공격이 끝나면 다시 이동 가능하도록.
						GetCharacterMovement()->SetMovementMode(
							EMovementMode::MOVE_Walking
						);
					}
				), AttackTime, false
			);

			// 애니메이션 재생.
			PlayAttackAnimation();

		}

		// 공격 입력이 들어오면 ServerRPC 호출.
		// 클라이언트의 시간을 보냄 -> 때에 따라서 잘못될 수 있음.
		//float StartTime = GetWorld()->GetTimeSeconds();

		// 시간을 보낼 때 서버의 시간 값을 보냄.
		ServerRPCAttack(
			GetWorld()->GetGameState()->GetServerWorldTimeSeconds()
		);
	}
}

void AABCharacterPlayer::PlayAttackAnimation()
{
	// 메시 컴포넌트로부터 애님 인스턴스를 가져온 다음,
	// 몽타주 애셋 재생.
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// 재생 중인 몽타주가 있으면 종료.
		AnimInstance->StopAllMontages(0.0f);

		// 몽타주 재생.
		AnimInstance->Montage_Play(ComboActionMontage);
	}
}

void AABCharacterPlayer::AttackHitCheck()
{
	// 공격 판정은 중요한 로직이기 때문에 서버에서 처리.
	//if (HasAuthority())

	// 기존에 서버에서 판정하던 내용을
	// 소유 클라이언트에서 처리하도록 변경.
	if (IsLocallyControlled())
	{
		// 로그 출력.
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const float AttackDamage = Stat->GetTotalStat().Attack;

		const FVector Forward = GetActorForwardVector();
		const FVector Start
			= GetActorLocation()
			+ Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + Forward * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(
			OutHitResult,
			Start,
			End,
			FQuat::Identity,
			CCHANNEL_ABACTION,
			FCollisionShape::MakeSphere(AttackRadius),
			Params
		);

		// 서버 기준 현재 시간.
		float HitCheckTime
			= GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		// 클라이언트.
		if (!HasAuthority())
		{
			// 클라이언트에서 판정해봤더니 부딪힌 경우.
			if (HitDetected)
			{
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
				//FDamageEvent DamageEvent;
				//OutHitResult.GetActor()->TakeDamage(
				//	AttackDamage,
				//	DamageEvent,
				//	GetController(),
				//	this
				//);
			}
			// 클라이언트에서 판정해봤더니 부딪히지 않은 경우.
			else
			{
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}
		}
		// 서버.
		else
		{
			// 디버그 드로우로 충돌 정보 보여주기.
			FColor DebugColor = HitDetected ? FColor::Green : FColor::Red;
			DrawDebugAttackRange(DebugColor, Start, End, Forward);

			// 서버에서는 추가로 판단하지 않고, 공격 판정 수락.
			if (HitDetected)
			{
				AttackHitConfirm(OutHitResult.GetActor());
			}
		}

		//#if ENABLE_DRAW_DEBUG
		//
		//		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
		//		float CapsuleHalfHeight = AttackRange * 0.5f;
		//		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;
		//
		//		DrawDebugCapsule(
		//			GetWorld(),
		//			CapsuleOrigin,
		//			CapsuleHalfHeight,
		//			AttackRadius,
		//			FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(),
		//			DrawColor,
		//			false,
		//			5.0f
		//		);
		//
		//#endif
	}
}

void AABCharacterPlayer::AttackHitConfirm(AActor* HitActor)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 서버에서만 실행해야함.
	if (HasAuthority())
	{
		// 공격 대미지를 스탯 컴포넌트에서 가져오기.
		const float AttackDamage = Stat->GetTotalStat().Attack;

		// 전달할 대미지 이벤트 변수.
		FDamageEvent DamageEvent;

		// 맞은 액터에 대미지 처리 진행.
		HitActor->TakeDamage(
			AttackDamage,
			DamageEvent,
			GetController(),
			this
		);
	}
}

void AABCharacterPlayer::DrawDebugAttackRange(
	const FColor& DrawColor,
	FVector TraceStart,
	FVector TraceEnd,
	FVector Forward)
{
#if ENABLE_DRAW_DEBUG

	const float AttackRange = Stat->GetTotalStat().AttackRange;
	const float AttackRadius = Stat->GetAttackRadius();

	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	//FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

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

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(
	const FHitResult& HitResult, float HitCheckTime)
{
	// 충돌 정보로부터 액터 가져오기.
	AActor* HitActor = HitResult.GetActor();
	if (IsValid(HitActor))
	{
		// 클라이언트로부터 받은 정보를 기반으로 처리는 하되,
		// 검증은 진행.
		// 거리 기반으로 검증.

		// 맞은 위치.
		const FVector HitLocation = HitResult.Location;

		// 맞은 액터의 범위 가져오기.
		// 캐릭터를 감싸는 박스 정보 가져오기.
		// 캐릭터의 위치를 사용해도 됨.
		const FBox HitBox = HitActor->GetComponentsBoundingBox();

		// 바운딩 박스의 중심 위치.
		const FVector ActorBoxCenter = HitBox.GetCenter();

		// 거리 확인.
		if (FVector::DistSquared(HitLocation, ActorBoxCenter)
			<= AcceptCheckDistance * AcceptCheckDistance)
		{
			// 인정 -> 대미지 처리.
			AttackHitConfirm(HitActor);
		}
		else
		{
			// 인정 안함.
			AB_LOG(LogABNetwork, Warning, TEXT("%s"), TEXT("Hit Rejected!"));
		}

		// 결과를 볼 수 있도록 디버그 드로우로 그려주기.
#if ENABLE_DRAW_DEBUG

		// 맞은 액터의 위치를 점으로 표시.
		DrawDebugPoint(
			GetWorld(),
			ActorBoxCenter,
			50.0f,
			FColor::Cyan,		// 청록색.
			false,
			5.0f
		);

		// 맞은 위치를 점으로 표시.
		DrawDebugPoint(
			GetWorld(),
			HitLocation,
			50.0f,
			FColor::Magenta,	// 다홍색.
			false,
			5.0f
		);
#endif
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(
	const FHitResult& HitResult, float HitCheckTime)
{
	// 공격 타이밍으로 검증.

	// 첫 공격인 경우에는 수락.
	//if (0.0f == LastAttackStartTime)
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// 이전에 공격한 시간 이후로,
	// 이번에 공격을 시도한 시간까지의 경과 시간이
	// 허용 가능한 시간보다 더 걸렸는지 판단.
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(
	FVector TraceStart,
	FVector TraceEnd,
	FVector TraceDir,
	float HitCheckTime)
{
}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(
	FVector TraceStart,
	FVector TraceEnd,
	FVector TraceDir,
	float HitCheckTime)
{
	// 공격 타이밍으로 검증.

	// 첫 공격인 경우에는 수락.
	//if (0.0f == LastAttackStartTime)
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// 이전에 공격한 시간 이후로,
	// 이번에 공격을 시도한 시간까지의 경과 시간이
	// 허용 가능한 시간보다 더 걸렸는지 판단.
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
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
	// 여기에서는 자신이 제어하는 클라 제외, 서버 제외,
	// Simulated 허상(Proxy)에서만 공격 애니메이션 재생.
	// 왜냐하면, 여기까지 오기 전에, 
	// 자신이 제어하는 클라에서는 이미 애니메이션 재생함.
	// 그리고 서버에서도 서버에 있는 캐릭터의 애니메이션 이미 재생함.
	// 이제 남은 건 서버, 해당 클라를 제외한 나머지에서 애니메이션 재생.
	if (!IsLocallyControlled())
	{
		PlayAttackAnimation();
	}
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	// LastAttackStartTime 값이 0인 경우는 공격을 한번도 안한 경우.
	// 즉, 첫 공격 시도일 때는 통과.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// 이전에 공격을 시작한 시간으로부터 충분한 시간이 지났는지 확인.
	return (AttackStartTime - LastAttackStartTime) > AttackTime;
}

// 이 함수는 서버에서 실행됨.
void AABCharacterPlayer::ServerRPCAttack_Implementation(
	float AttackStartTime)
{
	// 공격 시작 처리.
	bCanAttack = false;
	OnRep_CanAttack();

	// 서버에 전달된 시간 차를 확인.
	// 서버 기준 현재 시간에서 클라이언트가 전달한 시간을 뺌.
	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;

	// 시간 차가 어느정도인지 로그 출력.
	AB_LOG(LogABNetwork, Log, TEXT("LagTime: %f"), AttackTimeDifference);

	// 타이머 설정할 때 시간 값을 0또는 음수로 설정하면 동작 안함.
	// 타이머가 동작은 하도록 값 보정.
	AttackTimeDifference = FMath::Clamp(
		AttackTimeDifference, 0.0f, AttackTime - 0.01f
	);

	// 타이머 설정.
	// 공격 종료 시간을 계산할 때, 
	// 애니메이션 재생시간에서 클라에서 서버까지 메시지가 전달되는데까지
	// 걸린시간을 고려해서 설정.
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda(
			[&]()
			{
				// 공격 종료 처리.
				bCanAttack = true;
				OnRep_CanAttack();
			}
		), AttackTime - AttackTimeDifference, false
	);

	// 클라이언트가 공격을 시작한 시간을 저장(기록).
	LastAttackStartTime = AttackStartTime;

	// 서버에서도 애니메이션 재생.
	PlayAttackAnimation();

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
