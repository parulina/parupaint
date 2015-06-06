## PARUPAINT 0.75 ALPHA PREVIEW
This is my painting program. You can paint with others! And there's even a web client! See [parupaint-web](paruluna/parupaint-web) for that.

It has support for wacom tablets.

Parupaint is pretty barebones, please don't be surprised if something breaks.

### FEATURES
- Multiplayer painting
- Websocket, allowing web clients
- Animation abilities
- Key focused navigation
- Multiple save and export options:
 - Save as OpenRaster (ORA)
 - Save as Parupaint archive (PPA)
 - Export to PNG (PNG)
 - Export to PNG Sequence (ZIP)
 - Export to movie (AVI, ...?)


### TODO
- Undo/redo support in multiplayer
- Left-handed support (mirror the keys)
- Proper tutorial
- Create new canvas

### KEYS

#### General keys
- Press `Tab` to show the overlay: color picker, flayer list, etc.
- Hold `Tab` to make it persistent
- Press `Shift` + `Tab` to hide the overlay
- Press `Enter` to chat.
- Press `Ctrl` + `I` to open the connection dialog.
- Press `Ctrl` + `K` to quicksave the image as a png.
- Press `Ctrl` + `L` to save the canvas as something.
- Press `Ctrl` + `O` to open a file.

#### Canvas keys
- Hold `Space` to move canvas (you can also zoom with the mousewheel while holding it!).
- Hold `Ctrl` + `Space` to zoom.
- Hold `Shift` + `Space` to resize brush.
- Press `A`, or `F` to go backwards, forwards in frames.
- Press `S`, or `D` to go up, down in layers.
- Hold `Tab` with either of `A`, `S`, `D`, `F` to create, remove the respective things.
 - Example: `Tab` + `F` creates a new frame.
 - `Tab` + `A` removes the current frame.
- Press `Q` to toggle preview mode.

#### Brush keys
- `Mouse wheel` to change brush size.
- Press `E` key to switch between eraser and brush. 
- Press `Right mouse button` to also switch between eraser and brush.
