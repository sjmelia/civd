# Overlays

To run within the 640 KiB conventional memory ceiling of MS-DOS, *Civilization* uses Microsoft LINK's dynamic overlay manager.

All **23 overlays** share a single memory window of **9,648 bytes** located at resident linear address `0x23740` (frame `0x2374`). When an overlay routine is invoked, the overlay manager loads the requested overlay from disk (`CIV.EXE`) into this shared window before executing it.

```
       Resident Code Area                     Shared Overlay Swap Window
┌───────────────────────────────┐          ┌──────────────────────────────┐
│ Resident Modules (0000..23740)│ ──Call──>│ 2374:0000                    │
│                               │          │ Active Overlay Code (Max 9KB)│
│  [GAME.C, TURN.C, CITYMGT.C]  │<──Ret────│                              │
└───────────────────────────────┘          └──────────────────────────────┘
                                                          ▲
                                                          │ Swapped from disk
                                           ┌──────────────┴──────────────┐
                                           │ CIVILIZATION EXE File       │
                                           │  - Overlay 1  (SETUP)       │
                                           │  - Overlay 6  (AUDIENCE)    │
                                           │  - Overlay 7  (WORLDGEN)    │
                                           │  - Overlay 11 (SAVE)        │
                                           │  - Overlay 18 (SPACESHP)    │
                                           │  ... (23 Total Overlays)    │
                                           └─────────────────────────────┘
```

---

Each module can expose several entry points; the table shows one current shared declaration.

## Complete Catalog of the 23 Overlays

| Overlay ID | Module Source | Local Code Bytes | Example Shared Export | Functional Purpose |
|:---:|---|---:|---|---|
| **1** | `src/SETUP.C` | 1,018 | `setup_main()` | Hardware detection, graphics mode & audio driver configuration. |
| **2** | `src/INTRO.C` | 6,588 | `game_startup()` | Title sequence, MicroProse logo animations, credits. |
| **3** | `src/FAME.C` | 3,192 | `fame_load()` | Hall of Fame score calculation and high score table display. |
| **4** | `src/NOTES.C` | 1,146 | `o4_notes()` | In-game instructional popups, game tips, and key summaries. |
| **5** | `src/NEWGAME.C` | 7,534 | `new_game()` | New game menu, difficulty, world customization, tribe selection. |
| **6** | `src/AUDIENCE.C` | 9,642 | `leader_audience()` | Diplomatic negotiations, treaty proposals, leader audience screens. |
| **7** | `src/WORLDGEN.C` | 7,512 | `make_world()` | Procedural map generation, land growth, climate, rivers. |
| **8** | `src/PEDIA.C` | 6,134 | `civilopedia()` | Civilopedia encyclopedia viewer (units, improvements, wonders). |
| **9** | `src/REPLAY.C` | 5,468 | `o9_replay()` | Historical timeline replay displayed upon game victory/defeat. |
| **10** | `src/MAPVIEW.C` | 1,702 | `o10_mapview()` | Full-world strategic minimap viewer and territory inspection. |
| **11** | `src/SAVE.C` | 3,994 | `o11_save()` | Save/load menus, disk validation, `.SVE` and `.MAP` serialization. |
| **12** | `src/REPORTS.C` | 7,000 | `o12_reports()` | Military, foreign affairs, commerce, and demographic status reports. |
| **13** | `src/STATUS.C` | 2,472 | `o13_status()` | World status screen, civilization ranking, Top 5 Cities display. |
| **14** | `src/ADVISORS.C` | 7,026 | `advisor_open()` | Graphic advisor consultations (Military, Domestic, Foreign, Science). |
| **15** | `src/CIVWAR.C` | 2,426 | `civil_war()` | Civil war partition logic triggered when an enemy capital falls. |
| **16** | `src/QUIZ.C` | 1,372 | `o16_quiz()` | Manual-lookup copy protection challenge questions. |
| **17** | `src/PALACE.C` | 4,246 | `palace_view()` | Palace upgrade view and graphic construction screen. |
| **18** | `src/SPACESHP.C` | 5,842 | `spaceship_open()`| Alpha Centauri spaceship component assembly, flight simulation, arrival. |
| **19** | `src/CITYVIEW.C` | 7,332 | `city_scene()`| Hand-drawn graphical city interior view with active improvements. |
| **20** | `src/CITY.C` | 5,188 | `o20_city()` | City founding ceremony, naming prompt, tile radius activation. |
| **21** | `src/NEWS.C` | 1,066 | `news_box()` | Newspaper banner popups announcing world wonders and disasters. |
| **22** | `src/DIPLOMAT.C` | 4,434 | `dip_menu()`| Diplomat operations (bribery, sabotage, theft). |
| **23** | `src/NAMES.C` | 1,926 | `rename_leader()` | Procedural city name lists and tribal leader nomenclature. |

---

## Overlay Sizing Constraint

The overlay swap window is determined by the **largest single overlay** linked into the binary:
$$\text{Max Overlay Size} = \text{Overlay 6 (\texttt{AUDIENCE.C})} = 9,642\text{ bytes}$$

Because LINK allocates `9,648` bytes (rounded up to a paragraph boundary) at `[0x23740, 0x25CF0)`, any individual overlay is guaranteed to fit into the swap window without corrupting adjacent far data segments.
