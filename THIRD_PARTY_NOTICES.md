# Third-party notices

HawkTile's own code is MIT licensed (see [LICENSE](LICENSE)). It contains one
piece of code that was not written for this project.

## Windhawk tool-mod launcher

`src/90_toolmod.cpp`, from the "Windhawk tool mod implementation" comment down
to the end of the file, is the standard snippet from the Windhawk wiki page
[Mods as tools: Running mods in a dedicated process](https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process).
Windhawk mod authors are meant to paste it into their mods, and it appears in
many mods in [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods).
HawkTile's copy was taken from
[command-on-window-focus.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/command-on-window-focus.wh.cpp)
by rom4ster, published under the MIT license.

HawkTile changes it in one place: when a previous HawkTile process still holds
the single-instance mutex, the new process waits up to 15 seconds for it to
exit instead of quitting, so recompiling the mod never leaves it stopped.

## Things HawkTile builds on but does not include

- **Windhawk** ([ramensoftware/windhawk](https://github.com/ramensoftware/windhawk)):
  the mod API (`windhawk_api.h`) and compiler come from your Windhawk install at
  build time and are not part of this repository.
- **Omarchy** ([omarchy.org](https://omarchy.org)): the default key assignments
  follow Omarchy's documented hotkeys, with Alt in place of Super. No code.
- **Hyprland** ([hyprland.org](https://hyprland.org)): command names follow
  Hyprland's documented dispatcher names. No code.
