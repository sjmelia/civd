# State transfer table

`transfer_game()` in `SAVE.C` transfers the following fields in order.
Offsets are measured from the start of the SVE file. Near and far describe
the source address space; both write the listed number of bytes.

The total length is 37,856 bytes (`0x93e0`).

| Offset | Bytes | Variable | Transfer | Use |
| ---: | ---: | --- | --- | --- |
| `0x0000` | 2 | `turn_tick` | near | Turn number |
| `0x0002` | 2 | `civnow` | near | Active player index |
| `0x0004` | 2 | `human_set` | near | Civilizations under human control |
| `0x0006` | 2 | `map_rng_state` | near | RNG seed |
| `0x0008` | 2 | `year` | near | Calendar year |
| `0x000a` | 2 | `cur_tier` | near | Difficulty level (0..4) |
| `0x000c` | 2 | `players` | near | Living civilization bitmask |
| `0x000e` | 2 | `selected_index` | near | Advance being researched by the player |
| `0x0010` | 112 | `leader_names` | near | Leader names (8 × 14) |
| `0x0080` | 96 | `civ_names` | near | Civilization names (8 × 12) |
| `0x00e0` | 88 | `civ_adjectives` | near | Adjectives (8 × 11) |
| `0x0138` | 16 | `money` | near | Gold reserves (8 × int16) |
| `0x0148` | 16 | `civ_research_tally` | near | Research accumulated toward the next advance (8 × int16) |
| `0x0158` | 448 | `forces_present` | near | Units in play per civilization and type (8 × 28 × int16) |
| `0x0318` | 448 | `civ_units_ordered` | near | Cities building each unit type (8 × 28 × int16) |
| `0x04d8` | 16 | `tech_count` | near | Advances known per civilization (8 × int16) |
| `0x04e8` | 80 | `advances_known` | near | Known-advance bitmasks (8 × 5 × int16) |
| `0x0538` | 16 | `civ_gov` | near | Government type (8 × int16) |
| `0x0548` | 256 | `civ_area_plan` | near | AI policy per civilization and continent (8 × 16 × int16) |
| `0x0648` | 128 | `treaties` | near | Bilateral relations matrix (8 × 8 × int16) |
| `0x06c8` | 16 | `city_count` | near | Cities per civilization |
| `0x06d8` | 16 | `troops_total` | near | Non-settler units per civilization |
| `0x06e8` | 16 | `civ_held_cells` | near | Coarse 4 × 4 map cells holding own units or cities |
| `0x06f8` | 16 | `civ_settlers_n` | near | Settlers per civilization |
| `0x0708` | 16 | `citizens` | near | Total city sizes per civilization |
| `0x0718` | 16 | `military_table` | near | Military strength: attack plus defence of all units |
| `0x0728` | 16 | `nation_ratings` | near | Civilization power ranking |
| `0x0738` | 16 | `civ_toll` | near | Tax rate in tenths (8 × int16) |
| `0x0748` | 16 | `civ_ratings` | near | Historical victory score |
| `0x0758` | 16 | `last_met_turn` | near | Turn of the last audience with each civ (+16 after a refusal, -2 when destroyed) |
| `0x0768` | 16 | `x_cap` | near | X of the capital; centres map views |
| `0x0778` | 16 | `civ_tribe_of` | near | Tribe template type |
| `0x0788` | 256 | `defenders_island` | near | Defence strength per civilization and continent |
| `0x0888` | 256 | `civ_cont_attackers` | near | Attack strength per civilization and continent |
| `0x0988` | 256 | `ai_continent_towns` | near | Cities per civilization and continent |
| `0x0a88` | 256 | `continent_span` | near | Tiles per land region, then per sea region (2 × 64 × int16) |
| `0x0b88` | 32 | `ai_zone_places` | near | Good city sites per continent (16 × int16) |
| `0x0ba8` | 1,200 | `power_history` | far | Civilization power history (150 samples × 8) |
| `0x1058` | 1,200 | `war_record` | far | Civilizations each civ was at war with, sampled every 4 turns (150 × 8 masks) |
| `0x1508` | 3,584 | `townbuf` | near | The 128 City Records (128 × 28 bytes) |
| `0x2308` | 952 | `utypes` | near | The 28 Unit Types (28 × 34 bytes) |
| `0x26c0` | 12,288 | `armydata` | near | The 1,024 Active Units (8 × 128 × 12 bytes) |
| `0x56c0` | 4,000 | `map_visible_to` | far | Per-civilization tile visibility bits (80 × 50) |
| `0x6660` | 128 | `spot_classes` | near | AI site kind per civilization and slot |
| `0x66e0` | 128 | `ai_camp_site_weights` | near | AI site weight per slot |
| `0x6760` | 128 | `camp_xpos` | near | AI site X per slot |
| `0x67e0` | 128 | `civ_lot_y` | near | AI site Y per slot |
| `0x6860` | 144 | `advance_owners` | far | First discoverer of each advance |
| `0x68f0` | 128 | `killed` | near | Bilateral casualty counters (8 × 8 × int16) |
| `0x6970` | 3,328 | `place_name` | far | City Name Pool (256 × 13 bytes null-padded) |
| `0x7670` | 2 | `replay_length` | near | Bytes used in `replay_log` |
| `0x7672` | 4,096 | `replay_log` | far | Replay event log |
| `0x8672` | 44 | `marvel_site` | near | The 22 Wonders (22 × int16 city indices) |
| `0x869e` | 448 | `unit_losses` | far | Units lost per civilization and type (8 × 28 × int16) |
| `0x885e` | 576 | `adv_source` | far | Technology discovery tracking |
| `0x8a9e` | 2 | `polluted` | near | Global pollution tile counter |
| `0x8aa0` | 2 | `warm_levels` | near | Global warming risk (0..99; warming strikes above 16) |
| `0x8aa2` | 2 | `warming` | near | Global warming event stage |
| `0x8aa4` | 2 | `debug` | near | Debug cheat flag |
| `0x8aa6` | 260 | `block_ground_way` | near | Land connectivity of coarse 4 × 4 map cells (20 × 13) |
| `0x8baa` | 2 | `max_rating` | near | All-time highest score |
| `0x8bac` | 2 | `advances_future` | near | Future Technology advances discovered |
| `0x8bae` | 2 | `rule_options` | near | Game rule flags (all set at the start) |
| `0x8bb0` | 16 | `sci` | near | Science slider rate (8 × int16) |
| `0x8bc0` | 2 | `next_event_turn` | near | Scheduled historical event turn |
| `0x8bc2` | 16 | `history_rating` | near | Historian ranking points; [0] counts reports |
| `0x8bd2` | 1,440 | `spaceships` | far | Spaceship layouts (8 × 15 × 12) |
| `0x9172` | 2 | `game_flags` | near | Spaceship flags: bits 0–7 launched, 8–15 built |
| `0x9174` | 2 | `ship_success` | near | Human spaceship success chance, percent |
| `0x9176` | 2 | `ai_ship_success` | near | AI spaceship success chance, percent |
| `0x9178` | 16 | `spaceship_arrive_dates` | near | Spaceship arrival year (8 × int16, -1 none) |
| `0x9188` | 24 | `floors_palace` | near | Palace architectural room heights |
| `0x91a0` | 24 | `section_grades` | near | Palace architectural styles (Classical, etc.) |
| `0x91b8` | 256 | `city_site_x` | far | Proposed city site X |
| `0x92b8` | 256 | `city_site_y` | far | Proposed city site Y |
| `0x93b8` | 2 | `palace_stage` | near | Palace upgrade completion stage |
| `0x93ba` | 2 | `player_peace_run` | near | Peaceful turns since the last non-barbarian battle (counted in AD) |
| `0x93bc` | 2 | `civ_total` | near | Total living civilizations count |
| `0x93be` | 16 | `space_ship_habitation` | near | Habitation modules per civilization |
| `0x93ce` | 16 | `liftoff_when` | near | Spaceship launch year per civilization |
| `0x93de` | 2 | `civ_flags_bits` | near | Master civilization bitflags |
