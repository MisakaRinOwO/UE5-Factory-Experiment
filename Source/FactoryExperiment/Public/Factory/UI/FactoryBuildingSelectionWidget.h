#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryBuildingSelectionWidget.generated.h"

class UFactoryBuildingDataAsset;
class UFactoryBuildingSelectionItemWidget;

UCLASS()
class FACTORYEXPERIMENT_API UFactoryBuildingSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Building Selection")
	void InitToolbar();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Building Selection")
	void ClearAllHighlight();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Building Selection")
	UFactoryBuildingSelectionItemWidget* GetItemByIndex(int32 ItemIndex);
};
