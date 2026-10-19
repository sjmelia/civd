# Tribes, Colours and Titles

A game has eight civilization slots. Slot 0 is always the Barbarians; slots
1–7 are colour slots, and each can hold one of two tribes.
`civ_tribe_of[slot]` is an index into `civt`: the slot number selects the
first tribe and the slot number plus 8 the alternate.

| Slot | Colour index | First tribe (`civt[slot]`) | Alternate (`civt[slot + 8]`) |
| ---: | ---: | --- | --- |
| 0 | 12 | Barbarians | — |
| 1 | 15 | Romans | Russians |
| 2 | 10 | Babylonians | Zulus |
| 3 | 9 | Germans | French |
| 4 | 14 | Egyptians | Aztecs |
| 5 | 11 | Americans | Chinese |
| 6 | 13 | Greeks | English |
| 7 | 7 | Indians | Mongols |

The colour index comes from `civ_colours[8]` in `GAME.C`; `civ_shadows[8]`
holds a darker companion for each. Because a slot can hold only one tribe,
the Romans and Russians never meet in the same world.

## Choosing tribes

`new_game()` in `NEWGAME.C` offers the tribes of slots 1 to
`civ_total + 1`, first tribes then alternates, so a game with fewer rivals
offers fewer tribes. Leaving the menu without a choice picks a random slot
and calls `rename_tribe()` in `NAMES.C`, which stores the typed plural and
derives the adjective by dropping a final "s".

For each rival, `init_civ()` draws `rand_scaled(16)` until the result
belongs to that slot (`(tribe & 7) == slot`) and is not already in use.
When a slot is reused later in the game, the tribe is toggled with
`civ_tribe_of[civ] ^= 8`, so a destroyed tribe returns as its partner.
`civil_war()` in `CIVWAR.C` usually does the same for the breakaway
civilization; for a slot beyond the game's original rivals it picks either
tribe at random.

The tables `civ_names`, `civ_adjectives` and `leader_names` in `GAME.C` are
working copies. Their initial contents (including leaders such as Xerxes and
Saladin, who do not appear in `civt`) are replaced from `civt` as each slot
is filled.

## Titles

`leader_titles[civ_gov[civ]]` gives the title used in audiences and
reports:

| Government | Index | Title |
| --- | ---: | --- |
| Anarchy | 0 | Mr. |
| Despotism | 1 | Emperor |
| Monarchy | 2 | King |
| Communism | 3 | Comrade |
| Republic | 4 | President |
| Democracy | 5 | President |

While an audience with Elizabeth I (`civt[14]`) is open,
`leader_audience()` replaces entries 1 and 2 with "Empress" and "Queen".
On closing it restores them from `rank_title` in `INTRO.C`, whose five
entries ("Chief", "Lord", "Prince", "King", "Emperor") otherwise name the
five difficulty levels in the Hall of Fame and save-game lists.
