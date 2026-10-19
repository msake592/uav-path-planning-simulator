# 3D UAV Path Planning & Optimization Simulator — Project Plan

> **Status:** Architecture baseline v1.0 · **Date:** 2026-10-19  
> **Time horizon:** 4–6 week MVP; extensible 3–4 month research/engineering roadmap  
> **Purpose:** Educational, portfolio-oriented planning and simulation platform; **not** flight-certified or intended for real-world vehicle control.

## 1. Vision and engineering objective

Develop a **modular, high-performance 3D UAV path-planning and optimization framework** with interactive visualization and reproducible experiments. The central engineering question is:

> **When is flying over an obstacle preferable to flying around it, once path length, ascent cost, obstacle clearance, and computational cost are considered?**

The project must demonstrate competence in **modern C++**, 3D geometry and computer graphics, search algorithms, optimization, verification, benchmarking, and engineering documentation. The priority is **correctness and defensible experimental evidence**, not visual polish or a large feature count.

### Expected deliverables

- A native C++ planning library usable independently of the UI.
- A headless CLI for deterministic scenario execution and benchmarks.
- A React + Three.js interface to inspect environments, paths, and algorithm behavior.
- A validated 3D voxel planning model with collision-safe transitions.
- Dijkstra and A* with configurable, nonnegative edge costs.
- Experiments showing conditions under which **over** and **around** routes are preferred.
- Automated tests, versioned scenario definitions, documented assumptions, and a portfolio-ready demo.

## 2. Architecture decisions — baseline

| Area | Selected approach | Engineering rationale / boundary |
|---|---|---|
| Planning and simulation core | **C++20** | Native performance, data structure control, portability to simulation/robotics tooling. |
| Build | **CMake + CTest** | Standard cross-platform C++ build/test orchestration. |
| Dependencies | **vcpkg**, manifest mode | Reproducible third-party dependencies. |
| Unit tests | **GoogleTest** | Well-supported C++ testing. |
| Visualization | **Three.js** | Focus engineering effort on planning rather than a rendering engine. |
| UI | **React + TypeScript** | Interactive inspection and controls. |
| Application style | **Modular monolith** | Explicit boundaries without unnecessary distributed infrastructure. |
| Core/UI integration | **HTTP/JSON adapter** | Simple inspection/debugging; native core remains usable without the server. |
| Core world frame | **Right-handed, Z-up, meters** | Explicit 3D planning coordinates; conversion at renderer boundary. |
| Initial spatial model | **Bounded dense 3D occupancy grid** | Testable reference model; replaceable storage abstraction. |
| Planning connectivity | **26-connected**, collision-validated | Allows vertical and diagonal movement without invalid corner cutting. |
| Collision checking | **Full edge/segment validation + clearance** | Occupied endpoints alone do not establish a safe transition. |
| First planners | **Dijkstra + A\*** | Correctness baseline and heuristic-search comparison. |
| Initial A* heuristic | **Scaled Euclidean distance** | Admissible lower bound when edge costs dominate weighted path length. |
| Optimization | **Weighted sum of distance, ascent proxy, clearance penalty** | Interpretable trade-off studies, with nonnegative terms. |
| Constraint policy | **Hard feasibility constraints + soft preferences** | Safety/clearance limits cannot be traded away by weights. |
| Initial vehicle model | **Kinematic, simplified** | Assess route feasibility without claiming realistic flight dynamics. |
| Simulation clock | **Fixed timestep** | Reproducible state updates, decoupled from rendering FPS. |
| Grid storage | **Contiguous, data-oriented** | Predictable indexing and cache-friendly baseline. |
| Parallelism | **Single-threaded planning baseline** | Establish correctness and performance before concurrency. |
| Experiments | **Versioned scenarios + repeated measurements** | Reproducibility and honest comparison. |

**Status policy:** These are *selected baseline decisions*, not irreversible facts. Change them only when requirements or measured evidence justify it, and record the change in an ADR.

## 3. System boundaries

```text
                        React + Three.js
                         (Web Interface)
                                |
                           HTTP / JSON
                                |
                       C++ Server Adapter
                                |
                         Application API
                                |
               +----------------+----------------+
               |                |                |
         Planning Engine   Simulation Engine   Experiment Runner
               |                |                |
               +----------------+----------------+
                                |
                    Shared C++ Domain/Core
                 Geometry · Grid · Collision
                                |
                   C++20 Library + CMake
                                |
                  Native CLI / Unit Tests
```

