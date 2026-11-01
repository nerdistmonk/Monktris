# Monktris

An authentic, retro-faithful falling-block puzzle engine built in C11 using **SDL3** and `libvorbisfile`. 

---

## Features

- High Score Tables
- 3 Unique Music Tracks
- Colorful design
- Gamepad support

---

## Default Controls

| Action | Keyboard | Gamepad (Standard Layout) |
|---|---|---|
| **Move Left / Right** | Left / Right Arrow | D-Pad Left / Right or Left Stick |
| **Soft Drop** | Down Arrow | D-Pad Down or Left Stick Down |
| **Rotate Clockwise (A)** | `X` | South Button (`A` / Cross) or East Button (`B` / Circle) |
| **Rotate Counter-Clockwise (B)** | `Z` | West Button (`X` / Square) |
| **Start / Pause** | `Enter` | `Start` |
| **Select** | Right `Shift` | `Back` / `Select` |
| **Toggle Fullscreen** | `F11` | — |

---

## Prerequisites (Linux / Debian / Ubuntu)

Install the required build tools, SDL3 development packages, and Vorbis libraries:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libvorbis-dev libogg-dev
```

*Note: Monktris targets **SDL3**. Ensure SDL3 development libraries (`libsdl3-dev` or source installation) are installed and discoverable by CMake.*

---

## Building and Running

1. Clone or navigate to the project directory:
   ```bash
   cd Monktris
   ```

2. Generate the build files:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```

3. Compile the executable:
   ```bash
   cmake --build build
   ```

4. Run the game:
   ```bash
   ./build/monktris
   ```

*(The CMake build process automatically copies the `snd/` audio assets and `gamecontrollerdb.txt` directly to the `build/` directory.)*

---

## Project Structure

```
Monktris/
├── CMakeLists.txt         # Build configuration and asset staging
├── README.md              # Project documentation
├── gamecontrollerdb.txt   # Gamepad mappings database
├── snd/                   # Vorbis audio tracks (1.ogg, 2.ogg, etc.)
└── src/
    ├── main.c             # SDL3 windowing, loop, and input dispatcher
    ├── tetris_game.c      # Main game state machine and menu flow
    ├── tetris_game.h      # Memory layout, state registers, and button bitmasks
    ├── tetris_tables.h    # Mino offsets and orientation lookup tables
    ├── playfield.c        # Collision detection and playfield buffer ops
    ├── piece_control.c    # DAS movement, gravity timers, and piece lock
    ├── spawn_prng.c       # 16-bit LFSR PRNG and piece spawn selection
    ├── line_clear.c       # 20-frame curtain animation and stack collapse
    ├── hud_score.c        # BCD arithmetic and statistics counters
    ├── ppu.c              # Software rendering, mosaic generator, and font
    ├── ppu.h              # Renderer interface
    ├── nes_palette.h      # Master 64-color palette and piece colors
    ├── apu.c              # Vorbis music streaming and procedural synth SFX
    ├── apu.h              # Audio subsystem interface
    ├── scores.c           # High score persistence and initials handling
    └── scores.h           # High score data structures
```