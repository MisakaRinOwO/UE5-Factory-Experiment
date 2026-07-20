# FactoryExperiment Design Decisions

Last updated: 2026-07-20

This file records the reasoning behind the current implementation direction. It is based on the existing project memory and current repository state.

## Standalone Project

Decision: FactoryExperiment is a separate UE C++ project, not just a new level inside the older Grid Experiment.

Reasoning:

- The factory prototype has a different portfolio story from pathfinding/grid experiments.
- It can still reuse ideas from previous work, especially grid-cell struct design and top-down pawn control/view.
- A separate repo/project gives it a clearer identity as a factory systems case study.

## Square Grid and 4-Direction Conveyors

Decision: Use a square grid and 4-direction conveyor movement.

Reasoning:

- This matches the intended shapez-like factory layout better than triangular grid logic.
- It keeps placement, pathfinding, ports, conveyor direction, and debug visualization easier to reason about.
- It is enough for the first production chain MVP.

## Chunked Sparse World

Decision: Store world cells as sparse chunks:

```cpp
TMap<FGridCoord, FFactoryChunk> Chunks;
```

Each chunk contains a dense fixed-size cell array:

```cpp
TArray<FFactoryGridCell> Cells;
```

Reasoning:

- A single dense world array is simple but does not support large-world capability well.
- A pure sparse `TMap<FGridCoord, FFactoryGridCell>` wastes overhead per cell and has weaker locality.
- Chunked dense storage gives fast local indexing inside active chunks while avoiding allocation for unused world space.

Current chunk size:

```text
32x32
```

Chunk size decision:

- Keep chunk size compile-time fixed for the MVP.
- Do not refactor chunk size into an editable `AFactoryManager` property yet.
- Reason: MVP stability and lower implementation risk matter more than runtime tunability right now.

## Origin and Negative Coordinates

Decision: `AFactoryManager` actor location is the center of grid cell `(0,0)`, and negative coordinates are supported.

Reasoning:

- The world can expand naturally outward from the origin.
- Negative coordinate support avoids future rework when building around the manager origin.
- Chunk conversion uses floor-style division and positive local modulo correction so cells like `(-1,0)`, `(-32,0)`, and `(-33,0)` map correctly.

## GetCell vs GetOrCreateCell

Decision:

- `GetCell` is read-only and should not create chunks.
- `GetOrCreateCell` creates chunks and should be used only for placement/modification.

Reasoning:

- Hover, debug preview, and path preview should not mutate world state.
- Placement/removal is the explicit point where inactive world areas become active chunks.
- This keeps "look at world" and "change world" behavior cleanly separated.

## Lightweight Grid Cells

Decision: `FFactoryGridCell` should remain a small spatial record.

Current role:

```text
Occupancy
Direction
BuildingId
ConveyorCoord
```

Reasoning:

- Cells should answer "what is here?" quickly.
- Cells should not own full machine inventory, recipe state, actor-owned logic, or item movement logic.
- Heavy runtime state belongs in manager-owned maps/arrays or actor/data structures outside the cell.

## Unified Buildable Data Asset

Decision: `UFactoryBuildingDataAsset` currently represents both machine/building definitions and conveyor definitions.

Reasoning:

- The build UI wants one selected placeable path.
- It is easy to add conveyor grades or building types by creating new DA instances.
- `BuildableType` provides semantic organization.
- `bIsConveyor` remains the current implementation branch for placement.

When to revisit:

- The asset becomes too crowded in the editor.
- Machine and conveyor validation diverge heavily.
- Different asset inheritance is needed for authoring clarity.

## Actor-Backed Buildings

Decision: Major buildings/machines are actor-backed through `AFactoryBuilding` / `AFactoryMachine` and BP child actors.

Reasoning:

- Machines benefit from selection, visible meshes, status lights, ports, and BP-authored presentation.
- Actor-backed buildings are more transparent in editor and easier to author visually.
- Runtime production state should still move toward data-oriented C++ structs instead of living only in BP actors.

## Data-Backed Conveyors

Decision: Conveyors are not individual actors. They are represented by `FFactoryConveyorSegment` and rendered with HISM.

Reasoning:

- Actor-per-conveyor does not scale to large factory layouts.
- Conveyor identity is naturally its grid coord while conveyors are one-cell buildables.
- HISM provides efficient rendering and keeps the level/outliner cleaner.

## Conveyor Identity