### Non-negotiable dependency rules

1. **Planning, geometry, and collision code must not depend on React, Three.js, HTTP, or browser types.**
2. **The core library must compile and run headlessly.** HTTP is an adapter, not the engine.
3. World/grid coordinates must be converted at explicit boundaries; never mix world units with voxel indices.
4. The visualization is not the source of truth for collision or path validity.
5. The simulation clock and planning benchmarks must not depend on rendering frame rate.
6. Avoid speculative plugin frameworks or premature generalization. Add interfaces where there is a concrete variation point.

## 4. Suggested repository structure

```text
uav-simulator/
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
├── AGENTS.md
├── README.md
├── core/
│   ├── include/uav/core/
│   └── src/                 # geometry, grids, collision, domain types
├── planning/
│   ├── include/uav/planning/
│   └── src/                 # graph traversal, A*, Dijkstra, costs
├── simulation/
│   ├── include/uav/simulation/
│   └── src/                 # kinematics, playback, feasibility checks
├── experiments/
│   ├── scenarios/           # versioned JSON inputs
│   ├── src/                 # runner, measurement, export
│   └── results/             # small summaries; large data ignored
├── apps/
│   ├── cli/                 # headless entry point
│   └── server/              # HTTP/JSON adapter
├── visualization/
│   └── web/                 # React + TypeScript + Three.js
├── tests/
│   ├── core/
│   ├── planning/
│   └── simulation/
└── docs/
    ├── ARCHITECTURE.md
    ├── ROADMAP.md
    ├── EXPERIMENTS.md
    ├── DECISIONS.md
    └── adr/
```

**Implementation note:** This is the target structure, not an instruction to create empty modules on day one. Introduce directories when functionality exists.

## 5. Technical design

### 5.1 Coordinate and environment model

- **Core frame:** right-handed, `X/Y` horizontal and `Z` altitude; distances in **meters**.
- **Rendering:** Three.js uses a different conventional up axis; perform an explicit coordinate transformation in a dedicated adapter.
- **Planning states:** 3D grid indices `(i, j, k)` mapped to world-space voxel centers.
- **Storage:** bounded dense occupancy array, e.g. `std::vector<uint8_t>`; indexing function and bounds checks must be tested.
- **Obstacle geometry:** begin with axis-aligned boxes (AABB); add richer shapes only when experiments require them.
- **Vehicle clearance:** use obstacle inflation or equivalent configuration-space checks. Document the simplified vehicle geometry.
- **Transitions:** 26-connected neighbor candidates, with **full segment collision tests**. Prevent diagonal passage through corners or thin obstacles.
- **Resolution:** configurable and recorded per scenario. Coarser resolution is not assumed equivalent to finer resolution.

### 5.2 Planning algorithms

**Dijkstra:** reference implementation for graphs with nonnegative transition costs.  
**A\*:** priority-queue search using a documented admissible/consistent heuristic under the selected cost model.

The planner result should include:

- Status: `found`, `unreachable`, or `invalid`.
- Path waypoints in world coordinates.
- Total weighted cost and component breakdown.
- Geometric path length and positive ascent.
- Expanded-node count and planning duration.
- Minimum clearance estimate, where computed.

**Correctness requirement:** Under identical graph/cost assumptions and an admissible heuristic, A* must agree with Dijkstra on optimum total cost (allowing a documented numerical tolerance). Different but equal-cost paths are acceptable.

### 5.3 Optimization model

For a path \(P\):

\[
J(P) = w_d D(P) + w_c C_{\text{climb}}(P) + w_r R_{\text{clearance}}(P)
\]

Where:

- \(D(P)\): total Euclidean path length.
- \(C_{\text{climb}}(P)\): cumulative **positive altitude gain**, used as an **energy-related proxy**, not a validated battery model.
- \(R_{\text{clearance}}(P)\): nonnegative penalty for proximity to obstacles beyond mandatory clearance.
- \(w_d,w_c,w_r\): configurable **nonnegative** weights with explicitly documented scaling/units.

**Hard constraints:** collision-free path, bounded world, minimum mandatory clearance, valid start/goal.  
**Soft preferences:** shorter travel, lower ascent proxy, larger optional clearance.

