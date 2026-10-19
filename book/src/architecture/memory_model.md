# Game Memory Layout

Civilization keeps the main game loop, unit operations, city simulation and tactical AI resident in memory. Screens and less frequently used operations occupy [23 overlays](overlays.md), which share a 9,648-byte code window. Loading another overlay replaces the code in that window.

Shared state remains available across overlay changes. [City and unit records](../reference/records.md), civilization finances, diplomacy and rule tables use the common data area, DGROUP. Larger arrays such as `map_visible_to`, `power_history` and `place_name` occupy separate data storage. The execution stack is 2,048 bytes.

Tile attributes also use the [page 2 map layout](../map/memory_layout.md), accessed through the game's graphics and map routines. This storage links the map engine to the screen-page abstraction.

## Executable Layout

These addresses are relative to the executable's unrelocated load image.

| Area | Start | End | Role |
| --- | --- | --- | --- |
| Resident code | `00000` | `23740` | Main simulation and engine routines |
| Overlay code window | `23740` | `25CF0` | One active overlay body |
| Separate data storage | `25CF0` | `2B010` | Larger arrays and tables |
| Shared data and stack | `2B010` | `3A0E0` | DGROUP |

The [main module](../introduction.md) initializes the engine and enters the turn loop. The [overlay catalog](overlays.md) identifies the modules loaded for menus, reports, world generation and other operations.
