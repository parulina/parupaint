## PARUPAINT ALPHA
This is my painting program! Concept in development since 2012.

### FEATURES
- Multiplayer painting using websocket, allowing web clients
- Animation support, using normal and extended frames
- Key focused navigation with minimal hinderance of the interface
- Uses ParuPaint Archive (PPA), an zipfile retaining animation information
- Can load PNG, JPG, GIF, ORA
- Can export to PNG, JPG and everything the ffmpeg libraries support

### BUILD & INSTALL
parupaint is developed on Qt5. A simple `qmake && make release` is enough to build the project, provided you got the dependencies for the default configuration:
`qt5-base ffmpeg`

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

### MANUAL
For an list of default keybinds and a tutorial, please refer to the [main site](http://parupaint.sqnya.se).
