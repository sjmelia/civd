# Civilization 474.01, 474.03 and 474.05

Each version has its own branch. `release/474.01` and `release/474.03` rebuild
those executables byte for byte; `main` is the 474.05 reconstruction. This file lives
on `main` and is merged into the release branches, so every branch carries
the same comparison.

Compare the versions with:

```sh
git diff release/474.01 release/474.03 -- src/
git diff release/474.03 main -- src/
git diff --stat release/474.01 main -- src/
```

Each command shows changes from the earlier release to the later one. Names and C source
forms are reconstructed; a source diff includes compiler/layout choices as
well as changes in game behavior. It does not recover MicroProse's original
patch or establish when each change was introduced between these releases.

## Selected differences

| Area | 474.01 | 474.03 | 474.05 |
| --- | --- | --- | --- |
| Visibility storage | A near 80 × 50 byte array in DGROUP | As 474.05 | A far array; save transfer still covers 4,000 bytes |
| City specialists | No five-citizen restriction on changing specialist types | As 474.05 | Five-citizen restriction and warning dialog |
| City happiness | Simpler difficulty calculation and troop suppression | As 474.05 | Additional government/city-count calculation, excess-unhappiness handling and three-troop cap |
| City interface | No RENAME button | As 474.05 | RENAME button and shortcut handling |
| Barracks upkeep | Technology affects upkeep | As 474.05 | Difficulty adds further upkeep at levels 2 and 4 |
| Score | Completion score replaces the accumulated score | As 474.05 | Shows and selects a bonus score when it exceeds the accumulated score |
| AI civilization cleanup | No cleanup call in `unit_phase()` | As 474.05 | Checks civilizations with no settlers and no cities |
| Nuclear-unit direction | `((k - 1) ^ 4) + 1` | As 474.05 | `k ^ 4` |
| Barbarian target weighting | Divides by map distance | As 474.05 | Divides by map distance plus one |
| Production advice | Unusable choices rank 99 and are scaled like any other improvement | As 474.01 | Unusable choices rank 999; ranks of 400 or more skip the scaling and become `0x3fff` |
| Land connectivity | `link_land()` reads `terrain_at()`, reports progress through `story_step()` and writes the neighboring coarse cell without a bounds check; no page copy before linking | As 474.01 | Reads `gen_terrain_at()`, checks coarse coordinates before writing, makes no progress call and copies the work page before linking |
| Palette cycling | Several calls run without a VGA-mode guard | As 474.05 | Adds VGA-mode guards in core, introduction, spaceship and city-view paths |
| Hidden map command | `%` followed by `^` toggles x-ray viewing | As 474.05 | Command removed |
| Startup display | Prints the bundled introduction's version | Replaces its version digit with `3` | Replaces its version digit with `5` |
| Timer setup | Earlier timer initialization | As 474.05 | Adds PIT reprogramming |
| `ai_turn()` in `CONTROL.C` | Too large for the post-optimizer (C4063) | Post-optimized, 2,051 bytes smaller | Too large for the post-optimizer |

The 474.03 column is best read as 474.05 with two of its later fixes missing:
the production-advice and land-connectivity changes arrived after it.

See `CITYMGT.C`, `CITY.C`, `ECONOMY.C`, `UNITAI.C`, `WORLDGEN.C`, `CORE.C`,
`SETUP.C` and `TIMER.INC` for these operations. Other differences include production advice,
starting-unit placement order, spaceship handling, and disk-insertion calls.
Nuclear fallout is implemented in `CITYMGT.C` in 474.01 and `BATTLE.C` in 474.05.

The picture codec remains recovered assembly in both versions. Its shared
state and buffer addresses change with the memory layout; those address
changes do not establish a different image format.

## Build and verification

All reconstructions use the supplied Microsoft C 5.1, MASM 5.10, MAKE 4.07 and
LINK 5.01 tools. The 474.01 build also applies the reconstructed low-memory
compiler condition to `UNITAI.C`, preserving its unoptimized `ai_order()`.
The 474.03 build compiles `CONTROL.C` without the low-memory condition, and
declares four resident code segments (`F1459`, `F1C84`, `F1D05`, `F1F4D`) as
word-aligned, as the 474.01 build does.
Historical compiler memory, identifier spelling and exact original source
forms remain unknown.

Run `just verify` after checking out the desired branch. Each checkout pins
its own complete raw LINK output and final executable in `civ.sha256`.
The graphics, sound and MISC executables are identical between these versions.
The build rejects compiler/linker errors even if period MAKE leaves an output.

| Final executable | Bytes | SHA-256 |
| --- | ---: | --- |
| 474.01 | 304,512 | `21861cbb5eca78ddf22d7b84219b41a22a1d283c3f7b27f3d1bce98c30eb8e3b` |
| 474.03 | 302,982 | `31e35b01f4d00df81685dfcb81e6de196a3876bd2afff6e39cbbfd964f5852ad` |
| 474.05 | 305,030 | `22de2748e3089793644c7e501e2a95b553010ddec34a7ff42f4395cf779c9c2a` |

All reconstructions retain the documented four-byte startup-call exception. In
474.01 the call opcode is at file offset `0x205fd`; the destination changes
from `2058:0446` to `01a7:01c3`. Its pinned raw LINK hash is
`b04fc0ba243d446047708b04142024391877c52dd21f13bdfee2020ec4579466`.
In 474.03 the opcode is at `0x1fcaf` and the destination changes from
`1fc3:0448` to `01a8:01c1`, the same final target as 474.05. Its pinned raw
LINK hash is `f9d0cbe806d767b268e678838e4885425254ddbdb84be9e7f64ddaca9545e15c`.

Original game assets and compiler media are local inputs, outside Git.
The comparison covers the rebuilt executables; the disk releases also have
asset and text differences that this source branch does not reproduce.
