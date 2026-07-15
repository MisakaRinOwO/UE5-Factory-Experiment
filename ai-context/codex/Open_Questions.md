# FactoryExperiment Open Questions

Last updated: 2026-07-15

This file tracks unclear requirements or design choices that should be confirmed with the user before changing core direction.

## Resolved: Active Chunk Definition

Decision: Empty chunks created by `CreateInitialChunks` remain only debug/hover chunks for the MVP.

Confirmed behavior:

- A chunk can exist for hover/debug/grid visibility without being simulation-active.
- A chunk becomes simulation-active when it contains at least one placed conveyor, building, item, or machine.
- Simulation should iterate active conveyors/machines/items, not every cell in every active chunk.
- Empty chunks should not be removed/deactivated after all buildables inside them are removed for the MVP.
- Future optimization may add chunk cleanup/deactivation later.

## Resolved: Chunk Size Editability

Decision: Keep chunk size compile-time fixed for MVP stability.

Current implementation appears compile-time fixed.

Reason:

- Compile-time fixed is simple and safe.
- Runtime/editable size requires refactoring `FFactoryChunk` to store size-dependent arrays and may complicate serialization/debug.
- MVP stability is more important right now.

## Resolved: Building Lookup Refactor

Decision: Keep full `FFactoryPlacedBuildingInstance` behind coord-keyed lookup for the MVP.

Current direction:

- Add footprint-friendly lookup so hovering/deleting any occupied cell of a 2x2 building resolves the same building.
- Store full instance data for convenient `Map[Coord] -> Instance -> Object/Data` queries.
- Revisit a lighter representation later only if performance or memory pressure appears.

## Resolved: Conveyor Connection Refresh Rules

Decision: Conveyor connection uses runtime `WorldPorts` generated from DA port presets.

Current behavior:

- The placed/removed conveyor should refresh.
- The four neighboring conveyors should refresh because their output may now connect/disconnect.
- A conveyor output port looks toward its neighboring coord.
- The downstream conveyor must expose a compatible input port facing the source coord.
- This MVP was validated with moving debug strings/resource state.

## Resolved: Item Representation for First Movement MVP

Decision: Use segment-buffer resource state for the MVP.

Current implementation:

```cpp
FFactoryConveyorSegment::CurrentResourceType
```

Behavior:

- Conveyor data updates on `AFactoryManager`'s `0.2s` simulation interval.
- Resource mesh visuals interpolate every tick between fixed-step conveyor cells.
- `UFactoryResourceVisualDataAsset` maps resource types to meshes.
- Debug strings/resource names remain useful if visual meshes are not configured.

Future options:

- Add true item packets if multiple items per belt, variable spacing, or richer visual interpolation is needed.
- Add conveyor speed/progress once the full production chain is complete.

## Resolved: Miner Extraction and Output

Decision: Current miner MVP is 1x1 and extracts from its occupied resource-map coord.

Current behavior:

- A first resource map has been authored in Blueprint and assigned to `AFactoryManager`.
- The resource map DA is intended for initial primary resources such as iron/copper ore positions.
- Miner ports currently output in all four directions.
- Miner determines its extracted resource from `ResourceMapData`.
- If multiple output ports connect to unblocked conveyors, miner uses round-robin output.
- Raw resource enum values were migrated to `IronOre` and `CopperOre`.

Future direction:

- Larger miners can scale productivity by resource coverage ratio, for example a 2x2 miner covering one resource cell outputs at 25% rate.

## Resolved: Resource Identity

Decision: Use DA/resource-definition direction rather than loose string-only identity.

Current state:

- `UFactoryResourceMapDataAsset` has been added/in progress.
- Existing resource enum and DA cover raw/primary resources.
- `UFactoryResourceMapDataAsset` is only for initial map distribution of primary resources such as iron ore and copper ore.
- Add next-level products to the existing enum now.

Next-level enum products:

- Iron ingot
- Iron plate
- Iron rod
- Iron bolt
- Copper ingot
- Copper wire

Recipe DA decisions should wait until First Production Recipe work.

## First Production Recipe

Question: What exact first recipe should the assembler run?

Current assumption:

- Miner produces a raw resource.
- Assembler consumes that resource and produces a processed resource.
- Storage accepts the processed resource.

Needs confirmation:

- Example recipe: `2 IronOre -> 1 IronPlate`.
- Craft time and stack limits are not yet confirmed.

## Resolved: README and Context Update Workflow

Decision: After implementation and push, ask the user whether README and `ai-context/codex/` should be updated.

Do not automatically update them after every push unless the user confirms.
