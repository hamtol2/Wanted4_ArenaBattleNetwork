// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// __FUNCTION__ -> c문자열로 함수 이름 값 전달해줌.
// 참고: __FILE__, __LINE__ 등등
// ANSI_TO_TCHAR c문자열을 언리얼의 TCHAR 타입의 문자열로 변환하는 함수.

// 로컬 롤/리모트 롤 각각을 출력할 수 있는 매크로 선언.
#define LOG_SUBLOCALROLEINFO \
	*(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetLocalRole()))

#define LOG_SUBREMOTEROLEINFO \
	*(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetRemoteRole()))

// 로컬 롤/리모트 롤 각각을 출력할 수 있는 매크로 선언.
#define LOG_LOCALROLEINFO \
	*(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))

#define LOG_REMOTEROLEINFO \
	*(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))

// 넷모드 정보 출력.
#define LOG_NETMODEINFO ( (GetNetMode() == ENetMode::NM_Client) ? \
	*FString::Printf(TEXT("CLIENT%d"), UE::GetPlayInEditorID()) : \
	( (GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : \
	TEXT("SERVER")))

// 함수 정보를 문자열로.
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

// 로그 매크로 선언.
#define AB_LOG(LogCat, Verbosity, Format, ...) \
	UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), \
		LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, \
		LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// 로그 매크로 선언.
#define AB_SUBLOG(LogCat, Verbosity, Format, ...) \
	UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), \
		LOG_NETMODEINFO, LOG_SUBLOCALROLEINFO, LOG_SUBREMOTEROLEINFO, \
		LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// 로그 카테고리 선언.
DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);