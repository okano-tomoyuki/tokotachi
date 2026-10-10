// Node.js の環境変数を、WebAssembly の getenv から見えるようにする(Emscripten の ENV は既定では固定の値だけ)。
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  for (var key in process.env) {
    ENV[key] = process.env[key];
  }
});
