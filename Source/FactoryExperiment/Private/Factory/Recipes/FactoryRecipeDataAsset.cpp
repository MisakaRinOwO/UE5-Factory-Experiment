#include "Factory/Recipes/FactoryRecipeDataAsset.h"

bool UFactoryRecipeDataAsset::IsAllowedForBuilding(const UFactoryBuildingDataAsset* BuildingData) const
{
	if (!BuildingData)
	{
		return false;
	}

	return AllowedBuildings.IsEmpty() || AllowedBuildings.Contains(BuildingData);
}
