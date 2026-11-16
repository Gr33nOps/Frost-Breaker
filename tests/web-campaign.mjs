import assert from "node:assert/strict";
import { readFileSync, writeFileSync, mkdirSync } from "node:fs";
import { Game, clamp } from "../web/engine.js";

const source = readFileSync(
  new URL("../src/Levels.cpp", import.meta.url),
  "utf8",
);
const levels = [
  ...source.matchAll(/\{"([^"]+)",\s*"([^"]+)",\s*\{\{([\s\S]*?)\}\}\}/g),
].map((m) => ({
  name: m[1],
  tiles: [...m[3].matchAll(/"([.FRL]+)"/g)].map((r) => r[1]),
}));
const report = ["mode,level,seconds,lives"];
for (const mode of ["normal", "pickups"]) {
  const game = new Game(levels);
  game.start();
  for (let level = 1; level <= 10; level++) {
    if (mode === "normal") game.pickup.collected = true;
    let frame = 0,
      lastProgress = 0,
      previousScore = game.score,
      returns = 0;
    for (
      ;
      frame < 240 * 360 &&
      game.level === level &&
      game.state !== "win" &&
      game.state !== "over";
      frame++
    ) {
      game.launch();
      const b = game.ball;
      let catchX = b.x;
      if (b.vy > 0) {
        const travel = Math.max(0, (529 - b.y) / b.vy);
        let folded = (b.x - 10 + b.vx * travel) % 1000;
        if (folded < 0) folded += 1000;
        catchX = 10 + (folded <= 500 ? folded : 1000 - folded);
      }
      let target = { x: 260, y: 80 },
        best = Infinity;
      for (const brick of game.bricks)
        if (brick.type === "F" && !brick.destroyed) {
          const x = brick.x + 19,
            y = brick.y + 9,
            distance = (x - catchX) ** 2 + (539 - y) ** 2;
          if (distance < best) {
            best = distance;
            target = { x, y };
          }
        }
      let angle = Math.atan2(target.x - catchX, 529 - target.y);
      if ((frame - lastProgress) / 240 > 4)
        angle =
          ((2 * ((returns * 0.61803398875 + level * 0.1) % 1) - 1) *
            52 *
            Math.PI) /
          180;
      const offset = clamp(angle / ((65 * Math.PI) / 180), -0.85, 0.85) * 40;
      game.followMouse(Math.round(clamp(catchX - offset, 40, 480)));
      const approaching = b.y > 500 && b.vy > 0;
      game.update(1 / 240);
      game.events.length = 0;
      if (approaching && game.ball.vy < 0) returns++;
      if (game.score !== previousScore) {
        previousScore = game.score;
        lastProgress = frame;
      }
    }
    assert.ok(
      game.level !== level || game.state === "win",
      `${mode} level ${level}: ${game.state}, ${game.lives} lives`,
    );
    report.push(`${mode},${level},${(frame / 240).toFixed(2)},${game.lives}`);
    console.log(
      `PASS browser ${mode} level ${level}: ${game.lives} lives left`,
    );
  }
  assert.equal(game.state, "win");
  assert.ok(game.lives > 0);
}
mkdirSync(new URL("../build", import.meta.url), { recursive: true });
writeFileSync(
  new URL("../build/web-campaign.csv", import.meta.url),
  report.join("\n") + "\n",
);
