# 5. Processing a turn

Complete file:
[`src/TURN.C`](https://github.com/sjmelia/civd/blob/main/src/TURN.C).

`GAME.C` repeatedly calls `game_turn`. `TURN.C` implements that routine
alongside events, civilisation ratings, status displays and player-screen
setup. Reading the whole file shows which work the turn routine performs
directly and which work it delegates.

The local declarations need care. The macro named `units` points into
`townbuf`, the city array, at an offset of four bytes. Here, expressions such
as `units[i].size` refer to city data. The reconstructed name alone would give
a misleading account of the record.

{{#include src/TURN.C}}

## Civilisation activity

After the menu wrappers, `game_turn` counts active civilisations and handles
the first-turn display. Its main loop visits civilisation slots in ascending
order. The `players` bit mask and reload condition decide which slots run.

For each participating civilisation, the routine normally calls `civ_economy`,
caps money at 30,000, then calls `city_phase`, `unit_phase` and
`unit_cleanup`. It updates the current player's status and checks for
elimination. Unless the game is leaving, it then calls `ai_turn`. The reload
flag skips or limits earlier work before being cleared.

These calls establish the order of the phases. Their internal rules belong to
the called modules; this file does not contain the complete city or unit
simulation.

{{#include src/TURN.C | "int show_menu(char *a, int b, int c)"}}

## Events and calendar

After the civilisation loop, the routine processes uprisings and other events,
updates ratings and checks whether to display a tax warning. It then
increments `turn_tick` and advances `year`.

The calendar uses twenty-year steps before AD 1000, ten-year steps before 1500
and five-year steps before 1750. Later branches select two-year or one-year
steps. The code handles the transition through year zero explicitly and
corrects year 21 to 20.

Periodic work includes saved games when enabled, replay events and historian
reports. The remaining branches check palace progress, spaceship arrivals and
game completion before deciding whether play continues.

{{#include src/TURN.C | "uprising();"}}

## Reports and uprisings

The following functions implement the Hall of Fame and advertising screens
reached from the turn routine. `uprising` has an initial delay based on
difficulty, then tests conditions for creating barbarian units. Its later path
selects a city, tests unrest and searches for a suitable nearby location.

These are concrete event rules. Their placement after `game_turn` does not
change when they run: the calls in the turn routine establish that order.

{{#include src/TURN.C | "void show_hall_of_fame(a)"}}

## Ratings and status displays

`rate_civs` calculates and orders civilisation ratings. The display routines
that follow draw the banner, status information and palace images.
`draw_palace` uses the stored design for the current player and generates a
design for other civilisations.

These functions explain why a complete turn-processing file contains
substantial drawing code. The turn routine calls status updates at particular
points, and the supporting routines are compiled in the same resident module.

{{#include src/TURN.C | "void rate_civs(void)"}}

## Events and the player screen

`ai_event` first updates the warming state. It can display a temperature
warning or call `global_warming` in the previous chapter's file. It then
considers wonder-completion events for other civilisations, changes state and
presents the result when appropriate.

`sel_player` rebuilds the player screen around map revelation, panel drawing
and palette loading. The palette filename depends on government and a
technology check. These functions connect the state changes earlier in the
turn with the display presented to the player.

{{#include src/TURN.C | "void ai_event(void)"}}

[← Previous](04-world.c.html) · [Contents](00-opening.c.html) · [Next →](06-battle.c.html)
