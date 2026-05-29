// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// __FUNCTION__ -> c문자열로 함수 이름 값 전달해줌.
// 참고: __FILE__, __LINE__ 등등
// ANSI_TO_TCHAR c문자열을 언리얼의 TCHAR 타입의 문자열로 변환하는 함수.

// 넷모드 정보 출력.
#define LOG_NETMODEINFO ( (GetNetMode() == ENetMode::NM_Client) ? \
	*FString::Printf(TEXT("CLIENT%d"), UE::GetPlayInEditorID()) : \
	( (GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : \
	TEXT("SERVER")))

// 함수 정보를 문자열로.
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

// 로그 매크로 선언.
#define AB_LOG(LogCat, Verbosity, Format, ...) \
	UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), \
		LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// 로그 카테고리 선언.
DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);