Decision: Conveyor stable identity is `FGridCoord`, not integer id and not HISM instance index.

Reasoning:

- The common query path starts from cursor/world position to grid coord.
- Conveyor is currently 1x1, so coord is unique.
- Deletion and hover lookup are simpler with coord-keyed maps.
- It avoids early dense array/id/index maintenance complexity.

Current storage:

```cpp
TMap<FGridCoord, FFactoryConveyorSegment> ConveyorsByCellCoord;
```

## Runtime Port Data

Decision: Use DA-authored port presets as the source for runtime connection behavior.

Current implementation state:

- `UFactoryBuildingDataAsset` owns `Ports`.
- Each port has local coord, direction, port type, and accepted item id.
- `AFactoryManager::BuildWorldPorts` rotates local ports into world-space `FFactoryPlacedBuildingPort` values for the placed build direction.
- `FFactoryPlacedBuildingInstance` stores `WorldPorts`.
- `FFactoryConveyorSegment` also stores `WorldPorts`.

Reasoning:

- Connection rules should not be hardcoded separately for conveyors, miners, and machines.
- DA-authored ports let Blueprint assets define preset input/output behavior.
- Runtime `WorldPorts` already captures rotated direction and world coord, so conveyor connection can reuse it for MVP.

MVP connection implication:

- A conveyor output port should look toward its neighboring coord.
- A valid downstream connection should prefer a matching input port on the neighbor.
- For machine/miner/storage connection, the same placed port shape should be reused.
- If strict port validation slows MVP progress, fall back to conveyor direction to `NextCoord` but keep port data as the intended source of truth.

## HISM Instance Index Is Visual Only

Decision: HISM instance index is not gameplay identity.

Reasoning:

- HISM/ISM removal may compact or reorder internal instance indices.
- Gameplay state should not break because rendering storage changed.
- `VisualInstanceIndex` is only a handle for updating/removing the current visual.

Current removal tradeoff:

- After removing a conveyor instance, `RepairConveyorVisualInstanceIndices` scans and repairs remaining indices by matching world location.
- This is simple and acceptable for MVP.
- If deletion becomes a performance problem, replace this with a reverse lookup or lazy instance reuse/free-list.

## Centralized Simulation Timer

Decision: Factory simulation should run from `AFactoryManager` on a fixed-step timer, not from per-frame machine ticks.

Current intended interval:

```text
0.2 seconds
```

Reasoning:

- Machine crafting and conveyor logic do not need per-frame updates.
- A centralized update is easier to debug and profile.
- It supports deterministic-ish simulation thinking and clearer portfolio messaging.
- Visual interpolation can be added later separately from simulation.

Item movement MVP decision:

- Focus first on data correctness, not polished visuals.
- Use the existing `0.2s` simulation interval to update conveyor/machine state.
- Conveyor speed can exist as a DA parameter, but first implementation can treat speed as "attempt one transfer per simulation step" unless a simple accumulator is needed.
- Use debug drawing or developer UI to display current item/resource state before adding smooth mesh/item movement.

Resource visual follow-up:

- Smooth movement visual should be layered on top of the fixed-step data simulation.
- `UFactoryResourceVisualDataAsset` maps `EFactoryResourceType` to moving resource meshes.
- `UFactoryResourceVisualDataAsset` also maps `EFactoryResourceType` to static resource vein meshes.
- `AFactoryManager` creates/caches one HISM component per resource type for moving resources.
- `AFactoryManager` creates/caches one HISM component per resource type for static resource veins.
- Conveyor segments keep the resource visual instance index plus visual from/to coords and elapsed time.
- Data still moves on the `0.2s` simulation step; visuals interpolate every tick from previous coord to current coord.
- If `ResourceVisualData` or a resource mesh is missing, the data simulation should still work and debug strings remain useful.

## Ore Data and Visual Movement

Decision: Ore movement is data-first, with HISM visuals as a readable presentation layer.

Current data path:

```text
ResourceMapData coords
  -> 1x1 Miner detects covered resource type
  -> Miner round-robin chooses a connected, unblocked output port
  -> TryPushResourceToConveyor writes CurrentResourceType onto the target conveyor segment
  -> UpdateConveyors moves CurrentResourceType one conveyor segment per 0.2s simulation step
```

Current visual path:

