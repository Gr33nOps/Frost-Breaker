# Publish Frost Breaker on itch.io

1. Create a game page titled **Frost Breaker**. Choose **HTML** as the kind of project.
2. Upload **frost-breaker-2.1.0-web.zip** and select **This file will be played in the browser**. Its `index.html` is already at the ZIP root; upload the ZIP directly.
3. Set the embed size to **800 × 600**, use **Embed in page**, and enable the **Fullscreen button**. Do not mark the game as mobile-friendly: this release is designed for mouse and keyboard.
4. Upload **frost-breaker-2.1.0-windows.zip** as an optional download and tick **Windows**.
5. Use `docs/media/itch-cover.png` as the **630 × 500** cover. Use `gameplay.png`, `menu.png`, `browser-gameplay.png` and the normal-ball screenshots in `docs/media/levels/` as screenshots. `campaign-preview.png` gives a compact overview of all ten formations. The press-kit ZIP contains these files too.
6. Paste the short description and page copy from [itch-page.md](itch-page.md), then preview the page and play before publishing.
7. Send the published URL so it can replace the pending play link in the README and GitHub's Website field.

The browser ZIP runs without a backend, external libraries, account, API key or cross-origin isolation settings. Audio starts after the first click/key press. High scores use browser storage; the game also works if storage is unavailable. Leaving the game pauses play.

## Downloads

| File | Purpose |
| --- | --- |
| `frost-breaker-2.1.0-web.zip` | Upload directly as the playable HTML game |
| `frost-breaker-2.1.0-windows.zip` | Extract and run `Play.cmd` on Windows 10/11 |
| `frost-breaker-2.1.0-press-kit.zip` | Cover, logo, icon, screenshots and page copy |
| `frost-breaker-2.1.0-pixel-art-source.zip` | Editable Aseprite/Piskel artwork and exports |
| `SHA256SUMS.txt` | SHA-256 hashes of the release downloads |

The Windows download is an unsigned x86 application and includes the required SFML, OpenAL and Visual C++ runtime DLLs. Extract the complete ZIP before starting it; the executable resolves artwork and sounds from the folder above `bin/`.

For a local browser preview: `python -m http.server 8000 --directory dist/web`, then open `http://localhost:8000`.

The upload structure follows [itch.io's HTML5 game requirements](https://itch.io/docs/creators/html5).
