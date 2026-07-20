# FactoryExperiment Codex Master Prompt

Last updated: 2026-07-20

## Project Summary

FactoryExperiment is a standalone UE5 C++ project for a portfolio-focused top-down factory simulation prototype. It builds on earlier experience from Void Architect top-down pawn control/view and the previous Grid Experiment's grid-cell struct style, but this repository is its own project.

The project is inspired by flat factory/simulation games such as shapez and Oxygen Not Included. The goal is not to clone a complete game. The goal is to demonstrate strong gameplay/systems programming ability through a clear, scalable factory-system foundation:

- Chunked square-grid world storage.
- Manual building and conveyor placement.
- Data-driven conveyors and machine runtime state.
- Blueprint-authored visual actors for major buildings.
- HISM/ISM conveyor visuals instead of actor-per-conveyor.
- Centralized fixed-step simulation.
- Debug UI and debug drawing for data-driven state.
- Smooth resource mesh feedback layered over fixed-step data simulation.

## Current Direction

- Grid: square grid with 4-direction conveyor logic.
- World storage: sparse chunks stored in `TMap<FGridCoord, FFactoryChunk>`, with each chunk storing a dense fixed-size cell array.
- Chunk size: compile-time fixed at `32x32` for MVP stability.
- Origin convention: `AFactoryManager` actor location is the center of grid cell `(0,0)`.
- Negative coordinates: supported through floor-style chunk conversion and corrected local modulo.
- Query policy: `GetCell` is read-only and should not create chunks; `GetOrCreateCell` is for placement/world modification.
- Placement MVP: manual single-cell placement first, with build direction rotation.
- First production chain target: `Miner -> Conveyor -> Assembler -> Storage`.
- Conveyor identity: stable gameplay identity is `FGridCoord`, not HISM instance index.
- Conveyor visuals: HISM component cached per conveyor data asset.
- Port data: buildable DA ports are converted into runtime world ports and should be reused for conveyor/building connection rules.
- Simulation: centralized timer in `AFactoryManager`, intended interval `0.2s`.
- Debug: developer UI plus debug draw should make cell/chunk/building/conveyor state visible.
- Resource visuals: `UFactoryResourceVisualDataAsset` maps resource types to moving resource meshes and static resource vein meshes; simulation remains data-first.

## Current Progress Snapshot

Implemented or present in the repository:

- `GM_Factory`, `PC_Factory`, `P_FactoryPawn`.
- Enhanced Input Actions and `IMC_FactoryMain`.
- Main level `L_FactoryMain`.
- Developer widget `W_DeveloperMode` with C++ base `UFactoryDeveloperModeWidget`.
- `AFactoryManager` with chunk storage, hover raycast, debug draw, placement/removal, conveyor HISM visuals, developer UI updates, and simulation timer shell.
- `AFactoryManager` with chunk storage, hover raycast, debug draw, placement/removal, conveyor HISM visuals, resource visual HISM components, developer UI updates, and simulation timer.
- `UFactoryBuildingDataAsset` as the current unified buildable definition asset.
- Actor-backed building path through `AFactoryBuilding` / `AFactoryMachine`.
- Data-backed conveyor path through `FFactoryConveyorSegment`.
- `DA_Miner`, `DA_Conveyor`, `BP_Miner`, and early mesh/content assets.
- `UFactoryResourceMapDataAsset` appears to be in progress under `Factory/Resources`.
- Resource identity is moving toward DA/resource-definition driven data, with the current resource map DA used only for initial primary resource distribution.
- A first resource map has been authored in Blueprint and assigned to `AFactoryManager`.
- Miner extraction from resource map cells works for the current 1x1 miner.
- Miner output uses round-robin across connected, unblocked output conveyors.
- Conveyor item/resource movement works on the fixed-step simulation timer.
- Resource mesh visuals interpolate every frame between fixed-step conveyor cells.
- Resource vein visuals are generated from `ResourceMapData` coords using `ResourceVisualData.ResourceVeinMeshesByType`.
- Removing a conveyor carrying ore no longer breaks moving ore HISM visuals; resource visuals are hidden instead of removed so HISM instance index compaction does not invalidate other segments.
- Developer UI reports hovered resource, hovered building type id, selected buildable, and build direction.
- Number keys select conveyor/miner, and `R` rotates the build direction clockwise.

Current incomplete areas:

- Assembler and storage logic are not implemented yet.
- Recipe processing and output buffering are not implemented yet.
- Conveyor movement is data-correct for MVP but does not yet use conveyor speed as a throughput/progress parameter.
- Moving resource visual removal is MVP-safe but not memory-reusing yet; hidden instances may accumulate until a future pooling/free-list pass. Keep this implementation for now because it preserves correctness and avoids extra lifecycle complexity before assembler/storage/recipe flow is complete.
- Building footprint lookup should likely move toward a coord-keyed lookup for robust hover/delete.

## Agent Operating Rules

When continuing work on this project:

1. First preserve runnability. Make the smallest coherent change that compiles and can be tested in editor before adding polish.
2. Do not rewrite the whole source tree or reorganize folders unless the user explicitly asks. Keep changes scoped.
3. Respect the current architecture direction: chunked grid, coord-keyed conveyor identity, data-driven conveyor state, Blueprint visuals for authored buildings, centralized simulation.
4. Do not silently change core direction. If the request is unclear or conflicts with the current direction, write the issue in `Open_Questions.md` and ask the user.
5. Prefer implementing behavior in C++ where it is simulation or system logic. Use Blueprint for visuals, asset authoring, UI wiring, and editor-facing setup.
6. Avoid actor-per-cell, actor-per-conveyor, and actor-per-item as default architecture.
7. Pathfinding should operate on world grid coordinates and query `AFactoryManager`; it should not directly read chunk-local arrays.
8. HISM instance indices are visual implementation details and must not become gameplay identity.
9. Every work summary should state:
   - Files changed.
   - What behavior was implemented.
   - How it was verified.
   - What the user needs to hook up or adjust in the UE editor/BP.
10. After implementing and pushing changes, ask the user whether README and the files in `ai-context/codex/` should be updated to stay current.

## Recommended Next Implementation Order

1. Keep the current miner/conveyor/resource visual slice runnable.
2. Implement the next production slice:
   - Storage accepts resources from conveyors.
   - Assembler consumes inputs and produces outputs from recipe data.
   - Conveyor can deliver into building input ports.
3. Add debug UI for machine inventory, crafting progress, and output blocked state.
4. Improve building footprint lookup:
   - Add coord-keyed mapping so hovering any footprint cell resolves the same building instance.
5. Consider conveyor speed/progress only after the fixed-step production chain is complete.

## Verification Expectations

For C++ changes:

- Build `FactoryExperimentEditor Win64 Development`.
- Run PIE if editor/BP hookup is involved.
- Test at least one positive case and one failure/removal case when touching placement/removal.

For editor/BP-dependent changes:

- State exactly what asset needs to be assigned or updated.
- State which BP event/function needs to be connected.
- Do not assume BP assets are wired just because C++ compiles.

## Portfolio Message

The final portfolio write-up should emphasize:

- The project separates simulation data from visual actors.
- Chunked grid storage supports larger worlds without allocating the whole world upfront.
- Conveyors are data-driven and rendered through instancing.
- Buildings are authored through BP/DataAssets while C++ owns placement and simulation.
- Debug tools make the data-driven simulation inspectable.
- The demo shows a complete factory loop, not just isolated placement.