```text
ResourceVisualData.ResourceMeshesByType
  -> one moving-resource HISM component per resource type
  -> conveyor segment stores ResourceVisualInstanceIndex
  -> segment stores ResourceVisualFromCoord / ResourceVisualToCoord / ResourceVisualMoveElapsed
  -> Tick interpolates the HISM instance between fixed-step source and target coords
```

Static ore vein path:

```text
ResourceMapData.ResourceCoordsByType
  -> ResourceVisualData.ResourceVeinMeshesByType
  -> CreateResourceVeinVisuals at BeginPlay
  -> one static vein HISM component per resource type
```

Original approach:

- Data correctness came first: conveyor segments only stored `CurrentResourceType`.
- Debug strings proved that ore moved correctly before polished visuals existed.
- Smooth visuals were then layered over the fixed-step state by storing a HISM instance index on the segment that currently contains the resource.

Issue encountered:

- When a conveyor carrying ore was removed, resource/conveyor HISM visuals could disappear or appear misaligned.
- The underlying risk is that HISM `RemoveInstance` can compact/reorder instance indices.
- If a segment stores an instance index and another instance is removed or repaired incorrectly, the segment may continue updating the wrong visual instance.
- This confirmed the earlier decision that HISM instance index must remain visual-only and should not be treated as gameplay identity.

Fix:

- Gameplay identity remains `FGridCoord` and `EFactoryResourceType`, not HISM instance index.
- When removing a conveyor that contains a moving resource, `RemoveResourceVisual` now hides that moving-resource instance instead of calling `RemoveInstance`.
- Hiding moves the instance far below the map and scales it to zero, avoiding HISM index compaction for moving resources.
- Conveyor visual removal can still use `RemoveInstance` plus `RepairConveyorVisualInstanceIndices` because conveyor visuals are stationary and can be repaired by matching grid/world location.

Tradeoff:

- Hidden moving-resource instances may accumulate during heavy remove/place testing.
- This is acceptable for MVP because it keeps behavior stable and easy to verify.
- Future optimization can replace hiding with a free-list/pool, or rebuild moving-resource HISM instances from `ConveyorsByCellCoord` when needed.

Decision for now:

- Keep the current hide-instead-of-remove implementation.
- Do not add moving-resource visual pooling yet.

Reasoning:

- The current priority is a stable, runnable production-chain MVP.
- Hiding the instance avoids the immediate correctness bug caused by HISM instance index compaction.
- The likely cost only appears after long play sessions or repeated deletion of conveyors that are actively carrying resources.
- For short portfolio demos and current editor testing, the hidden-instance count should remain small enough to be acceptable.
- Adding pooling/free-list now would improve long-session hygiene, but it adds lifecycle complexity before assembler/storage/recipe flow is complete.

Potential performance issue:

- A hidden moving-resource instance still exists in the HISM component.
- Over time, frequent deletion of loaded conveyors can increase the internal instance count.
- Even if the hidden instances are below the map and scaled to zero, they may still increase memory use and HISM buffer/update overhead.
- This becomes worth optimizing if profiling shows large resource visual instance counts or delete/place stress tests start hitching.

Future optimization options:

- Add `TMap<EFactoryResourceType, TArray<int32>>` free lists for hidden moving-resource instance indices.
- Make `AddResourceVisual` reuse a hidden instance before calling `AddInstance`.
- Periodically rebuild moving-resource HISM instances from live `ConveyorsByCellCoord` data during a safe maintenance point.
- Move toward explicit item packets only if conveyor density, spacing, or multi-item-per-belt behavior requires it.

## Coordinate-Based Pathfinding Interface

Decision: Pathfinding should use world grid coordinates and query `AFactoryManager` through public methods.

Expected interface style:

```cpp
GetCell(Coord)
IsCellOccupied(Coord)
IsCellValidForPath(Coord)
GetNeighbors(Coord)
```

Reasoning:

- A* should not care whether storage is dense array, chunked map, or something else.
- Cross-chunk pathfinding works naturally if all neighbors are world coords.
- Storage details remain isolated in `AFactoryManager`.

## Developer Mode UI and Debug Draw

Decision: Use developer UI plus debug draw to make data-driven state visible.

Reasoning:

- Struct/map-driven systems are less visible than actor-only designs.
- Hover debug and UI compensate by showing cell coord, chunk coord, occupancy, direction, building, and conveyor data.
- Debug tools are part of the portfolio signal because they show the system can be inspected and reasoned about.

## Building Lookup Direction

Current implementation: building actors are stored in an array by `BuildingId`, and footprint cells store `BuildingId`.

