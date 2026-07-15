# FactoryExperiment

FactoryExperiment is a UE C++ systems prototype for a top-down factory simulation. It is designed as a portfolio-focused experiment that demonstrates chunked grid storage, data-driven factory simulation, Blueprint-authored building actors, instanced conveyor visuals, and debug tooling.

The project is inspired by flat factory/simulation games such as *shapez* and *Oxygen Not Included*, but the focus is not to clone a full game. The goal is to build a clear, scalable gameplay-system prototype in Unreal Engine.

## Project Goals

- Build a square-grid factory simulation in UE C++.
- Support large-world style expansion through chunked grid storage.
- Avoid Actor-per-cell and Actor-per-conveyor architecture.
- Use data-driven runtime state for conveyors, machines, and inventories.
- Use Blueprint child actors for visual authoring and interaction.
- Use a centralized fixed-step simulation timer instead of per-frame production logic.
- Provide debug visualization for cell, chunk, building, and conveyor state.

## Current Status

Implemented / added so far:

- Separate UE C++ project setup.
- Factory GameMode.
- Factory PlayerController.
- FactoryPawn.
- Enhanced Input Actions.
- Input Mapping Context.
- Reused top-down pawn control/view direction from earlier Void Architect work.
- Reused prior grid-cell struct style from the Grid Experiment.
- Chunked square-grid storage with negative coordinate support.
- Hover cell/chunk debug UI and debug drawing.
- Manual buildable selection and placement.
- Number-key building selection for conveyor/miner.
- `R` key clockwise build direction rotation.
- Data-asset driven miner and conveyor definitions with runtime world ports.
- HISM conveyor visuals.
- Resource map data asset for initial raw resource distribution.
- Miner extraction from resource map cells.
- Round-robin miner output across unblocked connected output conveyors.
- Fixed-step conveyor item/resource movement.
- Resource visual data asset for resource mesh feedback.
- Smooth per-frame resource visual interpolation over fixed-step simulation movement.

Immediate next step:

- Extend the first production chain beyond miner output by adding assembler/storage input, recipe processing, and output buffering.

## Core Design Direction

### Grid

The world uses a square grid with 4-direction logic.

Default chunk configuration:

```text
ChunkSize = 32x32
```

The chunk size is currently compile-time fixed for MVP stability.

The world supports negative grid coordinates. `FactoryManager` is treated as the center of grid cell `(0, 0)`, and the world expands outward from there.

### Chunked World Storage

The world is stored as sparse chunks with dense arrays inside each chunk:

```text
TMap<FGridCoord, FFactoryChunk> ActiveChunks
```

Each chunk stores:

```text
TArray<FFactoryGridCell> Cells
```

Important query policy:

```text
GetCell = read-only; does not create chunks
GetOrCreateCell = write path; creates chunks during placement/modification
```

Hover and path preview should use `GetCell`. Placement should use `GetOrCreateCell`.

### Cell Data

Factory cells should stay lightweight:

```text
Occupancy
Direction
BuildingId
ConveyorCoord
```

Cells should not own heavy runtime data such as full machine inventory, recipe state, or actor-owned simulation logic.

### Buildings

Buildings use a C++ base actor and Blueprint children:

```text
AFactoryBuilding
  BP_Building_Miner
  BP_Building_Assembler
  BP_Building_Storage
```

Initial building sizes:

```text
Miner     = 1x1
Conveyor  = 1x1
Assembler = 2x2
Storage   = 2x2
```

### First Production Chain

The first functional chain is:

```text
Miner -> Conveyor -> Assembler -> Storage
```

Machine inventory starts as simple input/output arrays. Per-port buffers can be added later after the full simulation loop works.

Current working slice:

```text
Resource Map -> Miner -> Conveyor
```

The miner is currently treated as a 1x1 extractor. Larger miners may later scale output by resource coverage ratio.

### Conveyors

Conveyors use 4-direction movement:

```text
Up -> Right -> Down -> Left
```

First version:

- Manual single-cell placement.
- `R` rotates the build direction.
- Conveyor visuals use Instanced Static Mesh / Hierarchical Instanced Static Mesh.
- One HISM component for straight conveyor visuals is enough for MVP.
- Direction is represented by instance transform rotation.
- Connection uses DA-authored runtime world ports.
- Runtime identity is grid coordinate, not actor id or HISM instance index.

Conveyors are stored as data structs rather than one Actor per conveyor segment.

### Simulation

The factory simulation should use a centralized fixed-step timer in `FactoryManager`.

Recommended first interval:

```text
0.2 seconds
```

Production logic should not require per-frame Tick.

The intended update model is:

```text
Update active conveyors
Update active machines
Update active item packets
```

The system should avoid iterating every cell in every chunk during simulation.

Current resource movement:

- Miner output is updated by the centralized simulation timer.
- Conveyor data advances at the fixed simulation step.
- Resource mesh visuals interpolate per frame between conveyor cells.

## Input and Control

The project has the basic input/control layer:

```text
Factory GameMode
Factory PlayerController
FactoryPawn
Input Actions
Input Mapping Context
```

Expected responsibilities:

```text
FactoryPawn:
  top-down camera, movement, reusable VA-style view/control feel

FactoryPlayerController:
  Enhanced Input setup, input action binding, mouse/grid hover query,
  placement command routing, number-key buildable selection, R-key rotation command

FactoryManager:
  grid conversion, chunk/cell query, placement, simulation, debug output
```

## Pathfinding Direction

Pathfinding is not part of the first conveyor MVP, but the architecture should remain cross-chunk ready.

A* should operate on world grid coordinates and query `FactoryManager` through interface methods:

```text
GetCell(Coord)
IsCellOccupied(Coord)
CanPlaceConveyorAt(Coord)
GetFourWayNeighbors(Coord)
```

Pathfinding should not directly read chunk-local arrays.

For future path-assisted conveyor placement, the search should include safety bounds:

```text
MaxSearchDistance
SearchBounds around start/end
MaxExpandedNodes
```

## Debug Plan

First debug layer:

- `DrawDebugString`

Hover debug should show:

```text
World Coord
Chunk Coord
Local Coord
Cell Index
Occupancy
Direction
BuildingId
ConveyorId
Chunk Active State
```

Later debug additions may include:

- Conveyor arrows
- Chunk boundaries
- Selected building state
- Machine recipe/inventory/progress
- Active chunk count
- Active conveyor count
- Simulation step counter

Current developer UI also reports:

- Hovered resource type.
- Current buildable selection.
- Current build direction.
- Building type id for occupied cells.

## Planned MVP Milestones

1. Validate GameMode / PlayerController / FactoryPawn / IA / IMC setup. Done.
2. Implement chunk coordinate conversion. Done.
3. Implement `GetCell` and `GetOrCreateCell`. Done.
4. Add hover debug and developer UI. Done.
5. Add manual conveyor/miner placement. Done.
6. Add HISM conveyor visuals. Done.
7. Add centralized 0.2s simulation timer. Done.
8. Implement miner extraction and conveyor item transfer. Done.
9. Add smooth resource visual feedback. Done.
10. Implement assembler/storage recipe processing.
11. Record a short portfolio demo showing placement, chunk debug, conveyor movement, and production.

## Portfolio Focus

The final portfolio write-up should emphasize:

- Why Actor-per-cell was avoided.
- How chunked grid storage supports larger worlds.
- How cell occupancy, building actors, conveyor data, and machine runtime state are separated.
- How Blueprint is used for authoring/visuals while C++ owns simulation logic.
- How fixed-step simulation improves clarity and control.
- How debug tooling makes data-driven systems transparent.
