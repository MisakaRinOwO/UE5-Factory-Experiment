#pragma once

#include "CoreMinimal.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Factory/Items/FactoryItemTypes.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "Grid/GridCoord.h"
#include "FactoryMachineTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryMachineRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BuildingId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord OriginCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryPlacedBuildingPort> WorldPorts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipeId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CraftProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWorking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOutputBlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryItemStack> InputInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryItemStack> OutputInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsExtractor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryResourceType ExtractedResourceType = EFactoryResourceType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 OutputRoundRobinIndex = 0;
};
