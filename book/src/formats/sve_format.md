# Saved games

`SAVE.C` writes two files per save: `CIVIL?.SVE` for variables and arrays,
and `CIVIL?.MAP` for the map page. Both are needed to restore a game.

## State transfer

`transfer_game()` uses the same list of addresses and byte counts to read
and write the SVE file. Values are stored in the DOS program's representation:
16-bit little-endian integers, four-byte longs and packed record bytes.
The file has no separate field-name or type metadata.

The [transfer table](sve_offset_table.md) gives exact offsets and lengths.
It includes the city, unit-type and unit arrays, exploration masks,
relations, replay history, spaceship layouts and palace design.

`transfer_near()` transfers shared-data buffers directly. `transfer_far()`
copies through `msgtext` in chunks of at most 512 bytes to handle data in
other segments.

## Header

The first eight two-byte values are:

| Offset | Variable | Meaning |
| ---: | --- | --- |
| `0x00` | `turn_tick` | Turn number |
| `0x02` | `civnow` | Human civilization slot |
| `0x04` | `human_set` | Human-controlled civilization mask |
| `0x06` | `map_rng_state` | Map seed |
| `0x08` | `year` | Calendar year |
| `0x0a` | `cur_tier` | Difficulty, 0–4 |
| `0x0c` | `players` | Living civilization mask |
| `0x0e` | `selected_index` | Human research selection |

Leader names occupy `0x10–0x7f`, civilization names `0x80–0xdf`, and
adjectives `0xe0–0x137`. The preview path in `read_save_header()` seeks
to selected fields to build menu entries without loading the full state.

The map seed controls resource and hut patterns. Its presence in a save
does not establish that the C runtime's complete random-generator state
has been saved.

Treaties are [bit flags](../ai/relations.md), not a numbered list of
mutually exclusive diplomatic states.
