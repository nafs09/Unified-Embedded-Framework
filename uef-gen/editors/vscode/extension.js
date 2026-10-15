"use strict";

const fs = require("node:fs");
const path = require("node:path");
const vscode = require("vscode");

const moduleDiagnostics = vscode.languages.createDiagnosticCollection("uetd-modules");
const tokenTypes = new vscode.SemanticTokensLegend(
  ["namespace", "type", "function", "unit", "enumMember", "property", "variable"],
  [],
);
const identifierTriggerCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_".split("");
const diagnosticRefreshTimers = new Map();
let moduleEntries = new Map();
let typeEntries = new Map();
let umathEntries = new Map();
let umathCEntries = new Map();
let unitEntries = new Map();
let unitPattern = null;
let registryRoot = null;
let languageClient = null;
let languageServerActive = false;
let registryLoaded = false;

function candidateRegistryPaths() {
  const configured = vscode.workspace.getConfiguration("uef-project-language").get("uefRoot");
  const roots = (vscode.workspace.workspaceFolders || []).map((folder) => folder.uri.fsPath);
  const candidates = [];
  if (configured) {
    const configuredPath = path.resolve(configured);
    candidates.push(path.join(configuredPath, "registry", "modules", "registry.json"));
    candidates.push(path.join(configuredPath, "UETD", "registry", "modules", "registry.json"));
  }
  for (const root of roots) {
    candidates.push(path.join(root, "UETD", "registry", "modules", "registry.json"));
    candidates.push(path.join(root, "registry", "modules", "registry.json"));
    candidates.push(path.join(root, "..", "UETD", "registry", "modules", "registry.json"));
  }
  return [...new Set(candidates.map((candidate) => path.resolve(candidate)))];
}

function loadModuleRegistry() {
  moduleEntries = new Map();
  typeEntries = new Map();
  umathEntries = new Map();
  umathCEntries = new Map();
  unitEntries = new Map();
  unitPattern = null;
  registryRoot = null;
  const indexPath = candidateRegistryPaths().find((candidate) => fs.existsSync(candidate));
  registryLoaded = true;
  if (!indexPath) return;

  try {
    const index = JSON.parse(fs.readFileSync(indexPath, "utf8"));
    registryRoot = path.resolve(path.dirname(indexPath), "..", "..");
    for (const [moduleId, reference] of Object.entries(index.modules || {})) {
      if (!reference || typeof reference.DSL_descriptor !== "string" || typeof reference.manifest !== "string") {
        continue;
      }
      const manifestPath = path.resolve(registryRoot, reference.manifest);
      if (!manifestPath.startsWith(registryRoot + path.sep)) continue;
      let manifest = {};
      try {
        manifest = JSON.parse(fs.readFileSync(manifestPath, "utf8"));
      } catch (_error) {
        manifest = {};
      }
      moduleEntries.set(reference.DSL_descriptor, {
        moduleId,
        manifestPath,
        authoring: manifest.authoring || null,
        available: manifest.available_for_generation !== false && manifest.scaffold_only !== true,
      });
    }
    const apiPath = path.join(registryRoot, "api.json");
    const api = JSON.parse(fs.readFileSync(apiPath, "utf8"));
    const indexedPath = (reference) => {
      if (typeof reference !== "string") return null;
      const resolved = path.resolve(registryRoot, reference);
      return resolved.startsWith(registryRoot + path.sep) ? resolved : null;
    };
    const typePath = indexedPath(api.type_registry);
    if (typePath && fs.existsSync(typePath)) {
      const catalogue = JSON.parse(fs.readFileSync(typePath, "utf8"));
      for (const entry of catalogue.types || []) {
        if (typeof entry.c_name === "string") typeEntries.set(entry.c_name, entry);
        if (typeof entry.dsl_name === "string") typeEntries.set(entry.dsl_name, entry);
      }
    }
    const umathPath = indexedPath(api.umath_api && api.umath_api.function_registry);
    if (umathPath && fs.existsSync(umathPath)) {
      const catalogue = JSON.parse(fs.readFileSync(umathPath, "utf8"));
      for (const entry of catalogue.functions || []) {
        if (typeof entry.dsl_name === "string") umathEntries.set(entry.dsl_name, entry);
        if (typeof entry.c_name === "string") umathCEntries.set(entry.c_name, entry);
      }
    }
    const unitPath = indexedPath(api.unit_registry);
    if (unitPath && fs.existsSync(unitPath)) {
      const catalogue = JSON.parse(fs.readFileSync(unitPath, "utf8"));
      unitEntries = new Map(Object.entries(catalogue.units || {}));
    }
    const symbols = [...unitEntries.keys()].sort((left, right) => right.length - left.length);
    if (symbols.length) {
      unitPattern = new RegExp(`(?<![A-Za-z0-9_])(${symbols.map(escapedRegex).join("|")})(?![A-Za-z0-9_])`, "g");
    }
  } catch (error) {
    vscode.window.showWarningMessage(`UETD module registry could not be loaded: ${error.message}`);
  }
}

