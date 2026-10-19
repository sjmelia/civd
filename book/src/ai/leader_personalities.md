# Leaders and Personalities

Each of the fourteen playable tribes has a 58-byte record in `civt[16]`
(`src/GAME.C`). Entries 1–7 are the first tribe of each colour slot and
entries 9–15 the alternate tribe; entries 0 and 8 are empty.

```c
struct civ_type {
    char leader[16];
    char plural[16];      /* empty: the adjective plus "s" */
    char adjective[16];
    int f0, f1, f2, f3, f4;
};
```

`f0`, `f1` and `f2` are the personality. Each is −1, 0 or 1. `f3` and `f4`
are tune numbers passed to `sound()`; `f4` plays when the leader's portrait
appears and `f3` when its emissary arrives, when it captures a city and
when it discovers an advance.

| Entry | Leader | Tribe | `f0` | `f1` | `f2` |
| ---: | --- | --- | ---: | ---: | ---: |
| 1 | Caesar | Romans | 0 | 1 | 1 |
| 2 | Hammurabi | Babylonians | −1 | −1 | 1 |
| 3 | Frederick | Germans | 1 | −1 | 1 |
| 4 | Ramesses | Egyptians | 0 | 0 | 1 |
| 5 | Abe Lincoln | Americans | −1 | 0 | 1 |
| 6 | Alexander | Greeks | 0 | 1 | −1 |
| 7 | M.Gandhi | Indians | −1 | −1 | 0 |
| 9 | Stalin | Russians | 1 | 0 | −1 |
| 10 | Shaka | Zulus | 1 | 0 | 0 |
| 11 | Napoleon | French | 1 | 1 | 1 |
| 12 | Montezuma | Aztecs | 0 | −1 | 1 |
| 13 | Mao Tse Tung | Chinese | 0 | 0 | 1 |
| 14 | Elizabeth I | English | 0 | 1 | 0 |
| 15 | Genghis Khan | Mongols | 1 | 1 | −1 |

## Trait labels

The foreign-affairs dossier in `ADVISORS.C` prints a label for each non-zero
trait:

| Field | 1 | −1 |
| --- | --- | --- |
| `f0` | Aggressive | Friendly |
| `f1` | Expansionistic | Perfectionist |
| `f2` | Civilized | Militaristic |

A debug key in `CONTROL.C` replaces all three traits of every tribe with
`rand_scaled(3) - 1` and reports "Leaders randomized."

## Where the traits are read

**Aggression (`f0`).** In a leader audience, `leader_audience()` counts
the AI's reasons to be conciliatory, such as wars with stronger rivals,
subtracts `f0`, and drops its hostility if the count is high enough. An
Aggressive leader needs one more reason and a Friendly leader one fewer. In AI-to-AI diplomacy, `alliance_ok()` subtracts
`f0` from its count of the civilization's wars before comparing military
strength. See [Leader Audiences](audience.md) and
[AI Diplomacy Between Rivals](ai_diplomacy.md).

**Expansion (`f1`).** `ai_order()` in `UNITAI.C` lets a settler with a
destination keep travelling while
`(f1 + 1) * ai_zone_places[region] > towns_on_region * 16`, where
`ai_zone_places` counts the continent's possible city sites. A
Perfectionist (`f1 = −1`) therefore stops at once and turns to terrain work;
an Expansionistic tribe keeps founding cities on a continent until it holds
about one town for every eight sites.

**Civilization (`f2`).** This trait affects several decisions:

- `init_civ()` in `NEWGAME.C` sets the starting science rate to `f2 + 3`,
  and `ECONOMY.C` adds `f2` when the AI resets its science rate.
- The AI production chooser divides each improvement's rank by `f2 + 3`
  (`rank = 3 * rank / (f2 + 3)`). A lower rank wins, so a Civilized tribe
  favours improvements and a Militaristic tribe favours units.
- The settler rank uses `4 - f2` (with a Granary) or `6 - f2` as its factor,
  so Militaristic tribes build fewer settlers.
- `advance_value()` returns `a + f2 * b` from the advance's
  `advance_attrs` entry. It weights the AI's random research choice, the
  advance taken when a city is captured and the advance an AI demands in an
  audience. Advances with a negative `b`, such as Bronze Working or
  Gunpowder, appeal more to Militaristic tribes.
- During the AI's city pass, `CITYMGT.C` subtracts from `margin_democracy`
  `7 - f2` (or `5 - f2` when the city has improvement bit `0x10`) for each
  unit counted as abroad. `unit_phase()` adopts the Republic only while
  `margin_democracy` is not negative, so Militaristic tribes with armies
  abroad keep a Monarchy or Communism.

## The "Nuclear Gandhi" story

A widely repeated story holds that Gandhi's aggression was stored as 1, that
adopting Democracy subtracted 2, and that an unsigned byte wrapped to 255.
The recovered code does not support it. Gandhi's `f0` is −1 (Friendly), the
field is a signed `int`, and no code adjusts it when a government changes.
The traits change only in the debug randomizer.
