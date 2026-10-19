# Movement

`ai_turn(civ)` in `CONTROL.C` moves a civilization's units. It is used for
every civilization. For the human it reads commands from the keyboard,
mouse or a unit's standing orders. For the computer it takes them from
[`ai_order()`](../ai/unit_orders.md). Directions 1–8 index the first ring
of `spiral_x` and `spiral_y`, starting north and turning clockwise.

## Movement points

Movement is counted in thirds. At the start of the phase every unit gets
`3 * utypes[type].moves` thirds, plus three for ships while the owner has
the Lighthouse or Magellan's Expedition. A unit that was fortifying
becomes fortified and does not move that turn. Sentried and fortified
units are skipped.

A land unit's step costs:

| Step | Cost in thirds |
| --- | --- |
| Road to road | 1 |
| Road to road, from a railroad, moved by hand by the human | 0 |
| Otherwise | `3 * terrains[destination].move_cost` |

Air and sea units always pay 3 per step.

The railroad rule is narrow. Only the human's manual moves are free, and
only the starting tile needs a railroad (both need a road). A unit on a
go-to order, and any computer unit, pays one third. A river is ordinary
terrain with a move cost of 1; it gives no road-like bonus.

### Moving with too few points

When a land unit with anything other than exactly three thirds left enters
terrain that costs more than it has, `rand_scaled(3 * move_cost) > moves`
decides whether it fails. On failure it loses its remaining points and
stays put. A unit with exactly one full move (three thirds) always
succeeds; so does a unit whose remaining points cover the cost. After a
successful step the cost is subtracted and clamped at 0.

## Zones of control

A land unit (other than a Diplomat or Caravan) cannot move from a tile
next to a foreign unit to another tile next to a foreign unit, unless the
destination holds units. `if_open()` treats a city tile as never blocked,
so moving into or out of a city is always allowed. Units at sea are
exempt. The human hears a sound; with debug bit 0 set (as at Chieftain)
the `*ZOC` help text also appears.

`unit_blocked()` performs the test. When the human is the one tested, it
also reveals the foreign units and cities that are doing the blocking.

## Entering a tile

Moving onto a tile occupied by another civilization's units is an attack;
see [Combat](combat.md). Other rules, in the order the code applies them:

- **Diplomats** entering a foreign city open the Diplomat menu, and moving
  onto foreign units tries to bribe them (`buy_u()`). Neither Diplomats nor
  Caravans may enter a foreign city from the sea.
- **Caravans** entering a foreign city establish a trade route. Entering
  one of their own cities, they offer "Keep moving", "Establish trade
  route" and, when the city is building a wonder, "Help build WONDER",
  which adds the Caravan's cost to the city's shields. At a `map_dist()` below nine
  from home the trade route is dimmed, and a city on the home continent,
  at a distance below nine and not building a wonder, is simply entered.
- **Boarding.** A land unit can enter an ocean tile only if its own ships
  there have free capacity (`stack_cap()`). It is sentried, and boarding
  uses up its movement.
- **Ships** cannot enter land other than a city.
- **Foreign cities** can be entered by land units only, which captures
  them (`capunit()`). A Nuclear unit entering one detonates it.
- **Leaving a ship** onto land ends the unit's movement.
- **AI stacks.** A computer unit outside a city will not join a stack of
  four or more of its own units (two or more when it has no movement left).
- **Turning back.** A computer unit that would step straight back the way
  it came, in a stack of at most two, stops and drops its destination.

### Territory

When a city is founded, `city_create()` writes the owner's number into the
site layer of page 2 at `(x + 80, y)` for its 21 working tiles. It also
lowers the site values of the next ring (to 8). A land unit stepping onto
an improved tile (with a road, irrigation or mine) owned by another of the
civilizations 1–7 can cause a diplomatic incident:

- The human is asked to confirm breaking the treaty (`attack_ok()`), and
  confirming cancels peace.
- A computer unit at peace with the human will not enter the human's
  territory.

The test applies only between the human and others. Units on improved
tiles of foreign territory become visible to its owner, and any other
civilization's step clears the ownership mark.

### Carrying units

A ship with capacity, or a Carrier, moves its cargo with it. Land units
travel if they are sentried, if the ship starts at sea, or (for computer
players) if they have no fortify orders. A Carrier moves aircraft instead.
At most `capacity` units go with a ship, and Barbarian ships take
everything. Carried units are sentried and aircraft are refuelled.

## Huts

`has_hut()` derives huts from the map seed: at most one per 4 × 4 block,
at a position chosen by `map_rng_state`, about half the blocks, not on
ocean, cities or the polar rows. Visiting a hut sets bit 0 (the
Barbarians' bit) in `map_visible_to` for the tile, which removes it.

`enter_hut()` in `UNITS.C` ignores Barbarians and aircraft and draws
`rand_scaled(4)`:

| Roll | Outcome |
| ---: | --- |
| 0 | At `town_gap <= 3` from a city: mercenaries. Otherwise, with a site value of 13 or more, "an advanced tribe" founds a city for the visitor; with a lower one, 50 gold. |
| 1 | "Scrolls of ancient wisdom": a random advance whose prerequisites are known. On the first turn or after 1000 AD, 50 gold instead. |
| 2 | "Valuable metal deposits": 50 gold. |
| 3 | At `town_gap <= 3` from a city, or with no cities: mercenaries. Otherwise a horde of barbarians appears around the hut: Legions on mountains, Cavalry elsewhere, up to eight units for a civilization with three or more cities. |

Mercenaries are a Legion or Cavalry unit for the visitor. Barbarian units
from a hut are visible to the visitor.