**Heuristic rule:** When each transition costs at least `w_d × segmentLength`, `h(n) = w_d × EuclideanDistance(n, goal)` is a valid lower bound. Re-evaluate heuristic guarantees whenever costs change.

### 5.4 Simulation model

- MVP: follow planned waypoints and display orientation based on direction of travel.
- Use fixed-step updates, initially `Δt = 0.02 s` as a **configurable starting assumption**.
- Record basic speed/climb-rate/acceleration limits and **check** them separately from geometric path validity.
- Do not claim a path is physically flyable merely because it is collision-free.
- Defer full dynamics, controllers, wind, sensor models, and hardware integration.

### 5.5 Visualization and diagnostics

- Orbit/follow cameras; world axes and coordinate readout.
- Render obstacles, start/goal, 3D route, and moving UAV model.
- Display planner selection, cost weights, path metrics, and collision status.
- Optional debug overlays: expanded nodes, route alternatives, clearance, and cost fields.
- Keep visualization updates independent of core benchmark timing.

## 6. Delivery roadmap

### Phase 0 — Engineering baseline (before feature implementation)

**Work:** Initialize CMake/C++20 library and tests; decide exact coordinate conventions, world bounds, grid resolution, obstacle/vehicle dimensions, scenario JSON schema, and minimal application interfaces. Write initial ADRs.

**Exit gate:** Core builds headlessly; one simple scenario can be loaded and its geometry validated; decisions and assumptions are documented.

### MVP — Approximately 4–6 weeks

| Week | Focus | Concrete deliverable | Acceptance gate |
|---|---|---|---|
| 1 | Core foundation + 3D visualization shell | C++20 project, CI-ready tests, Three.js scene, world/grid coordinate adapter | Consistent coordinates; native core builds without UI |
| 2 | Spatial model + collision | Dense 3D grid, AABB obstacles, clearance, 26-neighbor generation, segment validation | No out-of-bounds moves or diagonal corner-cutting |
| 3 | Planning | Dijkstra and A*, path reconstruction, priority queue, correctness tests | Same optimum cost under valid heuristic assumptions |
| 4 | End-to-end application | CLI scenario execution, HTTP adapter, path rendering, simple UAV playback | A scenario runs headlessly and in the UI |
| 5 | Optimization study | Weighted costs, ascent and clearance metrics, over/around canonical scenarios | Route preference changes explainably with parameters |
| 6 | Benchmark + release | Seeded experiments, repeated measurements, reports, README, demo | A reviewer can reproduce reported results |

**Scheduling note:** C++/web integration and collision correctness can consume more time than expected. If the schedule slips, **retain correctness, tests, and reproducibility**; defer interface polish and optional overlays rather than weakening validation.

### Longer-term expansion — 3–4 months

**Phase 2A — Better geometric routes:** Theta* and route smoothing; compare length and collision validity.  
**Phase 2B — Dynamic environments:** repeated A* baseline, then D* Lite if justified by measured replanning cost.  
**Phase 2C — Continuous-space planning:** RRT* and consistent scenario comparison; note different guarantees and representations.  
**Phase 2D — Multi-objective studies:** Pareto-front exploration, sensitivity analysis, alternative route sets.  
**Phase 2E — Motion-constrained planning:** trajectory generation and kinematic feasibility; optionally stronger vehicle models.  
**Phase 2F — Scaling:** sparse grids/octrees, profiling, memory-layout tuning, selected parallelism.

Each extension requires an explicit hypothesis, measurable success criteria, and an ADR if it changes architecture.

## 7. Experiment and validation plan

### Canonical experiments

1. **Over vs. around:** increase obstacle height while keeping other factors fixed. Identify the crossover in preferred route.
2. **Cost-weight sensitivity:** vary climb and clearance weights; explain why routes switch.
3. **A* vs. Dijkstra:** compare optimum cost, expanded nodes, time, and memory on identical graphs.
4. **Resolution scaling:** vary voxel size and measure search effort, memory, route cost, and collision validity.
5. **Obstacle density:** use seeded scenarios and evaluate feasibility, runtime, and failure modes.

### Required per-run metadata

- Scenario ID and random seed; git commit; planner and heuristic versions.
- World dimensions, voxel resolution, obstacle definitions, clearance, connectivity.
- Cost coefficients and units; algorithm parameters.
- Success/failure status, weighted cost, component costs, length, ascent, clearance.
- Expanded nodes, planning time, peak memory where available.
- Runtime, compiler/build mode, machine information, and number of repetitions.

