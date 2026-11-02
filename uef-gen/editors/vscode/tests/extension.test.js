"use strict";

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");

function harness({ enabled = true, start } = {}) {
  const commands = new Map();
  const clients = [];
  const settings = { "languageServer.enabled": enabled };
  const status = { show() { this.visible = true; }, hide() { this.visible = false; }, dispose() {} };
  const disposable = () => ({ dispose() {} });
  let outputCount = 0;
  const vscode = {
    DiagnosticSeverity: { Error: 0, Warning: 1 },
    StatusBarAlignment: { Right: 2 },
    SemanticTokensLegend: class {},
    languages: {
      createDiagnosticCollection: () => ({ ...disposable(), delete() {}, clear() {}, set() {} }),
      getDiagnostics: () => [], onDidChangeDiagnostics: disposable,
      registerDocumentSemanticTokensProvider: disposable, registerCompletionItemProvider: disposable, registerHoverProvider: disposable,
    },
    workspace: {
      workspaceFolders: [], textDocuments: [], getConfiguration: () => ({ get: (key, fallback) => settings[key] ?? fallback }),
      onDidOpenTextDocument: disposable, onDidChangeTextDocument: disposable, onDidCloseTextDocument: disposable,
      onDidChangeConfiguration: disposable, onDidChangeWorkspaceFolders: disposable, onDidSaveTextDocument: disposable,
    },
    window: {
      activeTextEditor: { document: { languageId: "uef-project", uri: "file:///demo.uef" } },
      createStatusBarItem: () => status,
      createOutputChannel: () => { outputCount += 1; return { ...disposable(), appendLine() {}, show() {} }; },
      onDidChangeActiveTextEditor: disposable, showWarningMessage() {},
    },
    commands: { registerCommand: (name, fn) => { commands.set(name, fn); return disposable(); }, executeCommand() {} },
  };
  class Client {
    constructor() { clients.push(this); this.disposed = false; }
    async start() { if (start) await start(); }
    async dispose() { this.disposed = true; }
    onDidChangeState(fn) { this.changeState = fn; return disposable(); }
  }
  const context = vm.createContext({
    require: (name) => name === "vscode" ? vscode : name === "vscode-languageclient/node"
      ? { LanguageClient: Client, TransportKind: { stdio: 0 }, State: { Running: 2, Stopped: 1, Starting: 3 } } : require(name),
    module: { exports: {} }, process: { env: {}, platform: "win32" }, setTimeout, clearTimeout,
  });
  vm.runInContext(fs.readFileSync(path.join(__dirname, "../extension.js"), "utf8"), context);
  return { api: context.module.exports, commands, clients, status, settings, vscode,
    run: (code) => vm.runInContext(code, context), outputCount: () => outputCount };
}

test("startup, restart and stopped clients show the real compiler state", async () => {
  const h = harness();
  h.api.activate({ subscriptions: [] });
  await h.run("serverLifecycle");
  assert.equal(h.clients.length, 1);
  assert.equal(h.status.visible, true);
  assert.match(h.status.text, /check/);
  await h.commands.get("uef-project-language.restartServer")();
  assert.equal(h.clients.length, 2);
  assert.equal(h.clients[0].disposed, true);
  assert.equal(h.outputCount(), 1);
  h.clients[1].changeState({ newState: 1 });
  assert.match(h.status.text, /Registry only/);
  assert.equal(h.run("languageServerActive"), false);
  h.clients[1].changeState({ newState: 2 });
  assert.equal(h.run("languageServerActive"), true);
  await h.api.deactivate();
  assert.equal(h.clients[1].disposed, true);
});

test("disabled compiler reports registry-only mode and does not launch", async () => {
  const h = harness({ enabled: false });
  h.api.activate({ subscriptions: [] });
  await h.run("serverLifecycle");
  assert.equal(h.clients.length, 0);
  assert.match(h.status.text, /Registry only/);
  h.vscode.window.activeTextEditor.document.languageId = "plaintext";
  h.run("updateStatus()");
  assert.equal(h.status.visible, false);
  await h.api.deactivate();
});

test("shutdown while starting disposes the late client", async () => {
  let finish;
  const waiting = new Promise((resolve) => { finish = resolve; });
  const h = harness({ start: () => waiting });
  h.api.activate({ subscriptions: [] });
  await new Promise((resolve) => setImmediate(resolve));
  const shutdown = h.api.deactivate();
  finish();
  await shutdown;
  assert.equal(h.clients[0].disposed, true);
  assert.equal(h.run("languageServerActive"), false);
});

test("theme source colors remain legible on each editor background", () => {
  function luminance(hex) {
    const rgb = hex.slice(1).match(/../g).map((part) => parseInt(part, 16) / 255)
      .map((part) => part <= 0.04045 ? part / 12.92 : ((part + 0.055) / 1.055) ** 2.4);
    return rgb[0] * 0.2126 + rgb[1] * 0.7152 + rgb[2] * 0.0722;
  }
  for (const name of ["dark", "light"]) {
    const theme = JSON.parse(fs.readFileSync(path.join(__dirname, `../themes/uef-pl-${name}-color-theme.json`), "utf8"));
    const background = luminance(theme.colors["editor.background"]);
    for (const value of [...theme.tokenColors.map((rule) => rule.settings.foreground),
      ...Object.values(theme.semanticTokenColors).map((rule) => typeof rule === "string" ? rule : rule.foreground)]) {
      const foreground = luminance(value);
      const contrast = (Math.max(background, foreground) + 0.05) / (Math.min(background, foreground) + 0.05);
      assert.ok(contrast >= 4.5, `${name} ${value}: contrast ${contrast.toFixed(2)}`);
    }
  }
});
