# Sysvol
Sysvol is a basic but functional volume indicator written in gtkmm4<br>
![preview](https://github.com/AmirDahan/sysvol/blob/main/preview.gif "preview")

# Configuration
sysvol can be configured in 2 ways<br>
1: By changing config.hpp and recompiling (Suckless style)<br>
2: Using launch arguments<br>
```
arguments:
  -p	Set position (0 = top | 1 = right | 2 = bottom | 3 = left)
  -W	Set window width
  -H	Set window Height
  -i	Set icon size
  -P	Hide percentage
  -m	Set margins
  -t	Set timeout
  -T	Set transition time (0 disables animations)
```

Pulseaudio support can be enabled instead of wireplumber by compiling using:<br>
```
make PULSEAUDIO=1
```

# Theming
sysvol uses your gtk4 theme by default, However it can be also load custom css,<br>
Just copy the included volume.css file to ~/.config/sys64/volume.css<br>

# Credits
Many thanks to [waybar](https://github.com/Alexays/Waybar) for showing how to write wireplumber stuff<br>
