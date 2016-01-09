## PARUPAINT ALPHA
This is my painting program! Concept in development since 2012.

### FEATURES
- Multiplayer painting using websocket, allowing web clients
- Animation support, using normal and extended frames
- Key focused navigation with minimal hinderance of the interface
- Uses ParuPaint Archive (PPA), an ora-compatible zipfile retaining animation information
- Can load PNG, JPG, GIF, ORA
- Can export to PNG, JPG and everything the ffmpeg libraries support

### BUILD & INSTALL
parupaint is developed on Qt5. A simple `qmake && make release` is enough to build the project, provided you got the dependencies for the default configuration:
`qt5-base qt5-websockets ffmpeg`

If you wish to customize the build process, run qmake with any of these -config switches:
```
noffmpeg - compile without ffmpeg support (no ffmpeg plugin export)
nogui - compile without gui support (standalone server)
noxml - compile without any xml support (no ORA support)
```
If you successfully build parupaint, but unable to export despite ffmpeg libraries loading correctly, make sure both the dynamic libraries and parupaint are built with the same version of the ffmpeg headers.

### CONFIG
Parupaint uses a configuration file found in the application data folders: `%APPDATA%/paru/parupaint.ini` on windows, `~/.config/paru/parupaint.ini` on linux.

```conf
;You can load the ffmpeg libraries through custom paths:
[library]
avformat=/path...
avutil=/path...
avcodec=/path...
swscale=/path...
av_loglevel=0
av_bitrate=100000
```

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
- Press `Ctrl` + `M` to set parupaint settings.

#### Canvas keys
- Hold `Space` to move canvas (you can also zoom with the mousewheel while holding it!).
- Hold `Ctrl` + `Space` to zoom.
- Hold `Shift` + `Space` to resize brush.
- Press `A`, or `F` to go backwards, forwards in frames.
- Press `S`, or `D` to go up, down in layers.
- Hold `Tab` with either of `A`, `S`, `D`, `F` to create, remove the respective things.
 - Example: `Tab` + `F` creates a new frame. `Tab` + `A` removes the current frame.
 - Holding `Shift` inserts the new thing at the position rather than appends.
 - Holding `Ctrl` redacts (A) or extends (S) the current frame.
- Press `G` to toggle preview mode.

#### Brush keys
- `Mouse wheel` to change brush size.
- Press `1-5`/`9-5` keys to select either of the 5 custom brushes.
- Press `E` key or `Right mouse button` to switch between eraser and brush. 
- Press `Right mouse button` to also switch between eraser and brush.
- Press `W` for dot pattern tool.
- Press `Q` for flood fill tool.
- Press `Ctrl` + `W` for line tool.
- Press `Ctrl` + `Q` for opacity drawing tool.
