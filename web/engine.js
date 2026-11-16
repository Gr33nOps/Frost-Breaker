export const clamp = (value, low, high) => Math.max(low, Math.min(high, value));
const dot = (a, b) => a.x * b.x + a.y * b.y;
const add = (a, b) => ({ x: a.x + b.x, y: a.y + b.y });
const scale = (a, s) => ({ x: a.x * s, y: a.y * s });

function sweepCircle(start, movement, radius, center) {
  const delta = { x: start.x - center.x, y: start.y - center.y };
  const distance = Math.hypot(delta.x, delta.y),
    speed = dot(movement, movement);
  if (!speed) return null;
  if (distance < radius) {
    const normal =
      distance > 0.00001
        ? scale(delta, 1 / distance)
        : scale(movement, -1 / Math.sqrt(speed));
    return dot(movement, normal) < 0
      ? { time: 0, normal, depth: radius - distance }
      : null;
  }
  const approach = dot(delta, movement);
  const discriminant =
    approach * approach - speed * (dot(delta, delta) - radius * radius);
  if (approach >= 0 || discriminant <= 0) return null;
  let time = (-approach - Math.sqrt(discriminant)) / speed;
  if (time < -0.00001 || time > 1.00001) return null;
  time = clamp(time, 0, 1);
  return {
    time,
    normal: scale(add(delta, scale(movement, time)), 1 / radius),
    depth: 0,
  };
}

export function sweepRectangle(start, movement, radius, bounds) {
  const { x: left, y: top, w, h } = bounds,
    right = left + w,
    bottom = top + h;
  const end = add(start, movement);
  if (
    Math.max(start.x, end.x) < left - radius - 0.0001 ||
    Math.min(start.x, end.x) > right + radius + 0.0001 ||
    Math.max(start.y, end.y) < top - radius - 0.0001 ||
    Math.min(start.y, end.y) > bottom + radius + 0.0001
  )
    return null;
  const delta = {
    x: start.x - clamp(start.x, left, right),
    y: start.y - clamp(start.y, top, bottom),
  };
  const distance = Math.hypot(delta.x, delta.y);
  if (distance < radius) {
    let normal,
      depth = radius - distance;
    if (distance > 0.00001) normal = scale(delta, 1 / distance);
    else {
      const gaps = [
        start.x - left,
        right - start.x,
        start.y - top,
        bottom - start.y,
      ];
      const side = gaps.indexOf(Math.min(...gaps));
      normal = [
        { x: -1, y: 0 },
        { x: 1, y: 0 },
        { x: 0, y: -1 },
        { x: 0, y: 1 },
      ][side];
      depth = radius + gaps[side];
    }
    return dot(movement, normal) < 0 ? { time: 0, normal, depth } : null;
  }
  let hit = null;
  const face = (time, across, low, high, normal) => {
    if (
      time < -0.00001 ||
      time > 1.00001 ||
      across < low - 0.0001 ||
      across > high + 0.0001
    )
      return;
    time = clamp(time, 0, 1);
    if (!hit || time < hit.time) hit = { time, normal, depth: 0 };
  };
  if (movement.x > 0) {
    const t = (left - radius - start.x) / movement.x;
    face(t, start.y + movement.y * t, top, bottom, { x: -1, y: 0 });
  } else if (movement.x < 0) {
    const t = (right + radius - start.x) / movement.x;
    face(t, start.y + movement.y * t, top, bottom, { x: 1, y: 0 });
  }
  if (movement.y > 0) {
    const t = (top - radius - start.y) / movement.y;
    face(t, start.x + movement.x * t, left, right, { x: 0, y: -1 });
  } else if (movement.y < 0) {
    const t = (bottom + radius - start.y) / movement.y;
    face(t, start.x + movement.x * t, left, right, { x: 0, y: 1 });
  }
  for (const corner of [
    { x: left, y: top },
    { x: right, y: top },
    { x: left, y: bottom },
    { x: right, y: bottom },
  ]) {
    const candidate = sweepCircle(start, movement, radius, corner);
    if (candidate && (!hit || candidate.time < hit.time)) hit = candidate;
  }
  return hit;
}

