## PARUPAINT 0.6D ALPHA PREVIEW
This is my painting program. You can paint with others! There's even a web client! See paruluna/parupaint-web for that.

It has support for wacom tablets.

Parupaint is pretty barebones, please don't be surprised if something breaks.

### FEATURES
- Multiplayer painting
- Websocket, allowing web clients
- Animation abilities
- Key focused navigation

### PLANNED FEATURES
- Multiple save/export options: webm, ora, parupaint archive (ppa), ...
- Undo/redo support in multiplayer
- Left-handed support (mirror the keys)

### KEYS

#### General keys
- Press `Tab` to show the overlay: color picker, flayer list, etc.
- Hold `Tab` to make it persistent
- Press `Shift` + `Tab` to hide the overlay
- Press `Ctrl` + `I` opens the connection dialog.
- Press `Ctrl` + `K` saves the image (at the moment only on the server)

#### Canvas keys
- Hold `Space` to move canvas.
- Hold `Ctrl` + `Space` to zoom.
- Hold `Shift` + `Space` to resize brush.
- Press `A`, or `F` to go backwards, forwards in frames.
- Press `S`, or `D` to go up, down in layers.
- Hold `Tab` with either of `A`, `S`, `D`, `F` to create, remove the respective things.
 - Example: `Tab` + `F` creates a new frame.
- Press `Q` to toggle preview mode.

#### Brush keys
- `Mouse wheel` to change brush size.
- Press `E` key to switch between eraser and brush. 
