# Syshud
Syshud is a simple system status indicator<br>
![preview](https://github.com/System64fumo/syshud/blob/qt-rewrite/preview.png "preview")<br>
[![Packaging status](https://repology.org/badge/vertical-allrepos/syshud.svg)](https://repology.org/project/syshud/versions)

# Configuration
syshud can be configured in 2 ways<br>
1: By changing config.hpp and recompiling (Suckless style)<br>
2: Using launch arguments<br>
```
arguments:
  -p	Set position (eg: top, top-left, bottom, right)
  -o	Set orientation (eg: v, h)
  -W	Set window width
  -H	Set window Height
  -i	Set icon size
  -P	Hide percentage
  -m	Set margins ("top right bottom left")
  -t	Set timeout (In milliseconds)
  -b	Set custom backlight path
  -l	Set things to monitor (speakers, microphone, backlight, keyboard)
  -k	Set keyboard path (/dev/input/by-id/my_keyboard-event-kbd)
  -v	Prints version info
```

To use pulseaudio instead of wireplumber,<br>
change `#define FEATURE_WIREPLUMBER` to `#define FEATURE_PULSEAUDIO` in `src/config.hpp`

# Theming
syshud uses your QT6 theme in addition to a custom style in `/usr/share/sys64/hud/style.qss`.<br>
Should you choose to override the theme just copy it over to ~/.config/sys64/hud/style.qss and adjust it.<br>

# Known bugs/issues
There is no slide animation, No clue if i can re-implement that.<br>
There is no drop shadow effect, QSS does not offer that, Might have to manually add support for that.<br>
There are no custom value QSS object names.<br>
The wireplumber implementation is unstable, A pipewire rewrite is in mind but unlikely to happen anytime soon.<br>

# Credits
[Jason White](https://gist.github.com/jasonwhite/1df6ee4b5039358701d2) for showing how to write pulseaudio stuff<br>
[waybar](https://github.com/Alexays/Waybar) for showing how to write wireplumber stuff<br>
