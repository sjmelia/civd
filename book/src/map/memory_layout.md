# Map storage

The game uses graphics page 2 as a map database. Each map tile has a
position in several 80-by-50 areas of the 320-by-200 page. Values are read
with `help_probe()` and written with `map_claim()`.

| Page coordinates for tile (x, y) | Contents |
| --- | --- |
| `(x, y)` | Encoded terrain, translated through `terrain_map[16]` |
| `(x, y + 50)` | Ranked land or sea region |
| `(x + 80, y)` | Site score or territory marker |
| `(x + 80, y + 50)` | Saved site score |
| `(x + 160, y)` | Civilization in low three bits; bit 3 means units present |
| `(x + 240, y)` | Overview-map colour |
| `(x, y + 100)` | Low nibble of current tile flags |
| `(x, y + 150)` | High nibble of current tile flags |
| `(x + 80, y + 100)` | Low nibble of remembered flags |
| `(x + 80, y + 150)` | High nibble of remembered flags |

`terrain_at()` translates the encoded terrain rather than returning the
page byte directly. `map_owner()` returns −1 if the unit-present bit is
clear; `map_city()` reads the low three bits regardless. `map_stack()`
then searches the owner's unit records for a unit at that location.

## Tile flags

`map_flags()` combines the nibbles at y+100 and y+150. These describe
improvements and tile state:

| Mask | Meaning |
| ---: | --- |
| `0x01` | City |
| `0x02` | Irrigation |
| `0x04` | Mine |
| `0x08` | Road |
| `0x10` | Railroad |
| `0x20` | Fortress |
| `0x40` | Pollution |

`flag_tile()` sets flags; passing zero clears them all. `unflag()` clears
selected bits. `known_flags()` reads the remembered copy, and `map_mark()`
refreshes that copy from the current flags.

## Exploration

Exploration uses the separate far array `map_visible_to[4000]`, indexed
as `x * 50 + y`. Bit n marks exploration by civilization n. It is separate
from tile flags and the unit-present bit. The Barbarian bit also records
visited huts. Unit visibility has another mask in each unit record.