Current direction: keep coord-keyed lookup using full placed-building instance data for MVP, such as:

```cpp
TMap<FGridCoord, FFactoryPlacedBuildingInstance>
```

Each occupied footprint cell can map to the complete `FFactoryPlacedBuildingInstance`.

Reasoning:

- Hovering any cell of a 2x2 building should resolve the same building.
- Deletion should clear all footprint cells without scanning large world state.
- Stable footprint lookup will matter before machine interaction/UI becomes more complex.
- Keeping the complete placed instance behind the coord key makes MVP lookup convenient: `Coord -> Instance -> Actor/Data/Ports/OccupiedCoords`.
- If this becomes a memory or performance bottleneck later, optimize to a lightweight id/ref per footprint cell plus canonical instance storage.

## Active Chunks vs Created Chunks

Decision: distinguish chunks that exist for debug/hover coverage from chunks active in simulation.

Current state:

- `CreateInitialChunks` creates four chunks around the origin: `(0,0)`, `(-1,0)`, `(0,-1)`, `(-1,-1)`.
- This helps debug the manager-centered origin.

Reasoning:

- The user wants chunks with no buildings/conveyors/items to be inactive for simulation.
- Created chunks should not automatically mean simulation-active chunks.
- Simulation should iterate active conveyors/machines/items, not every cell in every chunk.

MVP behavior:

- Empty chunks created by `CreateInitialChunks` remain debug/hover chunks only.
- Empty chunks are not removed or deactivated after buildables inside them are removed.
- Future optimization can add chunk cleanup/deactivation if profiling or world scale requires it.

## Resource Identity

Decision: resource identity should use data-oriented resource definitions, not loose strings.

Current implementation state:

- `EFactoryResourceType` exists in `FactoryResourceMapDataAsset.h`.
- `UFactoryResourceMapDataAsset` maps resource types to initial world coordinates.
- The resource map DA is currently intended only for initial map distribution of primary resources such as iron/copper ore.
- The map value uses a wrapper struct instead of a bare `TArray` because UHT does not allow `TArray<FGridCoord>` directly as a reflected `TMap` value.
- Raw resource enum values were migrated to `IronOre` and `CopperOre`.

Current reflected storage shape:

```cpp
TMap<EFactoryResourceType, FFactoryResourceCoordList> ResourceCoordsByType;
```

Current enum direction:

- Keep the existing raw resource enum entries for the current DA path.
- Add next-level production outputs to the same enum for now:
  - Iron ingot
  - Iron plate
  - Iron rod
  - Iron bolt
  - Copper ingot
  - Copper wire

Recipe direction:

- Do not overload `UFactoryResourceMapDataAsset` with recipe concerns.
- Production recipe DA work should be considered later when implementing the first production recipe.

Miner/resource map direction:

- The first resource map has been authored in Blueprint and assigned to `AFactoryManager`.
- The map DA should define where primary resources exist in the world.
- Miner extraction should query the world/resource map to decide what it can produce, rather than each placed miner hardcoding a resource by default.
- Miner output should use its output ports. The current first miner setup has output ports in all four directions.
- Current miner MVP is 1x1 only.
- Future larger miners can scale productivity by resource coverage ratio, for example a 2x2 miner covering one resource cell outputs at 25% rate.
- If multiple output ports are connected and unblocked, miner output should use round-robin.

## Developer Mode Widget Events

Decision: expose C++ events that give Blueprint the display-ready context it needs instead of making the widget infer everything from low-level ids.

Current event direction:

- `SetCurrentCellCoord` includes `EFactoryResourceType` so hover UI can show resource name or N/A.
- `UpdateBuildingOnCell` includes `BuildingTypeId` so BP does not need to infer building name from `BuildingId`.
- `UpdateSelectedBuildable` reports selected buildable DA, buildable type id, and current build direction.
- `ClearSelectedBuildable` clears selected-buildable UI.

Reasoning:

- `BuildingId` is runtime identity, not user-facing display name.
- `BuildingTypeId` from the DA is the intended display/source id for hover UI.
- Selected buildable and direction belong in the developer HUD because they are core placement state.

## Build Selection Input

Decision: keep early placement input simple and explicit.

Current behavior:

- Number key `1` selects conveyor.
- Number key `2` selects miner.
- `R` rotates the current build direction clockwise.
- Developer UI has been verified to update selected buildable and direction accurately.
