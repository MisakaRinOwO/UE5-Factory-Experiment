#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryBuildingSelectionItemWidget.generated.h"

class UFactoryBuildingDataAsset;
class UTexture2D;

UCLASS()
class FACTORYEXPERIMENT_API UFactoryBuildingSelectionItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Building Selection")
	void SetStyle(UTexture2D* ItemTexture, int32 KeyNumber, const FText& ItemName);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Building Selection")
	void ToggleHighlight();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Factory|Building Selection")
	void SetKeyNumber(int32 KeyNumber);
};
