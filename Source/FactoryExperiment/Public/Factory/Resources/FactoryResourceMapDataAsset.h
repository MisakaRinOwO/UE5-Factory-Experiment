#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Grid/GridCoord.h"
#include "FactoryResourceMapDataAsset.generated.h"

UENUM(BlueprintType)
enum class EFactoryResourceType : uint8
{
	None UMETA(DisplayName = "None"),
	Iron UMETA(DisplayName = "Iron"),
	Copper UMETA(DisplayName = "Copper"),
	IronIngot UMETA(DisplayName = "Iron Ingot"),
	IronPlate UMETA(DisplayName = "Iron Plate"),
	IronRod UMETA(DisplayName = "Iron Rod"),
	IronBolt UMETA(DisplayName = "Iron Bolt"),
	CopperIngot UMETA(DisplayName = "Copper Ingot"),
	CopperWire UMETA(DisplayName = "Copper Wire")
};

USTRUCT(BlueprintType)
struct FFactoryResourceCoordList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TArray<FGridCoord> Coords;
};

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryResourceMapDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TMap<EFactoryResourceType, FFactoryResourceCoordList> ResourceCoordsByType;
};