function descriptorAtLine(line, lineNumber) {
  const matches = [];
  const patterns = [
    /\b(?:hybrid\s+)?(?:sensor|actuator|algorithm|protocol|bus)\s+[A-Za-z_][A-Za-z0-9_]*\s*:\s*([A-Za-z_][A-Za-z0-9_]*)/g,
    /\bruntime\s*=\s*([A-Za-z_][A-Za-z0-9_]*)/g,
  ];
  for (const pattern of patterns) {
    for (const match of line.matchAll(pattern)) {
      const name = match[1];
      const start = match.index + match[0].lastIndexOf(name);
      matches.push({ name, start, end: start + name.length, line: lineNumber });
    }
  }
  const direct = /^\s*([A-Za-z_][A-Za-z0-9_]*)\s+[A-Za-z_][A-Za-z0-9_]*\s*(?:;|\{)/.exec(line);
  if (direct) {
    const name = direct[1];
    const start = direct.index + direct[0].indexOf(name);
    matches.push({ name, start, end: start + name.length, line: lineNumber });
  }
  return matches;
}

function visibleCode(line) {
  return line.replace(/"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'/g, (quoted) => " ".repeat(quoted.length))
    .replace(/\/\/.*$|#.*$/, "");
}

function escapedRegex(value) {
  return value.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

function unitMapping(unit) {
  let scale = unit.scale_to_si;
  if (unit.exact_scale) {
    const exact = unit.exact_scale;
    scale = `${exact.numerator}/${exact.denominator}`;
    if (exact.pi_power === 1) scale += " × π";
    else if (exact.pi_power) scale += ` × π^${exact.pi_power}`;
  }
  if (unit.kind === "affine" || unit.offset_to_si !== undefined) {
    return `SI = value × ${scale || "1"} + ${unit.offset_to_si || "0"}`;
  }
  return `× ${scale || "unspecified"} to SI`;
}

function provideUnitTokens(builder, line, lineNumber) {
  if (!unitPattern) return;
  unitPattern.lastIndex = 0;
  for (const match of line.matchAll(unitPattern)) {
    const prefix = line.slice(0, match.index);
    const followsValue = /(?:[0-9](?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?\s*$/.test(prefix);
    const followsUnitSyntax = /[<*/^]\s*$/.test(prefix);
    if (followsValue || followsUnitSyntax) {
      builder.push(lineNumber, match.index, match[1].length, 3, 0);
    }
  }
}

function refreshDiagnostics(document) {
  if (document.languageId !== "uef-project" && document.languageId !== "uef-system") return;
  if (languageServerActive) {
    moduleDiagnostics.delete(document.uri);
    return;
  }
  const diagnostics = [];
  for (let lineNumber = 0; lineNumber < document.lineCount; lineNumber += 1) {
    const line = visibleCode(document.lineAt(lineNumber).text);
    for (const occurrence of descriptorAtLine(line, lineNumber)) {
      const entry = moduleEntries.get(occurrence.name);
      if (!entry) continue;
      const authoring = entry.authoring;
      const generationReady = authoring && authoring.generation && authoring.generation.status === "ready";
      let message = null;
      if (!authoring) {
        message = `UETD module '${entry.moduleId}' is registered, but has no DSL authoring contract yet. It is discoverable, but configuration and generation are not implemented.`;
      } else if (authoring.status !== "reviewed" || !generationReady || !entry.available) {
        message = `UETD module '${entry.moduleId}' has a ${authoring.status || "missing"} authoring/generation contract. The DSL can resolve it, but generation readiness is incomplete.`;
      }
      if (message) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(lineNumber, occurrence.start, lineNumber, occurrence.end),
          message,
          vscode.DiagnosticSeverity.Warning,
        );
        diagnostic.code = "uetd-module-not-implemented";
        diagnostic.source = "UETD registry";
        diagnostics.push(diagnostic);
      }
    }
    for (const match of line.matchAll(/\b([A-Za-z_][A-Za-z0-9_]*)\s*\(/g)) {
      const entry = umathEntries.get(match[1]);
      if (!entry) {
        const cEntry = umathCEntries.get(match[1]);
        if (!cEntry) continue;
        const start = match.index;
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(lineNumber, start, lineNumber, start + match[1].length),
          `Raw C symbol '${match[1]}' is hidden in UEF-PL; call '${cEntry.dsl_name}' using its registered DSL signature.`,
          vscode.DiagnosticSeverity.Warning,
        );
        diagnostic.code = "uetd-umath-c-name-hidden";
        diagnostic.source = "UETD UMATH registry";
        diagnostics.push(diagnostic);
        continue;
      }
      if (entry.available_for_generation === true && entry.dsl_binding_status === "ready") continue;
      const start = match.index;
      const status = entry.dsl_binding_status || "missing DSL binding";
      const moduleStatus = entry.implementation_status || "unknown implementation status";
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(lineNumber, start, lineNumber, start + match[1].length),
        `UEF-PL function '${match[1]}' is discoverable, but its UETD implementation/binding is not ready (${moduleStatus}; ${status}).`,
        vscode.DiagnosticSeverity.Warning,
      );
      diagnostic.code = "uetd-umath-not-ready";
      diagnostic.source = "UETD UMATH registry";
      diagnostics.push(diagnostic);
    }
  }
  moduleDiagnostics.set(document.uri, diagnostics);
}

function scheduleDiagnostics(document) {
  if (languageServerActive) {
    moduleDiagnostics.delete(document.uri);
    return;
  }
  const previous = diagnosticRefreshTimers.get(document.uri.toString());
  if (previous) clearTimeout(previous);
  const key = document.uri.toString();
  const timer = setTimeout(() => {
    diagnosticRefreshTimers.delete(key);
    refreshDiagnostics(document);
  }, 150);
  diagnosticRefreshTimers.set(key, timer);
}

function clearDiagnosticRefreshTimers() {
  for (const timer of diagnosticRefreshTimers.values()) clearTimeout(timer);
  diagnosticRefreshTimers.clear();
}

function initializeFallbackFeatures() {
  if (!registryLoaded) loadModuleRegistry();
  for (const document of vscode.workspace.textDocuments) scheduleDiagnostics(document);
}

function provideModuleTokens(document) {
  const builder = new vscode.SemanticTokensBuilder(tokenTypes);
  if (languageServerActive) return builder.build();
  for (let lineNumber = 0; lineNumber < document.lineCount; lineNumber += 1) {
    const line = visibleCode(document.lineAt(lineNumber).text);
    provideUnitTokens(builder, line, lineNumber);
    const targetPeripheralTypes = new Set();
    for (const match of line.matchAll(/\bperipheral\s+[A-Za-z_][A-Za-z0-9_]*\s*:\s*([A-Za-z_][A-Za-z0-9_]*)/g)) {
      targetPeripheralTypes.add(match.index + match[0].lastIndexOf(match[1]));
    }
    for (const match of line.matchAll(/[A-Za-z_][A-Za-z0-9_]*/g)) {
      if (targetPeripheralTypes.has(match.index)) continue;
      const registeredModule = moduleEntries.has(match[0]);
      const registeredType = typeEntries.get(match[0]);
      if (registeredModule) {
        builder.push(lineNumber, match.index, match[0].length, 0, 0);
      } else if (registeredType) {
        builder.push(lineNumber, match.index, match[0].length, 1, 0);
        for (const member of registeredType.enum_members || []) {
          if (line.slice(match.index + match[0].length).startsWith(`.${member}`)) {
            builder.push(lineNumber, match.index + match[0].length + 1, member.length, 4, 0);
          }
        }
      } else if (umathEntries.has(match[0]) && line.slice(match.index + match[0].length).trimStart().startsWith("(")) {
        builder.push(lineNumber, match.index, match[0].length, 2, 0);
      }
    }
  }
  return builder.build();
}

function activate(context) {
  context.subscriptions.push(
    moduleDiagnostics,
    vscode.languages.registerDocumentSemanticTokensProvider(
      [{ language: "uef-project" }, { language: "uef-system" }],
      { provideDocumentSemanticTokens: provideModuleTokens },
      tokenTypes,
    ),
    vscode.workspace.onDidOpenTextDocument(scheduleDiagnostics),
    vscode.workspace.onDidChangeTextDocument((event) => scheduleDiagnostics(event.document)),
    vscode.workspace.onDidCloseTextDocument((document) => {
      const key = document.uri.toString();
      const timer = diagnosticRefreshTimers.get(key);
      if (timer) clearTimeout(timer);
      diagnosticRefreshTimers.delete(key);
      moduleDiagnostics.delete(document.uri);
    }),
    vscode.workspace.onDidChangeWorkspaceFolders(() => {
      if (!languageServerActive) {
        loadModuleRegistry();
        for (const document of vscode.workspace.textDocuments) refreshDiagnostics(document);
      }
    }),
    vscode.workspace.onDidChangeConfiguration((event) => {
      if (event.affectsConfiguration("uef-project-language.uefRoot") && !languageServerActive) {
        loadModuleRegistry();
        for (const document of vscode.workspace.textDocuments) refreshDiagnostics(document);
      }
    }),
    vscode.workspace.onDidSaveTextDocument((document) => {
      if (!languageServerActive && (document.uri.fsPath.endsWith(`${path.sep}api.json`) || document.uri.fsPath.includes(`${path.sep}registry${path.sep}`))) {
        loadModuleRegistry();
        for (const openDocument of vscode.workspace.textDocuments) refreshDiagnostics(openDocument);
        return;
      }
      refreshDiagnostics(document);
    }),
    vscode.languages.registerCompletionItemProvider(
      [{ language: "uef-project" }, { language: "uef-system" }],
      {
        provideCompletionItems(document, position) {
          if (languageServerActive) return [];
          const linePrefix = document.lineAt(position.line).text.slice(0, position.character);
          const prefixMatch = /[A-Za-z_][A-Za-z0-9_]*$/.exec(linePrefix);
          const prefix = prefixMatch ? prefixMatch[0] : "";
          const matchesPrefix = (candidate) => candidate.toLowerCase().startsWith(prefix.toLowerCase());
          const entries = [];
          for (const [descriptor, module] of moduleEntries) {
            const item = new vscode.CompletionItem(descriptor, vscode.CompletionItemKind.Module);
            item.detail = module.moduleId;
            item.documentation = module.available ? "Registered in the UETD module catalogue." : "Registered in UETD; authoring or generation is incomplete.";
            if (matchesPrefix(descriptor)) entries.push(item);
          }
          for (const [name, type] of typeEntries) {
            const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Struct);
            item.detail = type.header;
            item.documentation = type.declaration;
            if (matchesPrefix(name)) entries.push(item);
          }
          for (const [name, fn] of umathEntries) {
            const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
            item.detail = fn.dsl_signature || `${name}(...)`;
            item.documentation = `${fn.c_name} · ${fn.module_id || "UMATH"} · ${fn.implementation_status} · DSL ${fn.dsl_binding_status}`;
            if (matchesPrefix(name)) entries.push(item);
          }
          for (const [symbol, unit] of unitEntries) {
            const item = new vscode.CompletionItem(symbol, vscode.CompletionItemKind.Unit);
            item.detail = unitMapping(unit);
            item.documentation = `Dimension ${JSON.stringify(unit.dimension || {})}`;
            if (matchesPrefix(symbol)) entries.push(item);
          }
          return entries;
        },
      },
      ...identifierTriggerCharacters,
    ),
    vscode.languages.registerHoverProvider(
      [{ language: "uef-project" }, { language: "uef-system" }],
      {
        provideHover(document, position) {
          if (languageServerActive) return undefined;
          const wordRange = document.getWordRangeAtPosition(position, /[A-Za-z_][A-Za-z0-9_]*/);
          if (!wordRange) return undefined;
          const word = document.getText(wordRange);
          const fn = umathEntries.get(word);
          if (fn) {
            const call = fn.dsl_signature || `${fn.dsl_name}(...)`;
            return new vscode.Hover(`UEF-PL function \`${call}\`\n\nLowers to \`${fn.c_name}\` · ${fn.module_id || "UMATH"} · ${fn.implementation_status} · DSL binding ${fn.dsl_binding_status}\n\n${fn.c_signature}`);
          }
          const unit = unitEntries.get(word);
          if (unit) {
            return new vscode.Hover(`UETD unit \`${word}\`\n\nDimension ${JSON.stringify(unit.dimension || {})} · ${unitMapping(unit)}`);
          }
          const type = typeEntries.get(word);
          if (type) return new vscode.Hover(`\`${type.declaration}\`\n\n${type.header}`);
          const module = moduleEntries.get(word);
          if (module) return new vscode.Hover(`UETD module \`${module.moduleId}\`\n\n${module.available ? "Generation contract ready" : "Registered; authoring/generation contract incomplete"}`);
          return undefined;
        },
      },
    ),
  );
  void startLanguageServer(context);
}

async function startLanguageServer(context) {
  const configuration = vscode.workspace.getConfiguration("uef-project-language");
  if (!configuration.get("languageServer.enabled", true)) {
    initializeFallbackFeatures();
    return;
  }

  const output = vscode.window.createOutputChannel("UEF-PL Language Server");
  context.subscriptions.push(output);
  const { LanguageClient, TransportKind } = require("vscode-languageclient/node");
  const extraArguments = [...configuration.get("languageServer.arguments", [])];
  const uetdRoot = configuration.get("uefRoot", "");
  if (uetdRoot) extraArguments.push("--uetd-root", uetdRoot);
  const candidates = [];
  const addCandidate = (command, args, description) => {
    if (!command || candidates.some((candidate) => candidate.command === command && candidate.args.join("\0") === args.join("\0"))) return;
    candidates.push({ command, args, description });
  };
  addCandidate(process.env.UEF_PL_LANGUAGE_SERVER, extraArguments, "UEF_PL_LANGUAGE_SERVER");
  addCandidate(configuration.get("languageServer.executable", ""), extraArguments, "configured executable");
  addCandidate("uef-gen-language-server", extraArguments, "PATH entry point");
  const pythonArgs = ["-m", "uef_gen.language.server", ...extraArguments];
  const python = process.env.UEF_PL_PYTHON || configuration.get("languageServer.pythonExecutable", "python");
  addCandidate(python, pythonArgs, "configured Python module");
  if (process.platform === "win32") addCandidate("py", ["-3", ...pythonArgs], "Python launcher");

  for (const candidate of candidates) {
    const clientOptions = {
      documentSelector: [
        { scheme: "file", language: "uef-project" },
        { scheme: "file", language: "uef-system" },
      ],
      outputChannel: output,
      traceOutputChannel: output,
    };
    const client = new LanguageClient(
      "uefProjectLanguage",
      "UEF Project Language",
      { command: candidate.command, args: candidate.args, transport: TransportKind.stdio },
      clientOptions,
    );
    try {
      output.appendLine(`Starting language server with ${candidate.description}: ${candidate.command}`);
      await client.start();
      languageClient = client;
      languageServerActive = true;
      context.subscriptions.push(client);
      clearDiagnosticRefreshTimers();
      for (const document of vscode.workspace.textDocuments) moduleDiagnostics.delete(document.uri);
      output.appendLine("UEF-PL language server initialized.");
      return;
    } catch (error) {
      output.appendLine(`Could not start ${candidate.description}: ${error.message}`);
      try { await client.stop(); } catch (_stopError) { /* failed start may already be stopped */ }
    }
  }

  languageServerActive = false;
  initializeFallbackFeatures();
  output.appendLine("Language-server startup failed; using registry-only fallback features.");
  void vscode.window.showWarningMessage(
    "The UEF-PL language server did not start. Registry completion and hover remain available; see the UEF-PL Language Server output channel.",
  );
}

function deactivate() {
  clearDiagnosticRefreshTimers();
  moduleDiagnostics.clear();
  if (languageClient) {
    const stopping = languageClient.stop();
    languageClient = null;
    languageServerActive = false;
    return stopping;
  }
}

module.exports = { activate, deactivate };
