#pragma once

#include "CoreMinimal.h"
#include "Factory/Buildings/FactoryBuilding.h"
#include "FactoryMachine.generated.h"

UCLASS()
class FACTORYEXPERIMENT_API AFactoryMachine : public AFactoryBuilding
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName DefaultRecipeId;
};