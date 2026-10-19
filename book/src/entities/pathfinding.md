# Coarse and Local Pathfinding (`PATH.C`)

`src/PATH.C` contains six functions: `unit_goal`, `path_coarse`, `path_select`, `path_search`, `path_probe`, and `wrap_x`. Its code combines coarse connectivity, a cached local cost search, and directional fallback scoring.

`path_coarse` traverses 20×13 land or sea connectivity cells using a circular queue. It selects an intermediate target near a 4×4 cell center. `path_select` tries nearby connected cells when the initial coarse cell is unavailable.

`path_search` clears a 256-byte local cost grid and propagates costs outward from the target using the shared queue. It restricts coordinates to a window within eight tiles of the target, relaxes cheaper arrivals, and limits processing with `tail6 < 225`. The next direction is selected from neighboring cells using stored costs, with stack and distance tie breakers. Queue indices wrap through unsigned-byte conversion. This is not an unbounded optimal-path guarantee.

`unit_goal` tries local and coarse searches before scoring adjacent directions. The fallback includes distance, movement cost and a penalty for changing direction. `path_probe` reuses the local search with temporary probe state. `wrap_x` adds or subtracts 80 once, preserving the original behavior rather than providing arbitrary modulo normalization.

The [source declarations](../reference/resident_api.md) list shared entry points. Shared scratch storage remains external to this translation unit.
