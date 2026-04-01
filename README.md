# Deskible

Deskible is a lightweight desktop Bible verse overlay window for Qt. It stays behind all other windows, providing a peaceful and non-intrusive way to keep the Word of God on your desktop.

## Installation & Data
1. Place `kjv.txt` at `<project-root>/bibleversions/kjv.txt`.
2. The source for the default Bible text is [www.BibleProtector.com](http://www.BibleProtector.com) (Pure Cambridge Edition).
3. To add more versions, simply drop additional `.txt` files into the `bibleversions/` folder.

## Build Instructions
### Using Qt Creator
1. Open `Deskible.pro` in Qt Creator.
2. Build and run.

### Using Command Line
```bash
qmake && make
```

## Usage
- **Drag**: Left-click and drag the window anywhere on your desktop.
- **Refresh**: Double-click the window to fetch a new random verse.
- **Menu**: Right-click the window or the tray icon for options (Next, Previous, Random, Settings, Quit).
- **Tray**: Click the tray icon to quickly show or hide the overlay.

## Settings
Deskible offers several customization options in the Settings dialog:
- **Auto-Switching**: Configure the interval (default 60s) for automatic verse changes.
- **Font & Style**: Choose any installed font, font size, and custom colors for verses and references.
- **Window Opacity**: Adjust transparency levels (20% - 100%).
- **Layout**: Set the maximum width of the overlay.
- **Startup**: Enable or disable "Launch at system startup".

## Operating System Support
- **Windows 10+**: Full support via Registry entry for startup.
- **macOS 11+**: Supported via LaunchAgents.
- **Linux**: Supported via `.desktop` file in autostart. Note: `WindowStaysOnBottomHint` behavior may vary depending on the Compositor (X11 vs Wayland).

## Known Issues
- `WindowStaysOnBottomHint` behavior varies across operating systems and desktop environments. On some Linux distributions, the window might not stay strictly behind all others.

## License
This project is licensed under Apache 2.0 License - see the [LICENSE](LICENSE) file for details.
