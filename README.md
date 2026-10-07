# MicrobeWorld C++ / WebAssembly core

The simulation source in `microbeworld.cpp` is the canonical core architecture: organisms, player state, feeding, predator behavior, energy, health, growth, evolution, and time-based updates live here.

## Emscripten build

Install the Emscripten SDK locally, then:

```bash
emcmake cmake -S . -B build
emmake cmake --build build
```

For a browser module build, the source also exposes a small C ABI for Emscripten. A direct command is:

```bash
em++ microbeworld.cpp -O3 -sMODULARIZE=1 -sEXPORT_ES6=1 \
  -sEXPORTED_FUNCTIONS='["_mw_create","_mw_reset","_mw_set_input","_mw_update","_mw_evolve"]' \
  -o microbeworld.js
```

Copy the generated `microbeworld.js` and `microbeworld.wasm` beside the deployed web assets and wire the exported functions into `game.js`.

## Hatchable prototype note

Hatchable's hosted build environment does not expose an Emscripten compiler as a deploy-time build step. The deployed prototype therefore contains the C++/Wasm source and a self-contained browser simulation fallback so the game is playable immediately. The browser-side simulation is intentionally organized around the same state model and interaction rules as the C++ core, making the eventual Wasm swap a boundary change rather than a rewrite.