# Unit Orders and Attack Evaluation

When `ai_turn()` in `CONTROL.C` reaches a computer player's unit, it calls
`ai_order(player, unit)`. The result is the command a human would type, so
the AI's units go through the same command handlers as the human's. Any
non-zero result also clears the unit's destination. After five orders in
one turn the unit is made to wait.

| Result | Command |
| --- | --- |
| `0` | Continue towards `goto_x`, `goto_y` |
| `1`–`8` | Step in direction *n* of `spiral_x` / `spiral_y` |
| `' '` | Wait for the next turn |
| `'b'` | Found a city, or join the city the unit is in |
| `'f'` | Fortify |
| `'h'` | Return to base (aircraft) |
| `'i'`, `'m'`, `'r'` | Irrigate, mine, build a road or railroad |
| `'p'` | Clean up pollution |
| `'P'` | Pillage |
| `'u'` | Unload a ship |

Barbarians (civilization 0) use a separate routine, `unit_ai_move()`,
described at the end of this chapter.

Distances quoted as `map_dist()` or `town_gap` use the game's
[distance score](../map/geometry.md#distance), which includes half the
shorter coordinate difference. They are not exact travel costs.

## Special units

**Bombers** return to base (`'h'`) when out of fuel. **Fighters** without a
destination also return when they have used half their movement. Otherwise
they chase the nearest human Bomber they can see. If the Bomber is out of
reach, the Fighter flies to the nearest of its own cities within range of
it.

**Nuclear units** pick a city to destroy. A candidate city has no SDI
Defense and is larger than size 4. Its owner must not be at peace with the
AI and must have been marked for nuclear exchange (`0x080`). In addition,
the AI must be weaker than the owner, hold a grievance, or face an owner
with no nuclear weapons that it does not outclass two to one. The score is
half the city's size, plus one for each tile of its 3 × 3 block occupied
by units of such an enemy, minus two for each tile with the AI's own units
and 99 for each tile with anyone else's. The AI needs one of its own cities at `map_dist() <= 16` from the
target. If the best score is at least 10 and the AI has more than one
Nuclear unit, the unit is moved directly onto a free tile beside the
target and attacks it. A strike on the human sets `last_met_turn` to −2,
which lowers the price of the next audience.

**Diplomats** without a destination head for the nearest city if it is
foreign, and are disbanded inside the AI's own territory.

## City sites and huts

[World generation](../map/worldgen.md) stores a
site value of 8–15 for every possible city tile in page 2 at `(x + 80, y)`
(0 elsewhere). Settlers, attackers and defenders on a tile with a site
value look at the eight neighbours. A neighbour with a hut attracts any
unit except Settlers. If the best value is at least 8, at least
`15 - distance to the nearest city`, and no enemy is adjacent, then:

- Settlers found a city (`'b'`), or step towards a better neighbour first;
- other units record the site (`site_add()`, kind 0, priority 2) for
  settlers to find later.

On the first turn of an EARTH game, the threshold is treated as met, so
settlers placed at their tribe's historical start found a city at once.
(The flag for an EARTH game is named `beginner_help` in the source.) Above
Chieftain, an AI settler near a human city may also found a city
there when the AI knows fewer advances than the human. This uses the
unmodified copy of the site value at `(x + 80, y + 50)`, with a threshold
of 9 and `14 - distance`.

## Defenders

A defender alone in a city fortifies. In a larger stack, the strongest
defender also fortifies. The city needs `size / divisor + 1` defence,
where the divisor is 4 (3 if enemies are adjacent), one less for the
capital and one more on a continent in plan 0. While the stack's defence
is no more than that, every defender fortifies, and while it is less, the
city is listed as a defence target. Every eighth turn a surplus defender's
fortification is cleared so that it can be sent elsewhere.

Away from a city, a defender with no destination fortifies when it is
within three tiles of one of its own cities with no other unit beside it,
or when enemies block it. An attacker alone in a city waits there and
lists the city as a defence target.

## Ships

A transport (role 5) or warship (role 3) first counts the land units it
carries. A transport with too little cargo heads for one of the AI's
coastal cities on a continent with at least 16 defence. It prefers
continents planned for expansion and avoids ports where it is not needed.
Arriving, it adds settle, defend and attack targets there so that units
come to it.

A loaded transport looks for an adjacent landing site: land with no foreign
units, on a continent where its cargo is useful (one with little defence
and no expansion plan, a continent planned for the cargo's role, or the
destination). The continent must have at least five city sites. If
possible it first moves along the coast to the beach with the most
unclaimed land (`ocean_edge()`), then unloads (`'u'`). On a continent
planned for attack it also lists the landing site as a naval target.

A ship with no destination then chooses one (Triremes excepted):

1. an enemy city, taking the nearest of every eighth city (an offset that
   changes each turn). While the human ranks first, the only enemy cities
   considered are the human's;
2. failing that, a distant city of its own on another continent, preferring
   one whose plan matches the units' roles;
3. failing that, a random land tile on a continent with none of its cities.

When the last defender of a human city is destroyed, `city_captured()`
sends every loaded transport at sea at a `map_dist()` less than three times the ship's movement allowance, belonging
to a civilization without a treaty with the human, towards that city.

## Settlers at work

A settler outside its city's square, at `map_dist() <= 2` from the AI's own city,
improves terrain when the rule option allows (always, in practice):

- `work_tile()` chooses irrigation (`'i'`) or a mine (`'m'`) when the
  terrain rule gives a worthwhile bonus and water is available;
- road (`'r'`) on irrigated or mined grassland, plains or desert;
- railroad, once the AI knows the RailRoad advance, on a tile yielding at
  least two shields (one if it already has a road).

A settler on polluted land cleans it (`'p'`). Next comes the expansion
test from the leader's `expansionist` trait (see
[Leaders and Personalities](leader_personalities.md#where-the-traits-are-read)):
a settler with a destination keeps going while the continent has room.

A settler beside its own city then looks for a neighbouring tile to work,
and otherwise builds roads that join existing ones. Rivers need Bridge
Building. With nothing to improve, it explores a random nearby 4 × 4 block
holding none of its cities or settlers. If it finds none and the continent
already has more than one town per eight sites, a settler in a city of
size under 10 joins it (`'b'`), and one elsewhere goes home.

## Pillage

A slow unit (fewer than two moves) near a human city (`town_gap <= 3`),
when the AI has no treaty with the human, pillages an irrigated or mined
tile it stands on (`'P'`).

## Choosing a step

When no rule above has given an order, `ai_order()` scores the eight
neighbouring tiles and takes the best:

- A base of `rand_scaled(5)`. For a unit on land that no one can see, the
  base is `rand_scaled(3)` minus twice the move cost (attackers) or minus
  the terrain defence (others).
- A tile of a friendly stack scores by the stack's mix: attackers like
  stacks that need attack, defenders stacks that need defence. Other tiles
  add four times the terrain defence.
- An undefended foreign city scores 999 and a hut adds 20.
- When the unit is free to move, it gains 8 for an unexplored 4 × 4 block
  in that direction, 2 for each unseen tile around the point four steps
  away, and loses 2 for each occupied one. Settlers add the food of those
  tiles.
- Turning is penalized by `2 * d * d`, where `d` is the change of direction
  from the last step (`f8`).

### Attack odds

A neighbouring enemy stack at war (Settlers never attack) is scored with
`battle(..., 0)`, which returns the attack-to-defence ratio without
fighting (see [Combat](../entities/combat.md)):

```text
odds = battle() * (defenders in the stack + 1) / attacker cost
odds *= 3   against a city
odds *= 3   for an attacker on a continent planned for attack
odds *= 2   for an attacker in a stack with more than twice its attack
```

The unit attacks if `odds >= 6`, adding `4 * odds` to the tile's score.
Otherwise the tile scores −999. Then, a land attacker on an attack
continent facing a lone fortified or veteran defender on hills or
mountains outside a city may, one time in four, call `buy_u()`, the
Diplomat's bribery routine. With fewer than three movement thirds left,
the unit does not attack at all.

Stacks of a civilization at peace are not attacked. Units other than
Fighters ignore enemy aircraft outside cities.

## Barbarians

`unit_ai_move()` handles civilization 0:

- Barbarian units in the polar rows are disbanded.
- An empty barbarian ship is disbanded. A loaded ship attacks an adjacent
  unit of another civilization. Otherwise it lands beside a free,
  unclaimed land tile within a `town_gap` of 8 from a city. If the human can see
  the landing near one of its cities, it reports "Barbarian raiding party
  lands near …! Citizens are alarmed." Otherwise the ship sails for the
  city with the highest `(unrest_chance + 50) / (distance + 1)`.
- A land unit alone in a city, or a defender there, fortifies. Every fourth
  turn a unit is disbanded if it is more than eight tiles from the nearest
  city or on another continent.
- Units march towards the nearest human city on their continent, unless
  that civilization has fewer than two cities and less than 100 gold.
  They attack any adjacent city that is not theirs.
- The barbarian leader (a Diplomat) stays with its troops, keeps away from
  enemies and is disbanded when left alone too far from a city. Capturing
  it pays a ransom of 100 gold (`BATTLE.C`).
