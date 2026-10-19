# World generation

`make_world()` in `WORLDGEN.C` is overlay 7. It prepares terrain, regions,
connections and potential city sites while advancing the creation pictures
and text through calls to `story_step()`.

`custom_params[0..3]` hold land mass, temperature, climate and age. If
`beginner_help` selects Earth, the routine loads `map.pic` instead of
generating terrain, then runs the common region and connection stages.

## Land and climate

`grow_land()` and `walk_land()` build land through bounded random walks.
The initial growth stage stops at `640 + 320 * custom_params[0]` land
cells. Border cleanup and later terrain changes follow, so this threshold
is not a promise about the final land count.

A pass removes diagonal checkerboard contacts. Land is then assigned
terrain using `abs(29 - (rand_scaled(8) + y)) + 1 - custom_params[1]`.
Dividing this value by six selects Desert, Plains, Tundra or Arctic.

Rainfall passes scan each row in both directions. Ocean cells build a
rain counter; land consumes it and can change terrain. Mountains consume
extra rain. The climate parameter affects both the amount collected and
random depletion. Age controls a further `800 + 800 * custom_params[3]`
terrain-change iterations.

## Rivers

`make_rivers()` saves a working area before attempting a river. It traces
a route and tests the endpoint and length. Accepted routes remain; rejected
ones restore the saved area. The code uses terrain tests and random choices,
rather than an elevation field with a general downhill-flow calculation.

## Regions

`number_regions()` labels connected land and sea separately, merging labels
as the scan finds connections. It then ranks regions by size. The fourteen
largest receive IDs 1–14, with 1 the largest; remaining regions share ID 15.
`continent_span[0]` holds land sizes and `[1]` sea sizes. Entry 15 is set
to 1, rather than a combined area for all remaining regions.

`link_land()` and `link_seas()` build connections on a coarser 20-by-13
grid for [pathfinding](../entities/pathfinding.md).

## City sites

`score_sites()` scores Plains, Grassland and River locations. It combines
food, shields and trade across the city radius, with extra weight for
nearby tiles and adjustments for resources and potential improvements.
The result is reduced to a small value stored at x+80 in the working page.
`ai_zone_places` counts candidate sites by region.

This supplies later settlement decisions. It does not directly choose and
found eight starting cities.