export class Game {
  constructor(levels, best = 0) {
    this.levels = levels;
    this.best = Math.max(0, Number.isFinite(best) ? Math.floor(best) : 0);
    this.events = [];
    this.effects = [];
    this.trail = [];
    this.level = 1;
    this.score = 0;
    this.lives = 5;
    this.loadLevel();
    this.state = "menu";
  }
  sound(name) {
    this.events.push(name);
  }
  impact(kind, x, y, compact = false) {
    this.effects.push({
      kind,
      x: Math.round(x / 2) * 2,
      y: Math.round(y / 2) * 2,
      compact,
      age: 0,
    });
    if (this.effects.length > 48) this.effects.shift();
  }
  serve() {
    this.paddle = { x: 260, y: 550, target: null, direction: 0 };
    this.ball = {
      x: 260,
      y: 528.99,
      vx: 8 * 0.34202014,
      vy: -8 * 0.93969262,
      lava: false,
      lavaSeconds: 0,
      age: 0,
    };
    this.effects = [];
    this.trail = [];
    this.trailTime = 0;
    this.state = "ready";
  }
  loadLevel() {
    this.bricks = [];
    this.pickups = [];
    this.levels[this.level - 1].tiles.forEach((line, row) =>
      [...line].forEach((type, col) => {
        const brick = {
          x: col * 40,
          y: row * 20 + 60,
          w: 38,
          h: 18,
          type,
          destroyed: false,
        };
        if (type === "F" || type === "R") this.bricks.push(brick);
        else if (type === "L")
          this.pickups.push({ ...brick, collected: false });
      }),
    );
    this.serve();
  }
  start() {
    this.level = 1;
    this.score = 0;
    this.lives = 5;
    this.loadLevel();
    this.sound("start");
  }
  launch() {
    if (this.state === "ready") {
      this.state = "playing";
      this.sound("launch");
    }
  }
  stopInput() {
    this.paddle.target = null;
    this.paddle.direction = 0;
  }
  followMouse(x) {
    if (this.state !== "ready" && this.state !== "playing") return;
    this.paddle.target = x >= 0 && x < 520 ? clamp(x, 40, 480) : null;
    this.paddle.direction = 0;
  }
  pause() {
    if (this.state !== "playing" && this.state !== "pause") return;
    this.state = this.state === "playing" ? "pause" : "playing";
    this.stopInput();
    this.sound(this.state === "pause" ? "pause" : "resume");
  }
  addLava() {
    this.ball.lavaSeconds += 10;
    this.ball.lava = true;
  }
  advanceLava(seconds) {
    const b = this.ball;
    if (!b.lava || seconds <= 0) return;
    b.lavaSeconds = Math.max(0, b.lavaSeconds - seconds);
    if (b.lavaSeconds < 0.00001) {
      b.lava = false;
      b.lavaSeconds = 0;
      this.trail = [];
      this.trailTime = 0;
    }
  }
  moveBall(seconds, paddleMovement = 0) {
    const b = this.ball,
      duration = seconds * 60;
    if (duration <= 0) return;
    b.x = clamp(b.x, 10, 510);
    b.y = Math.max(b.y, 10);
    let position = { x: b.x, y: b.y },
      remaining = duration;
    for (let bounce = 0; bounce < 8 && remaining > 0; bounce++) {
      const segment = b.lava
        ? Math.min(remaining, b.lavaSeconds * 60)
        : remaining;
      const movement = { x: b.vx * segment, y: b.vy * segment };
      let first = 1,
        hits = [];
      const remember = (contact, kind, brick = null, pickup = null) => {
        if (!contact || contact.time < 0 || contact.time > 1) return;
        if (contact.time < first - 0.00001) {
          hits = [];
          first = contact.time;
        }
        if (Math.abs(contact.time - first) <= 0.00001)
          hits.push({ contact, kind, brick, pickup });
      };
      if (movement.x < 0)
        remember(
          {
            time: (10 - position.x) / movement.x,
            normal: { x: 1, y: 0 },
            depth: 0,
          },
          "wall",
        );
      if (movement.x > 0)
        remember(
          {
            time: (510 - position.x) / movement.x,
            normal: { x: -1, y: 0 },
            depth: 0,
          },
          "wall",
        );
      if (movement.y < 0)
        remember(
          {
            time: (10 - position.y) / movement.y,
            normal: { x: 0, y: 1 },
            depth: 0,
          },
          "wall",
        );
      const travel = (paddleMovement * segment) / duration;
      const bounds = {
        x: this.paddle.x - 40 - (paddleMovement * remaining) / duration,
        y: 539,
        w: 80,
        h: 22,
      };
      const paddleHit = sweepRectangle(
        position,
        { x: movement.x - travel, y: movement.y },
        10,
        bounds,
      );
      if (
        b.vy > 0 &&
        paddleHit &&
        paddleHit.normal.y < 0 &&
        position.y + movement.y * paddleHit.time <= 539
      )
        remember(paddleHit, "paddle");
      for (const brick of this.bricks) {
        if (!brick.destroyed)
          remember(
            sweepRectangle(position, movement, 10, brick),
            "brick",
            brick,
          );
      }
      for (const p of this.pickups) {
        if (p.collected) continue;
        const distance = Math.hypot(
          position.x - clamp(position.x, p.x, p.x + p.w),
          position.y - clamp(position.y, p.y, p.y + p.h),
        );
        remember(
          distance <= 10
            ? { time: 0, normal: { x: 0, y: 0 }, depth: 0 }
            : sweepRectangle(position, movement, 10, p),
          "pickup",
          null,
          p,
        );
      }
      position = add(position, scale(movement, first));
      remaining = Math.max(0, remaining - segment * first);
      this.advanceLava((segment * first) / 60);
      if (!hits.length) continue;
      for (const hit of hits)
        if (hit.kind === "pickup") {
          hit.pickup.collected = true;
          this.addLava();
          this.impact("LavaBurst", hit.pickup.x + 19, hit.pickup.y + 9);
          this.sound("lava-ignite");
        }
      let normal = { x: 0, y: 0 },
        paddle = false,
        wall = false;
      const solid = [];
      for (const hit of hits) {
        if (hit.kind === "pickup") continue;
        const contact = hit.contact;
        const impact = add(position, scale(contact.normal, contact.depth - 10));
        if (hit.kind === "brick") {
          if (hit.brick.type === "F") {
            hit.brick.destroyed = true;
            this.score += 10;
            this.best = Math.max(this.best, this.score);
            this.impact("IceBurst", impact.x, impact.y);
            this.sound("frost-shatter");
            if (b.lava) continue;
          } else {
            this.impact("RockChip", impact.x, impact.y, true);
            this.sound("rock-hit");
          }
        } else if (hit.kind === "paddle") {
          paddle = true;
          this.impact("RockChip", impact.x, impact.y, true);
          this.sound("paddle-knock");
        } else wall = true;
        normal = add(normal, contact.normal);
        solid.push(hit);
      }
      const length = Math.hypot(normal.x, normal.y);
      if (!length) continue;
      normal = scale(normal, 1 / length);
      if (paddle) {
        const center = this.paddle.x - (paddleMovement * remaining) / duration;
        const angle =
          (clamp((position.x - center) / 40, -1, 1) * 65 * Math.PI) / 180;
        b.vx = 8 * Math.sin(angle);
        b.vy = -8 * Math.cos(angle);
        for (const hit of solid.filter((hit) => hit.kind === "wall")) {
          const approach = dot({ x: b.vx, y: b.vy }, hit.contact.normal);
          if (approach < 0) {
            b.vx -= 2 * approach * hit.contact.normal.x;
            b.vy -= 2 * approach * hit.contact.normal.y;
          }
        }
      } else {
        const approach = dot({ x: b.vx, y: b.vy }, normal);
        b.vx -= 2 * approach * normal.x;
        b.vy -= 2 * approach * normal.y;
      }
      let separation = 0.001;
      for (const hit of solid) {
        const alignment = dot(normal, hit.contact.normal);
        if (alignment > 0)
          separation = Math.max(
            separation,
            (hit.contact.depth + 0.001) / alignment,
          );
      }
      position = add(position, scale(normal, separation));
      if (wall) this.sound("wall-tick");
    }
    b.x = position.x;
    b.y = position.y;
  }
  update(seconds) {
    if (seconds <= 0 || this.state === "menu" || this.state === "pause") return;
    seconds = Math.min(seconds, 0.1);
    for (const effect of this.effects) effect.age += seconds;
    this.effects = this.effects.filter((effect) => effect.age < 0.36);
    for (const dot of this.trail) dot.age += seconds;
    this.trail = this.trail.filter((dot) => dot.age < 0.16);
    if (this.state === "win" || this.state === "over") return;
    const steps = Math.max(
      1,
      Math.ceil(
        (Math.max(7, Math.hypot(this.ball.vx, this.ball.vy)) * seconds * 60) /
          2,
      ),
    );
    const dt = seconds / steps;
    for (let i = 0; i < steps; i++) {
      const p = this.paddle,
        oldX = p.x,
        distance = 420 * dt;
      if (p.direction) {
        p.target = null;
        p.x += p.direction * distance;
      } else if (p.target !== null)
        p.x += clamp(p.target - p.x, -distance, distance);
      p.x = clamp(p.x, 40, 480);
      this.ball.age += dt;
      if (this.state === "ready") {
        this.ball.x = p.x;
        this.ball.y = 528.99;
        continue;
      }
      this.moveBall(dt, p.x - oldX);
      if (this.ball.y - 10 > 600) {
        this.lives--;
        this.serve();
        if (this.lives <= 0) this.state = "over";
        this.sound(this.lives <= 0 ? "game-over" : "life-lost");
        return;
      }
      if (this.ball.lava) {
        this.trailTime += dt;
        if (this.trailTime >= 0.025) {
          this.trailTime %= 0.025;
          this.trail.push({
            x: Math.round(this.ball.x / 2) * 2,
            y: Math.round(this.ball.y / 2) * 2,
            age: 0,
          });
          if (this.trail.length > 12) this.trail.shift();
        }
      }
    }
    if (
      this.state === "playing" &&
      !this.bricks.some((brick) => brick.type === "F" && !brick.destroyed)
    ) {
      this.sound("level-clear");
      if (this.level < this.levels.length) {
        this.level++;
        this.loadLevel();
      } else {
        this.stopInput();
        this.state = "win";
      }
    }
  }
}
