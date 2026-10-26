# Starting conditions: units and technologies

For DOS version 474.05, “rolling a start” combines random placement with
compensation for the resulting geography. Every active civilization starts
with one Settlers unit. A weaker starting site can receive a second Settlers
unit and free advances. The number of bonuses follows a score comparison;
the particular advances are chosen randomly with civilization-dependent weights.

## Setup order and baseline

[`new_game()`](https://github.com/sjmelia/civd/blob/main/src/NEWGAME.C)
sets the year to 4000 BC, clears technology ownership and calls `init_civ()`
for slots 0–7, then calls `rate_starts()` once all starting units exist.
Slot 0 is barbarians; the rating pass considers slots 1–7 with a live unit
in position 0, covering both the human and computer players.

`init_civ()` clears the civilization's units and known-advance bits, sets
Despotism and zero money, and creates one unit of type 0 (Settlers) at the
chosen location. It does not create an initial city or defender. After rating,
`new_game()` sets the human's science and tax sliders to 5 each and gives
50 gold on Chieftain. These are separate from the site compensation.
The assignment `tech_count[civ] = 1` is bookkeeping, not a grant of an advance.
The introductory message always appends “and Roads”; Roads is not a rolled
technology in the [advance table](../reference/enum_tech.md).

## Choosing the location

On a generated map, `init_civ()` draws x in 8–71 and y in 8–41. It rejects
Ocean, a hut, or a candidate failing any of these tests (integer division):

| Test | Required minimum on attempt `trial` |
| --- | --- |
| Stored city-site score at x+80 | `12 - trial / 32` |
| Distance from the nearest city or earlier starting unit | `10 - trial / 64` |
| Candidate sites in the region (`ai_zone_places`) | `32 - turn_tick / 16 - trial / 64` |

The attempt counter increments before these tests. At a new game
`turn_tick` is zero. Distance uses `map_dist()`, including horizontal wrapping;
see [map coordinates](geometry.md). Earlier starting units are checked in
slot order, so placement is sequential. With no nearby city the initial
distance value is 30. Thresholds relax as attempts accumulate. A computer
civilization reaching 2000 attempts can be removed from the active set;
the human's initial search does not take the over-2000 break.

On Earth (`beginner_help`), the routine still runs the search, then replaces
the result with the tribe's coordinates in `start_pos` and resets `trial`.
Thus Earth uses fixed tribe locations but still runs the common rating and
bonus pass. See [world generation](worldgen.md) for the site-score producer.

## Rating the starts

`rate_starts()` adds the following points for each starting unit. Higher is
better. Only other civilizations on the **same region** count as neighbours.

| Component | Points |
| --- | --- |
| Nearest neighbour distance below 10 | 0 |
| Distance 10–19 | 1 |
| Distance 20 or more | 2 |
| No other civilization on the same region | 4 |
| More than one River tile in the local nine tiles | 2 |
| Otherwise, at least three Grassland tiles there | 1 |
| Otherwise | 0 |
| Region area below 100 tiles | 0 |
| Region area 100–199 tiles | 1 |
| Region area at least 200 tiles | 2 |

The local nine tiles are the starting tile plus its eight neighbours,
`spiral_x/y[0..8]`. River and Grassland points are alternatives. Ocean
neighbours change the “Coastal” description but award no points. Region area
comes from `map_region()`; it uses the stored region size, including the
small-region fallback described in [world generation](worldgen.md#regions).

Let `gap` be the best score among the starts minus this civilization's score.
If `gap > 3`, give one extra Settlers unit on the same tile and add three to
its score. Then give one advance per remaining point of gap.

| Original gap | Total starting Settlers | Free advances |
| --- | --- | --- |
| 0 | 1 | 0 |
| 1–3 | 1 | 1–3 |
| 4 | 2 | 1 |
| 5 | 2 | 2 |
| 6–8 | 2 | 3–5 |

For example, if the best site scores 7 and yours scores 3, your gap of 4
buys a second Settlers and one advance. A score of 4 instead buys three
advances and no second Settlers. Improving a site can therefore change the
*kind* of compensation. If all sites have equal scores, none gets a bonus,
even if their absolute quality is poor. The maximum score is 8.

## Choosing each free advance

For every remaining point, `rate_starts()` calls `reduce_attitude(p, 0)` in
[`ECONOMY.C`](https://github.com/sjmelia/civd/blob/main/src/ECONOMY.C).
Despite its name, this function selects and grants an advance. At turn zero
it takes the automatic selection path for the human as well as the AI.

It scans technology IDs 0–71, considering only unknown advances whose two
prerequisites are known. `has_tech(civ, -1)` is true, so a missing prerequisite
requires nothing. Each eligible advance gets this random rating:

```text
weight = (signed char) advance_attrs[id].a
         + civt[civ_tribe_of[civ]].civilized * advance_attrs[id].b
rating = rand_scaled(weight * 4)
```

The greatest rating wins; ties keep the earlier ID because replacement uses
strict `>`. This is a competition between random ratings, not a uniform draw
or a probability proportional to the weight.

Only one personality trait takes part: the tribe's `civilized` value (1
Civilized, 0 neither, −1 Militaristic; see
[Leaders and Personalities](../ai/leader_personalities.md)) changes
the weights, and so which advances are likely, for the human's tribe as well
as the AI's. It does not change how many advances a civilization receives,
which depends only on the site comparison above, and it does not supply a
fixed list of starting advances. The Aggressive and Expansionistic traits
play no part.
`selected_index` is set to -1 before setup, so the human's research-target
exclusion does not remove any candidate here.

`give_advance()` immediately sets the known bit. Later bonus draws cannot
repeat it and can select technologies it unlocks: starting advances can form
a prerequisite chain. At turn zero this function returns before its normal
technology-counter increment and discovery presentation.

[`CORE.C`](https://github.com/sjmelia/civd/blob/main/src/CORE.C) implements
`rand_scaled(n)` as `((long)n * rand()) >> 15`. Placement and advance choice
consume the shared random sequence. A tribe or difficulty setting alone does
not determine the package. Difficulty is absent from the rating and bonus-count
formula; map generation, competition and placement affect the comparison.

## New games versus replacement civilizations

`init_civ()` also serves `kill_civ()` during play. With nonzero `turn_tick`,
it can switch to the alternative tribe, restrict placement by existing cities
and region occupancy, and give eligible technologies known by the human a
one-in-two selection test. That path is separate from new-game compensation:
`kill_civ()` does not call `rate_starts()`.

For a source-led reading, see the Docco chapter
[NEWGAME.C: rolling starting conditions](https://sjmelia.github.io/civd/story/04-world_starting.c.html).
