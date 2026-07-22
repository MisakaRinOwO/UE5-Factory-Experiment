#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryBuildingDataAsset.generated.h"

class AFactoryBuilding;
class UStaticMesh;
class UFactoryRecipeDataAsset;

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryBuildingDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	FName BuildingTypeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	EFactoryBuildableType BuildableType = EFactoryBuildableType::Machine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory", meta = (EditCondition = "!bIsConveyor"))
	TSubclassOf<AFactoryBuilding> BuildingActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	FIntPoint FootprintSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory|Visuals")
	TObjectPtr<UStaticMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	TArray<FFactoryBuildingPort> Ports;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	bool bIsConveyor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conveyor", meta = (EditCondition = "bIsConveyor"))
	TObjectPtr<UStaticMesh> ConveyorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conveyor", meta = (EditCondition = "bIsConveyor"))
	float ConveyorMeshYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conveyor", meta = (EditCondition = "bIsConveyor"))
	float ConveyorSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Machine", meta = (EditCondition = "!bIsConveyor"))
	TObjectPtr<UFactoryRecipeDataAsset> RecipeData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Machine|Extractor", meta = (EditCondition = "!bIsConveyor", ClampMin = "0.0"))
	float ExtractionRatePerSecond = 1.0f;
};