### Measurement rules

- Exclude rendering and network time from **core planner** timing.
- Separate environment construction time from search time.
- Benchmark optimized builds and disclose configuration.
- Use repeated runs; report median and spread, not a cherry-picked minimum.
- Keep raw experiment outputs or sufficient reproducibility artifacts.
- State limitations of proxy costs and discretization explicitly.

### Core test cases

- Empty world, known shortest path.
- Blocked start/goal, unreachable goal, out-of-bounds positions.
- Vertical moves and all diagonal transition types.
- Thin obstacle / diagonal corner-cutting prevention.
- Minimum clearance handling and obstacle inflation.
- Dijkstra/A* optimum-cost agreement where guarantees apply.
- Nonnegative edge-cost validation and stable path reconstruction.
- Coordinate transform round trips and grid-index conversion.

## 8. Risks and mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| 3D grid state explosion | Memory/time growth | Bounded maps, configurable resolution, benchmark early; sparse storage only when justified |
| Collision false negatives | Invalid paths | Full edge checks, clearance model, adversarial geometry tests |
| C++/web integration overhead | Schedule risk | Native CLI first; minimal HTTP adapter; do not make core depend on server |
| Physically implausible paths | Misleading simulation | Label geometric paths; add explicit kinematic feasibility checks |
| Incorrect heuristic | Loss of A* optimality | Document lower-bound assumptions; compare against Dijkstra |
| Cost weights lack meaning | Weak conclusions | Units/scaling, component reporting, sensitivity sweeps |
| Scope creep | Incomplete MVP | Milestone gates, backlog discipline, no premature dynamics/ML |
| Benchmark noise | Misleading comparisons | Headless optimized builds, repeats, fixed scenarios, hardware metadata |

## 9. AI-assisted development workflow

### Roles

- **Project owner:** final technical decisions, learning, algorithm comprehension, code review, experiment interpretation.
- **ChatGPT:** architecture trade-offs, mathematics, experiment design, documentation and review.
- **Codex:** bounded implementation tasks, refactoring, tests, build/CI work, benchmark tooling.

### Operating loop

1. Define one scoped task and acceptance tests.
2. Discuss mathematics/design before complex algorithm work.
3. Record accepted decisions in `docs/DECISIONS.md` or an ADR.
4. Give Codex a bounded implementation request with files and non-goals.
5. Review changes, run tests, inspect benchmark evidence.
6. Merge/commit and update roadmap plus documentation.

### Agent rules (`AGENTS.md`)

- Read this plan and relevant ADRs before substantial work.
- Do not silently change technology or expand scope.
- Keep core independent of UI/network; preserve headless execution.
- Include tests and explain algorithm assumptions.
- Never invent measurements or claim flight safety.
- Summarize changed files, test commands/results, and unresolved risks.

## 10. Definition of done

A feature is **done** only when:

- Its behavior and constraints are documented.
- It has appropriate automated tests and passes relevant existing tests.
- It respects module boundaries and coordinate conventions.
- It exposes useful error handling and measurable outputs where applicable.
- Any significant trade-off or change has an ADR entry.
- The project owner can explain its algorithmic and engineering rationale.

**MVP is complete** when a third party can clone the project, build the native core, run the canonical scenarios, inspect 3D paths in the UI, and reproduce at least one documented comparison of over-vs-around choices.

## 11. Explicitly deferred decisions

The following are **not** required to be finalized now: sparse grid versus octree; RRT*/Theta* details; dynamic replanning strategy; trajectory smoothing method; advanced physics/controller choice; WebAssembly integration; GPU acceleration; production deployment architecture. These decisions should be triggered by specific experiments or new requirements.

## 12. Immediate next actions

1. Create the repository and commit this plan as `docs/PROJECT_PLAN.md`.
2. Add `AGENTS.md` with the architectural invariants from §9.
3. Record accepted baseline decisions in ADRs (C++20 core, rendering boundary, 3D grid, planning and cost model).
4. Build a minimal headless C++20/CMake/GoogleTest skeleton.
5. Implement and test world-to-grid mapping **before** writing A*.
6. Define the first two canonical over/around scenarios and their expected qualitative behavior.

---

**Guiding principle:** *Build a trustworthy planning engine first; use the simulator to explain its decisions, and experiments to test its claims.*
