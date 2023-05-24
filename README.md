When I was working at [Delem](https://www.delem.com), there was a tradition that coworkers who married would receive a personalized device tailored to their specific hobbies.

The marriage target I was involved in the most was in 2011/2012, the *CC69*, which used the same hardware as the DA-69T control: an Intel Atom 1.6GHz processor, an Intel i915 GPU, a CompactFlash slot for storage, an on-board FPGA that was used to communicate with the custom hardware (buttons) and an IRTouch-based USB touch screen.

I decided to use this opportunity to go one step beyond the ordinary approach: the device would run Linux instead of Windows CE 6.0. My goal was to (1) show that Linux was a viable platform and could run on the hardware in use, (2) show that OpenGL could enhance the user interface by accelerating rendering and (3) prove that I had the skill to work on these projects. This paid off: the next line of controls, the DA-58T, used the i.MX6 processor on Linux!

This repository contains everything I salvaged from old copies/backups:

 * _buildroot_ contains the scripts used to build Buildroot for the device and install it.
 * _xf86-input-irtouch_ is a X11 input driver for the IRTouch USB touch screen.
 * _cc69_ contains the code/data for the application. It can show pictures and has a Tetris clone.

You should be able to build and run the _cc69_ application on Linux, provided you have the proper dependencies available (SDL, SDL_image, SDL_ttf, SDL_mixer, OpenGL (GL/GLU), boost (base, filesystem and thread). Boost was used both to speed up development (as C++11 was still unfamiliar territory at that point) and to try to gather goodwill among the developers to use it, which didn't happened at that time (compiling the _cc69_ application takes far longer than it should, which didn't exactly help things)

Unless otherwise specified, the code is licensed using [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). The legal status of the resources (images, fonts) is unknown - they were likely randomly scavaged from the internet. Do reach out if you have evidence indicating they should be removed.

*I'm releasing this for historical purposes: no kind of support is offered.*

**Note that most of this was thrown together in my spare time and is likely very buggy. If you care enough to fix bugs, I'll merge your pull requests!**