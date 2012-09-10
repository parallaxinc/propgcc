This demo shows how multiple COGs can be run in parallel using the
OpenMP API. When you "make" it, you'll get two different programs:
toggle.elf, and toggle_omp.elf. toggle.elf uses just 1 cog to toggle a
set of pins, toggle_omp.elf uses all 8 cogs. Since there is a delay
after each pin is toggled, the difference is dramatic and visible.

I recommend running this on a Quickstart board for full effect, or on
any board with mutiple LEDs set up.
