# Improvement and Wonder Values

Improvement numbers follow the [source improvement table](../city/improvements_wonders.md); the bit for improvement `n` in a city's `buildings` word is `n - 1`. `CIV.H` defines `HAS_*` macros for each improvement bit.

| ID | Bit | Mask | Macro | Improvement |
|:--:|:---:|:----:|:------|:------------|
| 1 | 0 | `0x00000001L` | `HAS_PALACE` | Palace |
| 2 | 1 | `0x00000002L` | `HAS_BARRACKS` | Barracks |
| 3 | 2 | `0x00000004L` | `HAS_GRANARY` | Granary |
| 4 | 3 | `0x00000008L` | `HAS_TEMPLE` | Temple |
| 5 | 4 | `0x00000010L` | `HAS_MARKETPLACE` | MarketPlace |
| 6 | 5 | `0x00000020L` | `HAS_LIBRARY` | Library |
| 7 | 6 | `0x00000040L` | `HAS_COURTHOUSE` | Courthouse |
| 8 | 7 | `0x00000080L` | `HAS_CITY_WALLS` | City Walls |
| 9 | 8 | `0x00000100L` | `HAS_AQUEDUCT` | Aqueduct |
| 10 | 9 | `0x00000200L` | `HAS_BANK` | Bank |
| 11 | 10 | `0x00000400L` | `HAS_CATHEDRAL` | Cathedral |
| 12 | 11 | `0x00000800L` | `HAS_UNIVERSITY` | University |
| 13 | 12 | `0x00001000L` | `HAS_MASS_TRANSIT` | Mass Transit |
| 14 | 13 | `0x00002000L` | `HAS_COLOSSEUM` | Colosseum |
| 15 | 14 | `0x00004000L` | `HAS_FACTORY` | Factory |
| 16 | 15 | `0x00008000L` | `HAS_MFG_PLANT` | Mfg. Plant |
| 17 | 16 | `0x00010000L` | `HAS_SDI` | SDI Defense |
| 18 | 17 | `0x00020000L` | `HAS_RECYCLING_CNTR` | Recycling Cntr. |
| 19 | 18 | `0x00040000L` | `HAS_POWER_PLANT` | Power Plant |
| 20 | 19 | `0x00080000L` | `HAS_HYDRO_PLANT` | Hydro Plant |
| 21 | 20 | `0x00100000L` | `HAS_NUCLEAR_PLANT` | Nuclear Plant |

Wonder names and ordering are documented in the [wonder catalog](../city/improvements_wonders.md).
