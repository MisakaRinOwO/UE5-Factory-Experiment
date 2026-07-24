#pragma once

#include "CoreMinimal.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "Grid/GridCoord.h"
#include "FactoryStorageTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryStorageSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryResourceType ResourceType = EFactoryResourceType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Count = 0;

	bool IsEmpty() const
	{
		return ResourceType == EFactoryResourceType::None || Count <= 0;
	}
};

USTRUCT(BlueprintType)
struct FFactoryStorageRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BuildingId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord OriginCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryPlacedBuildingPort> WorldPorts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AvailableSlots = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryStorageSlot> Slots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 OutputRoundRobinIndex = 0;
};
