#pragma once

#include "CoreMinimal.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "Grid/GridCoord.h"
#include "FactoryMachineTypes.generated.h"

class UFactoryRecipeDataAsset;

UENUM(BlueprintType)
enum class EFactoryMachineStorageType : uint8
{
	Internal UMETA(DisplayName = "Internal"),
	InputPort UMETA(DisplayName = "Input Port"),
	OutputPort UMETA(DisplayName = "Output Port")
};

USTRUCT(BlueprintType)
struct FFactoryMachinePortStorage
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FFactoryPlacedBuildingPort Port;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<EFactoryResourceType, int32> StoredResources;
};

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
	TObjectPtr<UFactoryRecipeDataAsset> RecipeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CraftProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWorking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOutputBlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryMachinePortStorage> InputStorageByPort;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryMachinePortStorage> OutputStorageByPort;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<EFactoryResourceType, int32> InternalStorage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsExtractor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryResourceType ExtractedResourceType = EFactoryResourceType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ExtractionProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ExtractionRatePerSecond = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 OutputRoundRobinIndex = 0;
};
