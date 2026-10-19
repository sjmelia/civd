# Caravan trade

Caravans are unit type 27. `CONTROL.C` handles their arrival and offers
trade or help with a wonder. `rpath()` in `CAPTURE.C` calculates the
delivery reward and updates route records.

Each city has three route slots in `routes`. A slot stores a destination
city index, with −1 for empty. Destroying a city clears routes that refer
to it. Routes link cities, rather than storing a fixed income in the save.

During `city_process()`, the city first records its trade after an initial
corruption calculation. It then visits its route slots in order. A foreign
route adds `(destination_trade + current_trade + 4) / 8`; a domestic route
uses a divisor of 16. Each addition changes `current_trade` for the next
slot. Corruption is recalculated before the national spending split.

For the one-time delivery reward, `rpath()` first calculates:

```text
reward = (map_dist(source, destination) + 10) * (source_trade + destination_trade) / 24
```

It halves the result when a region comparison succeeds, and halves it again
for a domestic destination. It then reduces the reward by a third for each
of Railroad and Flight known by the destination's owner. The result is added
to both treasury and research, and the Caravan is removed.

The region comparison contains a quirk: the source lookup uses the source
city's x coordinate for both x and y. It therefore does not reliably test
whether the two cities share a continent.
