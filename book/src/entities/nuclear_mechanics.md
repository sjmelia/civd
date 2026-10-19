# Nuclear Weapons

A Nuclear unit (type 25) can be built once anyone has completed the
Manhattan Project and its owner knows Nuclear Fission. It is an aircraft
with a fuel of one turn and 16 moves. It explodes when it enters a foreign
city (the move handler removes it and calls `atom(owner, x, y)` in
`BATTLE.C`) or attacks a unit (`battle()` calls `atom()` on the
defender's tile).

## The blast

`atom()`:

1. reports "… use nuclear weapons!" unless the human launched it;
2. plays the explosion (`nuke_animation()` in `DIPLOMAT.C`);
3. visits the target tile and its eight neighbours. On each tile with
   units, all of them (up to ten, see [Stacks](unit_stacking.md)) are
   destroyed and counted as losses. This includes the attacker's own units,
   so a Nuclear unit attacking from a neighbouring tile destroys itself. A
   city tile with SDI Defense reports "SDI protects …" and keeps its units;
4. sets `0x088` in `treaties[victim][attacker]` for each civilization that
   loses units: a grievance and the nuclear flag. The flag allows later
   [AI nuclear strikes](../ai/unit_orders.md#special-units) against the
   attacker;
5. calls `nuke_fallout()`.

`nuke_fallout()` visits the same nine tiles. Every city loses half its
size, rounded down (`size -= size / 2`). Each land tile without a city is
polluted two times in three. SDI Defense does not prevent either effect.

## Pollution and global warming

`add_pollution()` in `CITYMGT.C` sets map flag `0x40` and increments the
global count `polluted`. Each polluted tile costs ten points of score, and
a polluted tile's yields are halved, rounded up
([Tile Yields](../map/tile_yields.md)). Settlers clean a tile in four
turns.

Each turn `ai_event()` in `TURN.C` moves `warm_levels` one step towards
`polluted * 2 - warming * 4`. When it reaches 12 with more than six
polluted tiles, a news item reports "Scientists alarmed by rising
temperatures. Global Warming feared!" Above 16, `global_warming(warming++)`
in `WORLDGEN.C` changes the climate and the level returns to 0. Each
previous warming lowers the target by four, so the next one needs two
more polluted tiles.

`global_warming(severity)` considers every Desert, Plains, Grassland and
Forest tile:

- A tile with at least `7 - severity` ocean neighbours floods. Forest
  becomes Jungle and the others Swamp, and irrigation and mines are lost.
- Otherwise, one tile in eight (chosen by `(x * 11 + y * 13) & 7 ==
  severity`) dries. Desert and Plains become Desert, Grassland and Forest
  become Plains.

The message reads "Global temperature rises! Icecaps melt. Severe Drought."
