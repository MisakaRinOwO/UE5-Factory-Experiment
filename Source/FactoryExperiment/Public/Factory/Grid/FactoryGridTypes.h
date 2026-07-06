#pragma once

#include "CoreMinimal.h"
#include "Factory/FactoryTypes.h"
#include "FactoryGridTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryGridCell
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryCellOccupancy Occupancy = EFactoryCellOccupancy::Empty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryDirection Direction = EFactoryDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BuildingId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConveyorId = -1;

	bool IsEmpty() const
	{
		return Occupancy == EFactoryCellOccupancy::Empty;
	}

	bool IsOccupied() const
	{
		return Occupancy != EFactoryCellOccupancy::Empty;
	}
};

USTRUCT(BlueprintType)
struct FFactoryChunk
{
	GENERATED_BODY()

	static constexpr int32 ChunkSize = 32;
	static constexpr int32 CellCount = ChunkSize * ChunkSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryGridCell> Cells;

	FFactoryChunk()
	{
		Cells.SetNum(CellCount);
	}

	static int32 LocalCoordToIndex(int32 LocalX, int32 LocalY)
	{
		return LocalX + LocalY * ChunkSize;
	}

	FFactoryGridCell* GetCell(int32 LocalX, int32 LocalY)
	{
		if (LocalX < 0 || LocalX >= ChunkSize || LocalY < 0 || LocalY >= ChunkSize)
		{
			return nullptr;
		}

		return &Cells[LocalCoordToIndex(LocalX, LocalY)];
	}

	const FFactoryGridCell* GetCell(int32 LocalX, int32 LocalY) const
	{
		if (LocalX < 0 || LocalX >= ChunkSize || LocalY < 0 || LocalY >= ChunkSize)
		{
			return nullptr;
		}

		return &Cells[LocalCoordToIndex(LocalX, LocalY)];
	}
};