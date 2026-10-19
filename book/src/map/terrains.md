# Terrain tables

`GAME.C` defines twelve base terrains and twelve corresponding special
resource records. Each record is 19 bytes. The special record supplies a
replacement yield, rather than a bonus to add to the base terrain.

```c
{{#include ../../../src/CIV.H:5:10}}
```

```c
{{#include ../../../src/GAME.C:61:86}}
```

The fields after the name are movement cost, defence in halves, food,
shields, trade, and two further bytes. A defence value of 2 means the
normal factor; 3 means one and a half, 4 means double and 6 means triple.
Ships and aircraft have separate combat rules; see [Combat](../entities/combat.md).

## Special resources

`patterned_cell()` in `MAP.C` derives resource positions from coordinates
and `map_rng_state`. It selects one position per four-by-four block, except
on the two rows nearest each pole. The terrain at that position determines
which special record is used: Desert gives Oasis, Hills Coal, Mountains
Gold, Ocean Fish, and so on.

Grassland and River shields also depend on `(7 * x + 11 * y) & 2` in
`tile_yield()`. The base table alone therefore does not describe every tile.
