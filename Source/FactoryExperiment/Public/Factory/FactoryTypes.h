#pragma once

#include "CoreMinimal.h"
#include "FactoryTypes.generated.h"

UENUM(BlueprintType)
enum class EFactoryCellOccupancy : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Conveyor UMETA(DisplayName = "Conveyor"),
	Building UMETA(DisplayName = "Building"),
	Blocked UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EFactoryDirection : uint8
{
	None UMETA(DisplayName = "None"),
	Up UMETA(DisplayName = "Up"),
	Right UMETA(DisplayName = "Right"),
	Down UMETA(DisplayName = "Down"),
	Left UMETA(DisplayName = "Left")
};

UENUM(BlueprintType)
enum class EFactoryPortType : uint8
{
	Input UMETA(DisplayName = "Input"),
	Output UMETA(DisplayName = "Output")
};