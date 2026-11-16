import { Game } from "./engine.js";
import { loadImages, View } from "./view.js";
import { AudioPlayer } from "./audio.js";

const canvas = document.querySelector("#screen"),
  status = document.querySelector("#status");
const audio = new AudioPlayer(),
  keys = new Set();
let game,
  view,
  savedBest = 0,
  accumulator = 0,
  lastTime = 0;
try {
  savedBest = Number(localStorage.getItem("frost-breaker-best")) || 0;
} catch {}
try {
  const [images, response] = await Promise.all([
    loadImages(),
    fetch("levels.json"),
    document.fonts.load("40px Pixel"),
  ]);
  if (!response.ok) throw new Error("Could not load levels");
  game = new Game(await response.json(), savedBest);
  view = new View(canvas, images);
  status.hidden = true;
  requestAnimationFrame(tick);
} catch (error) {
  status.textContent = "Could not load Frost Breaker. Please reload.";
  console.error(error);
}

function point(event) {
  const rect = canvas.getBoundingClientRect();
  return {
    x: ((event.clientX - rect.left) * 800) / rect.width,
    y: ((event.clientY - rect.top) * 600) / rect.height,
  };
}
function clearInput() {
  keys.clear();
  game?.stopInput();
}
function focus(focused) {
  clearInput();
  accumulator = 0;
  audio.focus(focused);
  if (!focused && game?.state === "playing") game.pause();
}
function action(name) {
  clearInput();
  if (name === "start") game.start();
  else if (name === "resume") game.pause();
  else if (name === "menu") {
    game.state = "menu";
    game.sound("menu-back");
  }
}
canvas.addEventListener("pointermove", (event) => {
  if (!game) return;
  const p = point(event);
  view.mouse = p;
  if (p.y >= 0 && p.y < 600) game.followMouse(p.x);
  else game.stopInput();
  keys.clear();
});
canvas.addEventListener("pointerleave", () => {
  if (view) view.mouse = { x: -1, y: -1 };
  clearInput();
});
canvas.addEventListener("pointerdown", (event) => {
  if (!game || event.button !== 0) return;
  event.preventDefault();
  canvas.focus();
  audio.unlock().catch((error) => console.warn(error.message));
  const p = point(event);
  const button = view.buttons.find(
    (b) => p.x >= b.x && p.x <= b.x + b.w && p.y >= b.y && p.y <= b.y + b.h,
  );
  if (button) {
    action(button.action);
    return;
  }
  if (game.state === "ready" && p.x >= 0 && p.x < 520 && p.y >= 0 && p.y < 600)
    game.launch();
});
canvas.addEventListener("contextmenu", (event) => event.preventDefault());
canvas.addEventListener("keydown", (event) => {
  if (
    !game ||
    !["ArrowLeft", "ArrowRight", "Space", "Escape", "KeyM"].includes(event.code)
  )
    return;
  event.preventDefault();
  audio.unlock().catch(() => {});
  if (event.repeat) return;
  keys.add(event.code);
  if (event.code === "ArrowLeft" || event.code === "ArrowRight") {
    game.paddle.target = null;
    game.paddle.direction =
      Number(keys.has("ArrowRight")) - Number(keys.has("ArrowLeft"));
  } else if (event.code === "Space") {
    if (game.state === "menu" || game.state === "over" || game.state === "win")
      action("start");
    else game.launch();
  } else if (event.code === "Escape") {
    clearInput();
    game.pause();
  } else document.querySelector("#sound").click();
});
canvas.addEventListener("keyup", (event) => {
  keys.delete(event.code);
  if (game && (event.code === "ArrowLeft" || event.code === "ArrowRight"))
    game.paddle.direction =
      Number(keys.has("ArrowRight")) - Number(keys.has("ArrowLeft"));
});
canvas.addEventListener("blur", () => focus(false));
canvas.addEventListener("focus", () => focus(true));
window.addEventListener("blur", () => focus(false));
window.addEventListener("focus", () => focus(true));
document.addEventListener("visibilitychange", () => focus(!document.hidden));
document.querySelector("#sound").addEventListener("click", (event) => {
  audio.unlock().catch(() => {});
  audio.mute();
  event.currentTarget.setAttribute("aria-pressed", String(audio.muted));
  event.currentTarget.setAttribute(
    "aria-label",
    audio.muted ? "Unmute audio" : "Mute audio",
  );
  event.currentTarget.title = audio.muted ? "Unmute audio" : "Mute audio";
  event.currentTarget.textContent = audio.muted ? "×" : "♪";
  canvas.focus();
});
document.querySelector("#fullscreen").addEventListener("click", async () => {
  try {
    if (document.fullscreenElement) await document.exitFullscreen();
    else await document.querySelector("#game").requestFullscreen();
  } catch (error) {
    console.warn(error.message);
  }
  canvas.focus();
});
function tick(time) {
  const elapsed = lastTime ? Math.min((time - lastTime) / 1000, 0.1) : 0;
  lastTime = time;
  if (!document.hidden) {
    accumulator += elapsed;
    while (accumulator >= 1 / 120) {
      game.update(1 / 120);
      accumulator -= 1 / 120;
    }
    for (const sound of new Set(game.events)) audio.play(sound);
    game.events.length = 0;
    audio.update(game.state);
    if (game.best !== savedBest) {
      savedBest = game.best;
      try {
        localStorage.setItem("frost-breaker-best", String(savedBest));
      } catch {}
    }
    view.draw(game);
    canvas.style.cursor = view.buttons.some(
      (b) =>
        view.mouse.x >= b.x &&
        view.mouse.x <= b.x + b.w &&
        view.mouse.y >= b.y &&
        view.mouse.y <= b.y + b.h,
    )
      ? "pointer"
      : "default";
  }
  requestAnimationFrame(tick);
}
