# Global Game State

Game state is distributed across shared globals and larger arrays. It includes the current turn and year, active player, civilization finances and research, diplomatic relations, cities, units, wonders and map visibility.

`src/GAME.C` defines the main rule tables and civilization variables. Subsystem headers identify other shared globals. The [save transfer sequence](../formats/sve_offset_table.md) shows the serialized state and its ordering.
