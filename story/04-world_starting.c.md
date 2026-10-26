# 5. Rolling starting conditions

Complete file:
[`src/NEWGAME.C`](https://github.com/sjmelia/civd/blob/main/src/NEWGAME.C).

A new map still needs players. `NEWGAME.C`, overlay 5, chooses tribes,
places their first Settlers and compensates for differences between their
starting sites. This is the code behind a player's question about “rolling”
starting units and technologies. The [technical reference](../book/map/starting_conditions.html)
gives the scoring tables and worked examples.

The opening declarations lead to `start_pos`, a table indexed by tribe.
These are the fixed Earth starting coordinates, rather than a table of
tribe-specific starting units or advances.

{{#include src/NEWGAME.C}}

## Preparing all players before comparing them

`new_game` gathers difficulty, competition and tribe choices. It sets the
year to 4000 BC, clears shared state and sets `selected_index` to -1.
It then calls `init_civ` for every slot before calling `rate_starts` once.
That order matters: bonuses depend on the best start actually placed in
this game, including the computer civilizations.

After rating, the human gets science and tax settings of 5 and, on
Chieftain, 50 gold. The introductory message enumerates the actual known
advance bits and appends “and Roads.” That phrase is presentation text;
there is no separate Roads advance drawn by the bonus routine.

{{#include src/NEWGAME.C | "void new_game(void)"}}

## One Settlers, then a search for its home

`init_civ` resets a slot's economy, government, units and known advances.
For an active non-barbarian civilization it repeatedly draws a location.
Ocean and huts fail the test; site quality, distance and available sites
in the region impose thresholds that relax as `trial` increases.
Earlier slots' initial units enter the distance test, making this a
sequential placement process. Earth replaces the search result with
`start_pos[civ_tribe_of[civ]]`.

The accepted start receives `create_unit(civ, 0, x, y)`: one Settlers,
with no city founded. The nearby-barbarian cleanup follows. Assigning
`tech_count[civ] = 1` does not set any known-advance bit.

The `turn_tick` branches also show why this function cannot be read solely
as new-game setup. During play it can choose an alternative tribe and
copy selected technologies from the human. Those branches do not run at
turn zero.

{{#include src/NEWGAME.C | "int init_civ(int civ)"}}

## Reusing the initializer after destruction

`kill_civ` checks for surviving cities, announces destruction, clears units
and visibility, then calls `init_civ` again. It does not rerun the comparison
of all starting positions. Replacement-civilization technology grants and
new-game compensation therefore have different rules.

{{#include src/NEWGAME.C | "void kill_civ(int civ, int by)"}}

## Turning geography into units and skills

`rate_starts` makes a scoring pass, retaining `best_score`. Each live slot's
score combines distance to the nearest civilization on the same region,
River or Grassland in the surrounding nine tiles, and region area.
No same-region neighbour earns four points; River can earn two and area
another two. The “Coastal” label adds no points.

In the second pass, a gap greater than three earns another Settlers at
the original tile and raises `pts[p]` by three. Each remaining point of gap
calls `reduce_attitude(p, 0)`. For example, a four-point gap produces two
Settlers in total and one free advance; a three-point gap produces one
Settlers and three advances. Equal scores produce no bonuses.

The advance selector lives in
[`ECONOMY.C`](https://github.com/sjmelia/civd/blob/main/src/ECONOMY.C).
It gives every unknown, prerequisite-eligible advance a random rating
weighted by the tribe's personality and retains the highest. It grants
the winner immediately, so a later draw can use it as a prerequisite.
At turn zero the human uses this automatic path too. The number of grants
comes from geography; the random ratings choose which advances fill them.

The `isolated` bitmask here deserves care: it marks a finite neighbour
distance of at least ten, while the text “isolated” describes having no
same-region neighbour at all. The mask affects selection of the returned
slot. `new_game` stores that return in `ret` but does not use it to change
the human's tribe or placement.

{{#include src/NEWGAME.C | "int rate_starts(void)"}}

## The rest of the overlay

The remaining functions load terrain, unit and adviser artwork, select
and display the government cabinet, and reset spaceship records. They
share the setup overlay but do not add starting units or technologies.
Keeping them here completes the file and shows the presentation work that
surrounds the starting-condition rules.

{{#include src/NEWGAME.C | "void setup_art(void)"}}

[← Previous](04-world.c.html) · [Contents](00-opening.c.html) · [Next →](05-turn.c.html)
