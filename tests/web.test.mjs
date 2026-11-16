import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { Game, sweepRectangle } from "../web/engine.js";

const text = readFileSync(
  new URL("../src/Levels.cpp", import.meta.url),
  "utf8",
);
const levels = [
  ...text.matchAll(/\{"([^"]+)",\s*"([^"]+)",\s*\{\{([\s\S]*?)\}\}\}/g),
].map((m) => ({
  name: m[1],
  story: m[2],
  tiles: [...m[3].matchAll(/"([.FRL]+)"/g)].map((r) => r[1]),
}));
const near = (a, b, e = 0.002) =>
  assert.ok(Math.abs(a - b) < e, `${a} != ${b}`);
const play = () => {
  const g = new Game(levels);
  g.start();
  g.launch();
  return g;
};
const empty = () => {
  const g = play();
  g.bricks = [{ x: 0, y: 60, w: 38, h: 18, type: "F", destroyed: false }];
  g.pickup.collected = true;
  return g;
};

test("all ten native formations have one guarded lava brick", () => {
  assert.equal(levels.length, 10);
  for (const stage of levels) {
    assert.equal(stage.tiles.join("").split("L").length - 1, 1);
    const r = stage.tiles.findIndex((row) => row.includes("L")),
      c = stage.tiles[r].indexOf("L");
    for (const [dx, dy] of [
      [1, 0],
      [-1, 0],
      [0, 1],
      [0, -1],
    ])
      for (const distance of [1, 2])
        assert.equal(stage.tiles[r + dy * distance][c + dx * distance], "F");
  }
});
test("flat contact retains exact travel time", () => {
  const hit = sweepRectangle({ x: 50, y: 30 }, { x: 0, y: 50 }, 10, {
    x: 20,
    y: 60,
    w: 80,
    h: 20,
  });
  near(hit.time, 0.4);
  near(hit.normal.x, 0);
  near(hit.normal.y, -1);
});
test("corner contact has a round diagonal normal", () => {
  const hit = sweepRectangle({ x: 0, y: 0 }, { x: 30, y: 30 }, 10, {
    x: 20,
    y: 20,
    w: 40,
    h: 20,
  });
  near(hit.normal.x, -Math.SQRT1_2);
  near(hit.normal.y, -Math.SQRT1_2);
});
test("departing overlaps do not bounce again", () => {
  assert.equal(
    sweepRectangle({ x: 25, y: 52 }, { x: 0, y: -4 }, 10, {
      x: 20,
      y: 60,
      w: 80,
      h: 20,
    }),
    null,
  );
});
test("near corner misses are not square collisions", () => {
  assert.equal(
    sweepRectangle({ x: 10, y: 49 }, { x: 0, y: 2 }, 10, {
      x: 20,
      y: 60,
      w: 80,
      h: 20,
    }),
    null,
  );
});
test("wall bounce uses remaining travel at constant speed", () => {
  const g = empty();
  g.ball = { x: 508, y: 400, vx: 8, vy: 0, lava: false, age: 0 };
  g.moveBall(1 / 60, 0);
  near(g.ball.x, 503.999);
  near(g.ball.vx, -8);
  near(g.ball.vy, 0);
});
test("paddle center returns vertically", () => {
  const g = empty();
  g.ball = { x: 260, y: 525, vx: 0, vy: 8, lava: false, age: 0 };
  g.moveBall(1 / 60, 0);
  near(g.ball.vx, 0);
  near(g.ball.vy, -8);
  assert.ok(g.ball.y < 529);
});
test("paddle edge aims without changing speed", () => {
  const g = empty();
  g.ball = { x: 294, y: 525, vx: 0, vy: 8, lava: false, age: 0 };
  g.moveBall(1 / 60, 0);
  assert.ok(g.ball.vx > 5);
  assert.ok(g.ball.vy < 0);
  near(Math.hypot(g.ball.vx, g.ball.vy), 8);
});
test("paddle underside cannot rescue a missed ball", () => {
  const g = empty();
  g.ball = { x: 260, y: 567, vx: 0, vy: 8, lava: false, age: 0 };
  g.moveBall(1 / 60, 0);
  near(g.ball.vy, 8);
});
test("frost breaks and scores once", () => {
  const g = empty();
  g.bricks = [{ x: 240, y: 200, w: 38, h: 18, type: "F", destroyed: false }];
  g.ball = { x: 260, y: 231, vx: 0, vy: -8, lava: false, age: 0 };
  g.moveBall(1 / 60, 0);
  assert.equal(g.score, 10);
  assert.equal(g.bricks[0].destroyed, true);
  assert.ok(g.ball.vy > 0);
  g.moveBall(1 / 60, 0);
  assert.equal(g.score, 10);
});
test("lava pierces frost and bounces on permanent stone", () => {
  const g = empty();
  g.bricks = [{ x: 240, y: 200, w: 38, h: 18, type: "F", destroyed: false }];
  g.ball = { x: 260, y: 231, vx: 0, vy: -8, lava: true, age: 0 };
  g.moveBall(1 / 60, 0);
  assert.ok(g.ball.vy < 0);
  g.bricks = [{ x: 240, y: 200, w: 38, h: 18, type: "R", destroyed: false }];
  g.ball.y = 231;
  g.moveBall(1 / 60, 0);
  assert.ok(g.ball.vy > 0);
  assert.equal(g.bricks[0].destroyed, false);
  assert.equal(g.score, 10);
});
test("lava pickup is optional, rectangular, and gives no score or bounce", () => {
  const g = empty();
  g.pickup = { x: 240, y: 200, w: 38, h: 18, collected: false };
  g.ball = { x: 260, y: 231, vx: 0, vy: -8, lava: false, age: 0 };
  g.moveBall(1 / 60, 0);
  assert.equal(g.ball.lava, true);
  assert.equal(g.pickup.collected, true);
  assert.equal(g.score, 0);
  assert.ok(g.ball.vy < 0);
});
test("lava resets on life loss without respawning a collected pickup", () => {
  const g = empty();
  g.ball.lava = true;
  g.ball.y = 612;
  g.update(1 / 120);
  assert.equal(g.lives, 4);
  assert.equal(g.state, "ready");
  assert.equal(g.ball.lava, false);
  assert.equal(g.pickup.collected, true);
});
test("rocks and uncollected lava do not prevent completion", () => {
  const g = play();
  g.lives = 3;
  g.ball.lava = true;
  g.bricks = g.bricks.filter((b) => b.type === "R");
  g.update(1 / 120);
  assert.equal(g.level, 2);
  assert.equal(g.lives, 3);
  assert.equal(g.ball.lava, false);
  assert.equal(g.pickup.collected, false);
});
test("fifth miss ends the entire campaign and restart restores level one", () => {
  const g = empty();
  g.level = 8;
  g.score = 300;
  g.lives = 1;
  g.ball.y = 612;
  g.ball.lava = true;
  g.update(1 / 120);
  assert.equal(g.state, "over");
  assert.equal(g.ball.lava, false);
  g.start();
  assert.equal(g.level, 1);
  assert.equal(g.score, 0);
  assert.equal(g.lives, 5);
  assert.equal(g.state, "ready");
});
test("ten levels share the same lives and end with a win", () => {
  const g = play();
  g.lives = 2;
  for (let i = 1; i <= 10; i++) {
    assert.equal(g.level, i);
    g.bricks = [];
    g.update(1 / 120);
    if (i < 10) g.launch();
  }
  assert.equal(g.state, "win");
  assert.equal(g.lives, 2);
});
test("mouse movement has the keyboard speed cap and cannot enter sidebar", () => {
  const g = new Game(levels);
  g.start();
  g.followMouse(519);
  g.update(0.1);
  near(g.paddle.x, 302);
  g.followMouse(900);
  g.update(0.1);
  near(g.paddle.x, 302);
  g.followMouse(0);
  for (let i = 0; i < 200; i++) g.update(1 / 120);
  near(g.paddle.x, 40);
  near(g.ball.x, 40);
});
test("pause clears mouse movement and freezes physics", () => {
  const g = play();
  g.followMouse(400);
  g.pause();
  const old = { ...g.ball };
  g.update(0.1);
  assert.deepEqual(g.ball, old);
  g.pause();
  const x = g.paddle.x;
  g.update(1 / 120);
  near(g.paddle.x, x);
});
test("moving paddle uses its position at contact", () => {
  const g = empty();
  g.paddle.x = 264;
  g.ball = { x: 260, y: 525, vx: 0, vy: 8, lava: false, age: 0 };
  g.moveBall(1 / 60, 4);
  const center = 260 + 4 * 0.5;
  near(g.ball.vx, 8 * Math.sin((((260 - center) / 40) * 65 * Math.PI) / 180));
});
