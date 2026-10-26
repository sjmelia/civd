# Contents

- [Reading the source](introduction.md)
- [Building the same executable](architecture/toolchain_clues.md)

# Game structure

- [Memory layout](architecture/memory_model.md)
    - [Shared game data](architecture/dgroup_map.md)
- [Overlays](architecture/overlays.md)
    - [Loading overlays](architecture/overlay_dispatch.md)

# Graphics, sound and input

- [Graphics](engine/graphics.md)
    - [The MCGA graphics driver](engine/graphics_driver.md)
- [Sound](engine/audio.md)
    - [The AdLib sound driver](engine/sound_driver.md)
- [Input and interrupts](engine/input_interrupts.md)

# The world

- [Map coordinates](map/geometry.md)
- [Map storage](map/memory_layout.md)
- [Terrain tables](map/terrains.md)
- [Tile yields and improvements](map/tile_yields.md)
- [World generation](map/worldgen.md)
- [Starting conditions](map/starting_conditions.md)
- [Exploration and visibility](map/fog_of_war.md)

# Units and combat

- [Unit records](entities/unit_struct.md)
    - [Stacks](entities/unit_stacking.md)
- [Unit types](entities/unit_types.md)
- [Movement](entities/movement.md)
    - [Pathfinding](entities/pathfinding.md)
    - [Aircraft](entities/air_mechanics.md)
- [Combat](entities/combat.md)
    - [Nuclear weapons](entities/nuclear_mechanics.md)

# Cities and the economy

- [City records](city/city_struct.md)
- [Production and growth](city/production.md)
- [Taxes and science](city/economy.md)
- [Happiness and disorder](city/happiness.md)
- [Caravan trade](city/trade_routes.md)
- [Buildings and wonders](city/improvements_wonders.md)

# AI and diplomacy

- [Leaders and personalities](ai/leader_personalities.md)
    - [Tribes, colours and titles](ai/civ_identities.md)
- [Diplomatic relations](ai/relations.md)
    - [Leader audiences](ai/audience.md)
    - [Diplomacy between computer players](ai/ai_diplomacy.md)
- [Unit AI](ai/unit_ai.md)
    - [Unit orders and attack evaluation](ai/unit_orders.md)

# File formats

- [Saved games](formats/sve_format.md)
    - [State transfer table](formats/sve_offset_table.md)
- [The map save file](formats/map_file.md)
- [Pictures, palettes and fonts](formats/asset_formats.md)

# Source reference

- [Constants](reference/civ_types.md)
    - [Governments and civilization slots](reference/enum_gov_civ.md)
    - [Unit and terrain IDs](reference/enum_unit_terrain.md)
    - [Buildings and wonders](reference/enum_buildings.md)
    - [Technologies](reference/enum_tech.md)
- [Record declarations](reference/records.md)
    - [City](reference/struct_city.md)
    - [Unit](reference/struct_unit.md)
    - [Unit type](reference/struct_unit_type.md)
    - [Terrain](reference/struct_terrain.md)
    - [Global state](reference/struct_game_state.md)
- [Shared declarations](reference/api_signatures.md)
    - [Resident modules](reference/resident_api.md)
    - [Overlay modules](reference/overlay_api.md)
