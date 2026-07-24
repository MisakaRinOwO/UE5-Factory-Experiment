# FactoryExperiment Codex Master Prompt

Last updated: 2026-07-24

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
- Selected-building hover preview and machine production debug text.
- Storage input/output runtime with slot-based resource storage.

## Current Direction

- Grid: square grid with 4-direction conveyor logic.
- World storage: sparse chunks stored in `TMap<FGridCoord, FFactoryChunk>`, with each chunk storing a dense fixed-size cell array.
- Chunk size: compile-time fixed at `32x32` for MVP stability.
- Origin convention: `AFactoryManager` actor location is the center of grid cell `(0,0)`.
- Negative coordinates: supported through floor-style chunk conversion and corrected local modulo.
- Query policy: `GetCell` is read-only and should not create chunks; `GetOrCreateCell` is for placement/world modification.
- Placement MVP: manual single-cell placement first, with build direction rotation.
- First production chain target: `Miner -> Conveyor -> Smelter -> Storage`.
- Conveyor identity: stable gameplay identity is `FGridCoord`, not HISM instance index.
- Conveyor visuals: HISM component cached per conveyor data asset.
- Port data: buildable DA ports are converted into runtime world ports and should be reused for conveyor/building connection rules.
- Simulation: centralized timer in `AFactoryManager`, intended interval `0.2s`.
- Debug: developer UI plus debug draw should make cell/chunk/building/conveyor state visible.
- Resource data: `UFactoryResourceDataAsset` maps resource types to moving meshes, static vein meshes, and stack sizes; simulation remains data-first.
- Recipe data: one `UFactoryRecipeDataAsset` represents one unique recipe. Buildings reference recipe DA directly through `RecipeData`; old global recipe database/string lookup paths should not be reintroduced.
- Buildable data: category editability is driven by `BuildableType`; use `Extractor` for miner-like resource extraction buildings and `Storage` for storage buildings.

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
- Resource vein visuals are generated from `ResourceMapData` coords using `ResourceData.ResourceVeinMeshesByType`.
- Removing a conveyor carrying ore no longer breaks moving ore HISM visuals; resource visuals are hidden instead of removed so HISM instance index compaction does not invalidate other segments.
- `DA_Smelter` / smelter assets have been added on the content side.
- Recipe DA structure exists with input/output resource maps, craft time, and allowed buildings.
- `AFactoryManager` no longer owns a global `RecipeDatabase`; machine/building DA direct `RecipeData` references are the source of truth.
- `DefaultRecipeId` legacy fallback was removed from building/machine data.
- Machine runtime data has per-port input/output storage plus shared internal storage.
- Miner extraction uses `ExtractionRatePerSecond`, mines into shared internal storage, then flushes to conveyor or adjacent machine input storage.
- Conveyor output can deliver resources into adjacent non-extractor machine input storage.
- Developer UI reports hovered resource, hovered building type id, selected buildable, and build direction.
- Number keys select conveyor/miner, and `R` rotates the build direction clockwise.
- Selected-building hover preview exists on `AFactoryManager`; it uses the selected buildable's preview mesh and falls back to conveyor mesh for conveyors.
- Smelter recipe processing works for the first recipe: `1 IronOre -> 3s -> 1 IronIngot`.
- `AFactoryManager` updates conveyors before machines so newly output resources remain visible for one fixed-step interval.
- Machine debug text can show `RecipeId` and `CraftProgress / CraftTime`.
- Conveyor resource text and machine recipe/progress text have separate debug toggles.
- Machine runtime data can be queried from Blueprint by hovered/selected coord or building id for future UI.
- `W_BuildingInfo` exists on the Blueprint/content side, but it is not connected to `PC_Factory` yet.
- Storage runtime data exists separately from machine runtime data.
- Storage input receives resources through DA-authored input ports.
- Storage output flushes resources through DA-authored output ports using the same conveyor/building output target path as machines.
- Storage slots are controlled by `AvailableSlots`; each slot stores one resource type up to the resource stack size.
- Storage runtime data can be queried from Blueprint by coord or building id.
- Miner/extractor debug text shows extracted resource, extraction progress/rate, and internal storage.
- Storage debug text shows slot contents.

Current incomplete areas:

- `W_BuildingInfo` is not wired into click-selection / `PC_Factory` yet.
- Conveyor movement is data-correct for MVP but does not yet use conveyor speed as a throughput/progress parameter.
- Moving resource visual removal is MVP-safe but not memory-reusing yet; hidden instances may accumulate until a future pooling/free-list pass. Keep this implementation for now because it preserves correctness and avoids extra lifecycle complexity before assembler/storage/recipe flow is complete.
- Building footprint lookup should likely move toward a coord-keyed lookup for robust hover/delete.
- Future architecture may unify miner/extractor production with processor-machine recipe production through a terrain/resource input source. Do not do this during the current MVP unless explicitly requested; the current miner path is verified and should stay stable.

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
2. Keep the current smelter recipe slice runnable:
   - `1 IronOre -> 3s -> 1 IronIngot`.
   - Validate direct `Miner -> Smelter` and `Miner -> Conveyor -> Smelter` paths.
3. Implement the next production/UI slice:
   - Keep storage input/output behavior stable.
   - Click a building in `PC_Factory` and open `W_BuildingInfo`.
   - Conveyor can deliver into building input ports.
4. Add BP UI display for machine inventory, crafting progress, and output blocked state using the existing runtime query APIs.
5. Improve building footprint lookup:
   - Add coord-keyed mapping so hovering any footprint cell resolves the same building instance.
6. Consider conveyor speed/progress only after the fixed-step production chain is complete.
7. Later, consider `Extractor/Machine Runtime Unification`:
   - Miner becomes a special machine with terrain/resource-map input.
   - Normal processor machines and miners share the same production lifecycle.
   - This can also motivate cleaning up the currently unified `UFactoryBuildingDataAsset`.

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
