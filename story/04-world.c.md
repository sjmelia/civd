# 4. Generating the world

Complete file:
[`src/WORLDGEN.C`](https://github.com/sjmelia/civd/blob/main/src/WORLDGEN.C).

`GAME.C` calls `make_world` after choosing the world parameters. `WORLDGEN.C`
contains that function and the helpers for land growth, rivers, regions, site
scoring and the creation presentation. It also contains a later operation on
the same terrain: global warming.

This module is overlay 7. The build gives it a separate overlay code segment.
Its opening declarations include both generation state and presentation data.

{{#include src/WORLDGEN.C}}

## Generation stages

`make_world` selects a working graphics page. For Earth it loads `map.pic`.
Otherwise it grows land until `land_count` reaches a threshold controlled by
the land-mass parameter. The expression is equivalent to `640 + 320 *
custom_params[0]`; it controls this stage, not the final count of each terrain
type.

The following passes adjust neighbouring cells, assign terrain using latitude
and temperature, then change it through rainfall passes in both horizontal
directions. The age parameter controls another sequence of terrain changes.
River generation follows.

Calls to `story_step` appear between these operations. The creation
presentation advances during generation rather than running as a separate
sequence after the map is complete. Both paths then number the regions and
finish the presentation.

{{#include src/WORLDGEN.C | "void page_zero(void)"}}

## Rivers and land growth

`make_rivers` saves a working area with `blit` before attempting a river. It
traces a route, tests its endpoint and length, then either keeps the result or
restores the saved area. The second blit is the rejection path.

`grow_land` uses another working area. `walk_land` marks cells during a
bounded random walk; `grow_land` applies those marks to the accumulating map.
The random choices call `rand_scaled`, defined in `CORE.C`. Their order
affects the generated result, so replacing individual choices changes more
than the appearance of one local step.

{{#include src/WORLDGEN.C | "int land_corner(int x, int y)"}}

## Regions and settlement sites

`number_regions` makes separate passes over land and sea. It labels connected
cells, merges labels when they meet, and ranks the resulting regions. It
writes region values into the working page at a vertical offset of fifty
cells.

The function then calls `link_land`, `link_seas` and `score_sites`. Site
scoring combines food, trade and shield values across the city radius. Nearby
cells receive extra weight, and the result is reduced to a small stored value.
These scores describe potential sites for later use; this routine does not
itself found cities.

{{#include src/WORLDGEN.C | "void number_regions(void)"}}

## Connections and map access

The connection routines work over a coarser 20 by 13 grid. Their arrays and
neighbour tests supply information for movement across the generated
geography. Keeping them in the complete file shows the processing that follows
the visible terrain changes.

The accessors near the end of this section clarify the storage.
`gen_terrain_at` translates a working-page value through `terrain_map`.
`gen_region_at` reads from the same page fifty cells lower. The graphics page
carries several kinds of map data.

{{#include src/WORLDGEN.C | "void link_land(int show)"}}

## The creation presentation

`story_step` opens `story.txt`, tracks its position in the presentation, loads
pictures and controls palette cycles and delays. It returns control to
generation between updates. Its mode variables explain why calls appear
throughout the earlier loops.

The picture and timing arrays remain at the end of the file. Like the rule
tables in `GAME.C`, these initialised values form part of the translation unit
being reproduced.

{{#include src/WORLDGEN.C | "/* Initialized story state follows literals emitted by overlay7_story. */"}}

## Later terrain changes

`global_warming` revisits the map during play. It considers particular terrain
types, counts neighbouring ocean cells and uses the severity argument in its
tests. When it changes a tile, it updates visibility state and redraws the
location if the current player can see it.

This function gives the file a role after initial generation. `TURN.C` calls
it through the event processing that follows each round of civilisation
activity.

{{#include src/WORLDGEN.C | "void global_warming(int severity)"}}

[← Previous](03-game.c.html) · [Contents](00-opening.c.html) · [Next →](05-turn.c.html)
