#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Factory/Debug/FactoryDebugTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryDeveloperModeWidget.generated.h"

UCLASS()
class FACTORYEXPERIMENT_API UFactoryDeveloperModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Debug|UI")
	void SetCurrentCellCoord(FGridCoord Coord, EFactoryCoordType CoordType);
};
