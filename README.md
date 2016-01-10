
This is a C99 implementation of 1D Markov localization in a looped world. It
was intended to be applied to a flywheel, but there's nothing about it that
makes it necessarily so.

The code was written to only use the stack. I intend to use it in an embedded
environment where dynamic/heap allocation is expensive. This means you probably
shouldn't define a world map with thousands of locations, sorry.

I have included two sets of test data:

The first is purely synthetic, and is obviously only for basic functional
testing. It simulates a 4-1 toothed wheel.

The second is a set of real data from a Microsquirt user who was having trouble
with noisy input. Do not expect the localizer to make complete sense of it.
