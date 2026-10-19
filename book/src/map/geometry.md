# Map coordinates

The world is 80 tiles wide and 50 tiles high. Coordinates run from 0–79
and 0–49. Horizontal movement wraps; vertical movement does not.
`wrap_x()` in `PATH.C` adds or subtracts 80 once, so callers must keep
coordinates within one map width of the valid range. `map_in_bounds()`
in `MAP.C` checks both axes.

## Distance

`map_dist(x1, x2, y1, y2)` in `CORE.C` takes the shortest horizontal
difference across the wrap seam. It then calls `half_dist()`:

```text
dx = min(abs(x1 - x2), 80 - abs(x1 - x2))
dy = abs(y1 - y2)
distance = max(dx, dy) + min(dx, dy) / 2
```

Division truncates. An orthogonal or diagonal neighbour is at distance 1;
two diagonal steps give distance 3. This is a scoring measure, not a count
of movement points. The separate `distance_twice()` helper returns
`2 * max(dx, dy) + min(dx, dy)` without the division.

## Neighbours and city radius

`spiral_x` and `spiral_y` provide offsets for searches around a tile.
Directions 1–8 start north and proceed clockwise. City workers use the
separate `radius_dx[21]` and `radius_dy[21]` tables. Their footprint is a
five-by-five square without its four corners. Tile 20 is the centre.

## Display

`reveal()` in `MAP.C` draws a 15-by-12 tile viewport, with 16-by-16 pixel
tiles beginning at screen position (80, 8). The left panel includes an
80-by-50 overview. Display coordinates and world coordinates are separate;
`map_x0` and `map_y0` place the viewport on the world.
