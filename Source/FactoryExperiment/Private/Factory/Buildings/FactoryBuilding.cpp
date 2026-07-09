#include "Factory/Buildings/FactoryBuilding.h"

AFactoryBuilding::AFactoryBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryBuilding::InitializeBuilding(int32 InBuildingId, FGridCoord InOriginCoord)
{
	BuildingId = InBuildingId;
	OriginCoord = InOriginCoord;

	BP_OnBuildingInitialized();
}

void AFactoryBuilding::SetWorldPorts(const TArray<FFactoryPlacedBuildingPort>& InWorldPorts)
{
	WorldPorts = InWorldPorts;
}