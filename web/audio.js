const sfxNames = [
  "paddle-knock",
  "frost-shatter",
  "rock-hit",
  "lava-ignite",
  "launch",
  "game-over",
  "level-clear",
  "start",
  "menu-back",
  "life-lost",
  "resume",
  "pause",
  "wall-tick",
];
const volume = {
  "wall-tick": 0.25,
  launch: 0.3,
  "rock-hit": 0.45,
  "lava-ignite": 0.45,
};

export class AudioPlayer {
  constructor() {
    this.context = null;
    this.muted = false;
    this.focused = true;
    this.buffers = new Map();
    this.voices = new Map();
    this.tracks = [];
  }
  async unlock() {
    if (this.context) {
      await this.context.resume();
      return;
    }
    const AudioContext = window.AudioContext || window.webkitAudioContext;
    if (!AudioContext) return;
    this.context = new AudioContext();
    this.master = this.context.createGain();
    this.master.connect(this.context.destination);
    this.master.gain.value = this.muted ? 0 : 1;
    await this.context.resume();
    const load = async (name, path) => {
      try {
        const response = await fetch(path);
        if (!response.ok) throw new Error(`Audio ${response.status}: ${path}`);
        this.buffers.set(
          name,
          await this.context.decodeAudioData(await response.arrayBuffer()),
        );
      } catch (error) {
        console.warn(error.message);
      }
    };
    await Promise.all([
      ...sfxNames.map((name) => load(name, `assets/sounds/sfx/${name}.wav`)),
      load("menu", "assets/sounds/music/snowbound-menu.ogg"),
      load("game", "assets/sounds/music/glacier-run.ogg"),
    ]);
    for (const name of ["menu", "game"]) {
      if (!this.buffers.has(name)) continue;
      const source = this.context.createBufferSource(),
        gain = this.context.createGain();
      source.buffer = this.buffers.get(name);
      source.loop = true;
      gain.gain.value = 0;
      source.connect(gain);
      gain.connect(this.master);
      source.start();
      this.tracks.push({ name, gain });
    }
  }
  play(name) {
    if (!this.context || !this.focused || this.muted || !this.buffers.has(name))
      return;
    this.voices.get(name)?.stop();
    const source = this.context.createBufferSource(),
      gain = this.context.createGain();
    source.buffer = this.buffers.get(name);
    gain.gain.value = volume[name] ?? 0.4;
    source.connect(gain);
    gain.connect(this.master);
    source.onended = () => {
      if (this.voices.get(name) === source) this.voices.delete(name);
      source.disconnect();
      gain.disconnect();
    };
    this.voices.set(name, source);
    source.start();
  }
  update(state) {
    if (!this.context) return;
    for (const track of this.tracks) {
      const playing =
        this.focused &&
        (track.name === "menu"
          ? state === "menu"
          : state === "ready" || state === "playing");
      track.gain.gain.setTargetAtTime(
        playing ? (track.name === "menu" ? 0.25 : 0.28) : 0,
        this.context.currentTime,
        0.12,
      );
    }
  }
  mute() {
    this.muted = !this.muted;
    if (this.context)
      this.master.gain.setTargetAtTime(
        this.muted ? 0 : 1,
        this.context.currentTime,
        0.02,
      );
  }
  focus(focused) {
    this.focused = focused;
    if (!this.context) return;
    if (!focused) {
      for (const source of this.voices.values()) source.stop();
      this.voices.clear();
      this.context.suspend();
    } else this.context.resume().catch(() => {});
  }
}
