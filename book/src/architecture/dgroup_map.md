# Shared Game Data

Civilization's resident modules and overlays share a common data area called DGROUP. It holds rule tables, civilization state, entity arrays, strings, scratch buffers and the execution stack. Changing the active overlay leaves this state available.

## Principal Record Arrays

Offsets below are relative to DGROUP.

| Offset | Array | Records | Record size |
| --- | --- | --- | --- |
| `0282` | `terrains` | 12 | 19 bytes |
| `112A` | `utypes` | 28 | 34 bytes |
| `70EC` | `townbuf` | 128 | 28 bytes |
| `81D4` | `armydata` | 8 × 128 | 12 bytes |

The [record declarations](../reference/records.md) show their fields. `src/GAME.C` also defines the [46-entry production table](../city/improvements_wonders.md), containing city improvements, spaceship parts and wonders.

Civilization-wide variables track treasury, science and tax rates, technological knowledge and diplomatic relations. The [save transfer sequence](../formats/sve_offset_table.md) shows which state is written to disk and in what order.

Larger arrays, including map visibility, power history and the city-name pool, use separate storage. Tile attributes are accessed through [screen page 2](../map/memory_layout.md).
