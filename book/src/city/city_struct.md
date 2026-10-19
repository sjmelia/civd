# City records

The world has room for 128 cities, shared by all eight civilization slots.
`townbuf[128]` starts at DGROUP offset `0x70ec`. Each record is 28 bytes,
so the array occupies 3,584 bytes. A `status` of −1 marks a free record.

```c
{{#include ../../../src/CIV.H:29:45}}
```

`o20_city()` in `CITY.C` finds a free record when a Settler founds a city.
`city_create()` sets up its position, owner, name, workers and reserves.
`vanish()` in `ECONOMY.C` removes a city, clears its map flags and removes
references to it from trade routes and supported units.

## Record layout

| Offset | Field | Use |
| ---: | --- | --- |
| `0x00` | `buildings` | 32-bit mask; improvement n uses bit n−1 |
| `0x04–05` | `x`, `y` | Map position |
| `0x06` | `status` | State flags; −1 means unused |
| `0x07` | `size` | Population size |
| `0x08` | `visible_size` | Last reported size for the map display |
| `0x09` | `production` | Unit ID, or negative improvement ID |
| `0x0a` | `trade` | Trade used in route calculations |
| `0x0b` | `owner` | Civilization slot |
| `0x0c` | `food_box` | Stored food, a 16-bit integer |
| `0x0e` | `shield_box` | Stored production, a 16-bit integer |
| `0x10` | `workers` | Worked tiles in bits 0–20; specialist count in bits 26–31 |
| `0x14` | `specialists` | Eight two-bit specialist types |
| `0x16` | `name_id` | Unsigned index into `place_name[256][13]`, accessed with a cast |
| `0x17–19` | `routes` | Up to three destination cities; −1 means empty |
| `0x1a–1b` | `garrison` | Two stored unit types, with veteran bit `0x40`; −1 means empty |

## Workers and specialists

`radius_dx` and `radius_dy` give the 21 tile offsets around a city. The
centre is tile 20 and is worked free of charge. Other worked tiles use
citizens. `city_process()` reserves neighbouring cities' worked tiles and
centres before assigning workers, so two cities cannot work the same tile.

The specialist count and specialist types are stored separately.
`set_specialist()` and `get_specialist()` use 0 for an unused slot, 1 for
a taxman, 2 for a scientist and 3 for an entertainer. Indices of eight or
more read back as taxmen. Each specialist adds two points to taxes, science
or luxuries, respectively, before building multipliers.

## Stored garrisons

`garrison_in()` in `UNITS.C` can store two fortified units in the city
record, keeping the first qualifying unit in the ordinary unit array. The
stored bytes hold types and veteran flags. `unpack_garrison()` recreates
those units with their fortified and veteran status.
