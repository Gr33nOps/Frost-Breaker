export const imageNames = [
  "background",
  "menu background",
  "logo",
  "logo2",
  "Paddle",
  "Brick",
  "Rock",
  "overlay",
  "start",
  "start2",
  "resume",
  "resume2",
  "menu",
  "menu2",
  "play again",
  "play again2",
  "heart",
];
export const sheets = [
  "BallGlint",
  "LavaBallFlicker",
  "LavaPickupPulse",
  "IceBurst",
  "RockChip",
  "LavaBurst",
];

export async function loadImages() {
  const images = {};
  await Promise.all(
    [
      ...imageNames.map((name) => [name, `assets/images/${name}.png`]),
      ...sheets.map((name) => [name, `assets/animations/png/${name}.png`]),
    ].map(
      ([name, path]) =>
        new Promise((resolve, reject) => {
          const image = new Image();
          image.onload = () => {
            images[name] = image;
            resolve();
          };
          image.onerror = () => reject(new Error(`Could not load ${path}`));
          image.src = path;
        }),
    ),
  );
  return images;
}

export class View {
  constructor(canvas, images) {
    this.ctx = canvas.getContext("2d", { alpha: false });
    this.ctx.imageSmoothingEnabled = false;
    this.images = images;
    this.mouse = { x: -1, y: -1 };
    this.buttons = [];
  }
  sprite(name, x, y) {
    const image = this.images[name];
    this.ctx.drawImage(
      image,
      Math.round(x - image.width / 2),
      Math.round(y - image.height / 2),
    );
  }
  text(text, x, y, heading = false, color = null) {
    const ctx = this.ctx;
    ctx.font = `${heading ? 60 : 40}px Pixel`;
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.lineJoin = "miter";
    if (heading) {
      ctx.strokeStyle = "#284955";
      ctx.lineWidth = 4;
      ctx.strokeText(text, x, y);
    }
    ctx.fillStyle = color || (heading ? "#edf7f7" : "#bddfe5");
    ctx.fillText(text, x, y);
  }
  button(name, x, y, action) {
    const image = this.images[name],
      inside =
        Math.abs(this.mouse.x - x) < image.width / 2 &&
        Math.abs(this.mouse.y - y) < image.height / 2;
    this.sprite(inside ? name + "2" : name, x, y);
    this.buttons.push({
      x: x - image.width / 2,
      y: y - image.height / 2,
      w: image.width,
      h: image.height,
      action,
    });
  }
  frame(name, x, y, w, h, index, scale = 1) {
    this.ctx.drawImage(
      this.images[name],
      index * w,
      0,
      w,
      h,
      Math.round(x - (w * scale) / 2),
      Math.round(y - (h * scale) / 2),
      w * scale,
      h * scale,
    );
  }
  draw(game) {
    const ctx = this.ctx;
    this.buttons = [];
    ctx.drawImage(this.images.background, 0, 0);
    if (game.state === "menu") {
      ctx.drawImage(this.images["menu background"], 0, 0);
      this.sprite("logo", 400, 200);
      this.button("start", 400, 350, "start");
      return;
    }
    ctx.save();
    ctx.beginPath();
    ctx.rect(0, 0, 520, 600);
    ctx.clip();
    for (const brick of game.bricks)
      if (!brick.destroyed)
        ctx.drawImage(
          this.images[brick.type === "R" ? "Rock" : "Brick"],
          brick.x,
          brick.y,
        );
    if (!game.pickup.collected)
      this.frame(
        "LavaPickupPulse",
        game.pickup.x + 19,
        game.pickup.y + 9,
        38,
        18,
        Math.floor(game.ball.age / 0.14) % 4,
      );
    for (const dot of game.trail) {
      ctx.globalAlpha = 1 - dot.age / 0.16;
      ctx.fillStyle = dot.age < 0.06 ? "#ffd06b" : "#ef5d34";
      const size = dot.age < 0.08 ? 4 : 2;
      ctx.fillRect(dot.x - size / 2, dot.y - size / 2, size, size);
    }
    ctx.globalAlpha = 1;
    for (const effect of game.effects)
      this.frame(
        effect.kind,
        effect.x,
        effect.y,
        48,
        48,
        Math.min(5, Math.floor(effect.age / 0.06)),
        effect.compact ? 0.5 : 1,
      );
    this.sprite("Paddle", game.paddle.x, game.paddle.y);
    const b = game.ball;
    this.frame(
      b.lava ? "LavaBallFlicker" : "BallGlint",
      b.x,
      b.y,
      20,
      20,
      Math.floor(b.age / (b.lava ? 0.1 : 0.12)) % (b.lava ? 4 : 6),
    );
    ctx.restore();
    for (const [label, y, value] of [
      ["Level", 100, game.level],
      ["Score", 300, game.score],
      ["High Score", 400, game.best],
    ]) {
      this.text(label, 660, y, true);
      this.text(String(value), 660, y + 50, false, "#ffffff");
    }
    this.text("Lives", 660, 200, true);
    for (let i = 0; i < game.lives; i++)
      this.sprite("heart", 660 + (i - (game.lives - 1) / 2) * 44, 250);
    if (game.state === "playing") return;
    ctx.drawImage(this.images.overlay, 0, 0);
    if (game.state === "ready") {
      this.text("READY?", 260, 220, true);
      this.text("MOUSE / ARROWS TO MOVE", 260, 325);
      this.text("CLICK / SPACE TO LAUNCH", 260, 370);
      return;
    }
    this.text(
      game.state === "pause"
        ? "Paused"
        : game.state === "win"
          ? "You Win!"
          : "Game Over!",
      260,
      205,
      true,
    );
    if (game.state === "win") this.text("The valley is free!", 260, 245);
    this.button(
      game.state === "pause" ? "resume" : "play again",
      260,
      300,
      game.state === "pause" ? "resume" : "start",
    );
    this.button("menu", 260, 400, "menu");
  }
}
