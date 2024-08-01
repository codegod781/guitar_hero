# guitar_hero
Kiryl Beliauski (kb3338), Patrick Cronin (pjc2192), & Dan Ivanovich (dmi2115)
CSEE 4840 at Columbia University, Spring 2024
# Abstract - guitar_hero
For our final project, we chose to recreate a level from Activision’s Guitar Hero, using
their original controller hardware with our FPGA. Our project had four main components: 1) the
guitar controller 2) VGA graphics 3) game logic/software 4) audio. While nowhere near as
polished as the original game, we were able to reproduce the core gameplay for a single level.
Unfortunately, difficulties with campus access meant we were unable to get the audio component
fully working in time, but the final results of the other components are robust and complete. We
made hardware modifications to the controller, designed and implemented custom hardware
modules in Verilog for processing the controller input and outputting to VGA, custom kernel
modules to communicate with these modules, game logic, a custom sprite-rendering system, and
a small set of development tools to automate tedious parts of the development process. We even
implemented a complete emulation system that allowed us to develop graphics and gameplay
without access to the lab — the full game is completely playable in emulation.

# Final Report
https://www.cs.columbia.edu/~sedwards/classes/2024/4840-spring/reports/Guitar-Hero-report.pdf

