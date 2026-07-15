#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Factory/Debug/FactoryDebugTypes.h"
#include "Factory/Grid/FactoryGridTypes.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "Grid/GridCoord.h"
#include "FactoryDeveloperModeWidget.generated.h"

class UFactoryBuildingDataAsset;

UCLASS()
class FACTORYEXPERIMENT_API UFactoryDeveloperModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void SetCurrentCellCoord(FGridCoord Coord, EFactoryCoordType CoordType, EFactoryResourceType ResourceType);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void ClearCurrentCellCoord();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void UpdateBuildingOnCell(FFactoryGridCell Cell, FName BuildingTypeId);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void ClearBuildingOnCell();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void UpdateSelectedBuildable(UFactoryBuildingDataAsset* BuildingData, FName BuildingTypeId, EFactoryDirection Direction);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void ClearSelectedBuildable();
};
