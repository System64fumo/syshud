# Syshud
Syshud is a simple system status indicator<br>
![preview](https://github.com/System64fumo/syshud/blob/qt-rewrite/preview.png "preview")<br>
[![Packaging status](https://repology.org/badge/vertical-allrepos/syshud.svg)](https://repology.org/project/syshud/versions)

> [!NOTE]  
> This project has been rewritten to use QT6 instead of GTKMM 4<br>
> Some features may be missing.<br>

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
  -t	Set timeout
  -T	Set transition time (0 disables animations)
  -b	Set custom backlight path
  -M	Set things to monitor (audio_in, audio_out, brightness, keyboard)
  -k	Set keyboard path (/dev/input/by-id/my_keyboard-event-kbd)
  -v	Prints version info
```

To use pulseaudio instead of wireplumber,<br>
change `#define AUDIO_WIREPLUMBER` to `#define AUDIO_PULSEAUDIO` in `src/config.hpp`

# Theming
syshud uses your QT6 theme in addition to a custom style in `/usr/share/sys64/hud/style.qss`.<br>
Should you choose to override the theme just copy it over to ~/.config/sys64/hud/style.qss and adjust it.<br>

# Known bugs/issues
There is no slide animation, No clue if i can re-implement that.<br>
There are no custom value QSS object names.<br>
Compile time selection of features is unavailable ATM.<br>
All listeners are active even if you haven't enabled them in the config.<br>
Vertical style seems to be inverted.<br>
Label seems to adjust it's size more than it should.<br>
Pulse audio support has not been ported yet.<br>

# Credits
[Jason White](https://gist.github.com/jasonwhite/1df6ee4b5039358701d2) for showing how to write pulseaudio stuff<br>
[waybar](https://github.com/Alexays/Waybar) for showing how to write wireplumber stuff<br>
