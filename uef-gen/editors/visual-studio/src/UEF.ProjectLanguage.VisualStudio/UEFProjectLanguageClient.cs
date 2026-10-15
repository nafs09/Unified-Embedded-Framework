using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.LanguageServer.Client;
using Microsoft.VisualStudio.Threading;
using Microsoft.VisualStudio.Utilities;
using System.ComponentModel.Composition;
using Newtonsoft.Json.Linq;
using StreamJsonRpc;

namespace UEF.ProjectLanguage.VisualStudio
{
    /// <summary>
    /// Starts the shared Python LSP process. The server analyzes UEF-PL; it is
    /// not a Python-language server. Keep language semantics in uef-gen.
    /// </summary>
    [ContentType(UETDProjectLanguageContentTypes.Name)]
    [Export(typeof(ILanguageClient))]
    public sealed class UETDProjectLanguageClient : ILanguageClient, ILanguageClientCustomMessage2
    {
        private static readonly string LogPath = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "UEF-PL-LSP.log");
        private static readonly object LogGate = new object();
        private const long MaximumLogBytes = 4 * 1024 * 1024;
        private Process _languageServerProcess;
        private readonly ILanguageClientMiddleLayer2<JToken> _middleLayer = new UETDLanguageServerMiddleLayer();

        public UETDProjectLanguageClient()
        {
            Log("MEF constructed the UEF-PL language client.");
        }

        public string Name => "UEF Project Language";

        public IEnumerable<string> ConfigurationSections => null;

        public object InitializationOptions => null;

        public IEnumerable<string> FilesToWatch => null;

        public object MiddleLayer => _middleLayer;

        // This extension only sends custom requests to the language server;
        // the server does not need a custom callback target on the client.
        public object CustomMessageTarget => null;

        public bool ShowNotificationOnInitializeFailed => true;

        public event AsyncEventHandler<EventArgs> StartAsync;

        public event AsyncEventHandler<EventArgs> StopAsync;

        public Task AttachForCustomMessageAsync(JsonRpc rpc)
        {
            UETDQuickInfoRpc.Attach(rpc);
            return Task.CompletedTask;
        }

        public async Task<Connection> ActivateAsync(CancellationToken token)
        {
            token.ThrowIfCancellationRequested();
            Exception lastFailure = null;
            foreach (ServerCommand command in GetServerCommands())
            {
                token.ThrowIfCancellationRequested();
                var startInfo = new ProcessStartInfo
                {
                    FileName = command.FileName,
                    Arguments = command.Arguments,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true
                };
                var process = new Process { StartInfo = startInfo, EnableRaisingEvents = true };
                process.ErrorDataReceived += (sender, eventArgs) =>
                {
                    if (!String.IsNullOrEmpty(eventArgs.Data))
                    {
                        Log("Server stderr: " + eventArgs.Data);
                    }
                };

                try
                {
                    Log("Trying server command: " + command.Description);
                    if (!process.Start())
                    {
                        throw new InvalidOperationException("Process.Start returned false.");
                    }
                    process.BeginErrorReadLine();
                    await Task.Delay(350, token);
                    if (process.HasExited)
                    {
                        throw new InvalidOperationException(
                            "The language-server process exited immediately with code " + process.ExitCode + ".");
                    }
                    _languageServerProcess = process;
                    Log("Started with " + command.Description + ".");
                    return new Connection(process.StandardOutput.BaseStream, process.StandardInput.BaseStream);
                }
                catch (OperationCanceledException)
                {
                    try
                    {
                        if (!process.HasExited)
                        {
                            process.Kill();
                        }
                    }
                    catch (InvalidOperationException) { }
                    process.Dispose();
                    throw;
                }
                catch (Exception exception)
                {
                    lastFailure = exception;
                    Log("Could not start " + command.Description + ": " + exception);
                    try
                    {
                        if (!process.HasExited)
                        {
                            process.Kill();
                        }
                    }
                    catch (InvalidOperationException) { }
                    process.Dispose();
                }
            }

            throw new InvalidOperationException(
                "Could not start the UEF-PL language server. Set UEF_PL_LANGUAGE_SERVER to its executable, " +
                "or install uef-gen into the Python interpreter available as UEF_PL_PYTHON, python, or py.",
                lastFailure);
        }

        private static IEnumerable<ServerCommand> GetServerCommands()
        {
            string configured = ReadEnvironmentSetting("UEF_PL_LANGUAGE_SERVER");
            string python = ReadEnvironmentSetting("UEF_PL_PYTHON");
            string uetdRoot = ReadEnvironmentSetting("UETD_PATH");
            var commands = new List<ServerCommand>();
            if (!String.IsNullOrWhiteSpace(configured))
            {
                commands.Add(new ServerCommand(configured, RootArguments(String.Empty, uetdRoot), "UEF_PL_LANGUAGE_SERVER"));
            }
            commands.Add(new ServerCommand("uef-gen-language-server", RootArguments(String.Empty, uetdRoot), "PATH entry point"));
            if (!String.IsNullOrWhiteSpace(python))
            {
                commands.Add(new ServerCommand(python, RootArguments("-m uef_gen.language.server", uetdRoot), "UEF_PL_PYTHON"));
            }
            commands.Add(new ServerCommand("python", RootArguments("-m uef_gen.language.server", uetdRoot), "python -m"));
            commands.Add(new ServerCommand("py", RootArguments("-3 -m uef_gen.language.server", uetdRoot), "py -3 -m"));
            return commands;
        }

        private static string ReadEnvironmentSetting(string name)
        {
            string value = Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.Process);
            if (!String.IsNullOrWhiteSpace(value))
            {
                return value;
            }

            // Visual Studio can remain open across a user-level environment
            // update. Read the persisted value as a fallback so a client
            // restart does not depend on restarting the whole IDE.
            value = Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.User);
            if (!String.IsNullOrWhiteSpace(value))
            {
                return value;
            }

            return Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.Machine);
        }

        private static string RootArguments(string arguments, string root)
        {
            if (String.IsNullOrWhiteSpace(root))
            {
                return arguments;
            }
            string quotedRoot = "\"" + root.Replace("\"", "\\\"") + "\"";
            return (arguments + " --uetd-root " + quotedRoot).Trim();
        }

        private sealed class ServerCommand
        {
            public ServerCommand(string fileName, string arguments, string description)
            {
                FileName = fileName;
                Arguments = arguments;
                Description = description;
            }

            public string FileName { get; private set; }
            public string Arguments { get; private set; }
            public string Description { get; private set; }
        }

        public async Task OnLoadedAsync()
        {
            Log("Visual Studio matched the UETD Project Language content type and loaded the language client.");
            if (StartAsync != null)
            {
                await StartAsync.InvokeAsync(this, EventArgs.Empty);
            }
            else
            {
                Log("No Visual Studio start callback was registered for the language client.");
            }
        }

        public Task<InitializationFailureContext> OnServerInitializeFailedAsync(
            ILanguageClientInitializationInfo initializationState)
        {
            Exception exception = initializationState == null
                ? null
                : initializationState.InitializationException;
            string message = exception == null
                ? (initializationState == null
                    ? "The UEF-PL language server failed to initialize."
                    : initializationState.StatusMessage)
                : exception.ToString();
            Log("Server initialization failed: " + message);
            return Task.FromResult(new InitializationFailureContext
            {
                FailureMessage = message
            });
        }

        public Task OnServerInitializedAsync()
        {
            Log("Language server initialized; LSP requests are ready.");
            return Task.CompletedTask;
        }

        public async Task OnServerStoppedAsync()
        {
            Process process = _languageServerProcess;
            _languageServerProcess = null;
            UETDQuickInfoRpc.Attach(null);
            if (process == null)
            {
                return;
            }
            Log("Stopping language-server process.");
            try
            {
                if (!process.HasExited)
                {
                    process.StandardInput.Close();
                    await Task.Delay(100);
                    if (!process.HasExited)
                    {
                        process.Kill();
                    }
                }
            }
            catch (InvalidOperationException) { }
            finally
            {
                process.Dispose();
            }
        }

        internal static void Log(string message)
        {
            string entry = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff") + " " + message;
            Trace.WriteLine("[UEF-PL LSP] " + entry);
            try
            {
                lock (LogGate)
                {
                    var file = new FileInfo(LogPath);
                    if (file.Exists && file.Length > MaximumLogBytes)
                    {
                        string previous = LogPath + ".1";
                        if (File.Exists(previous))
                        {
                            previous = LogPath + "." + DateTime.Now.ToString("yyyyMMdd-HHmmss-fff") + ".previous";
                        }
                        File.Move(LogPath, previous);
                    }
                    File.AppendAllText(LogPath, entry + Environment.NewLine);
                }
            }
            catch (IOException) { }
            catch (UnauthorizedAccessException) { }
        }
    }

    /// <summary>
    /// Leaves the native Visual Studio LSP hover as the single hover surface
    /// and records only unusually slow editor requests.
    /// </summary>
    internal sealed class UETDLanguageServerMiddleLayer : ILanguageClientMiddleLayer2<JToken>
    {
        public bool CanHandle(string methodName)
        {
            return String.Equals(methodName, "textDocument/hover", StringComparison.Ordinal)
                || String.Equals(methodName, "textDocument/definition", StringComparison.Ordinal)
                || String.Equals(methodName, "textDocument/implementation", StringComparison.Ordinal)
                || String.Equals(methodName, "textDocument/typeDefinition", StringComparison.Ordinal)
                || String.Equals(methodName, "textDocument/declaration", StringComparison.Ordinal);
        }

        public async Task<JToken> HandleRequestAsync(
            string methodName,
            JToken methodParam,
            Func<JToken, Task<JToken>> sendRequest)
        {
            var stopwatch = Stopwatch.StartNew();
            try
            {
                JToken result = await sendRequest(methodParam).ConfigureAwait(false);
                if (stopwatch.ElapsedMilliseconds >= 250)
                {
                    UETDProjectLanguageClient.Log(String.Format(
                        "Editor request {0} took {1} ms.", methodName, stopwatch.ElapsedMilliseconds));
                }
                return result;
            }
            catch (Exception exception)
            {
                UETDProjectLanguageClient.Log(String.Format("Editor request {0} failed after {1} ms: {2}",
                    methodName, stopwatch.ElapsedMilliseconds, exception));
                throw;
            }
        }

        public Task HandleNotificationAsync(
            string methodName,
            JToken methodParam,
            Func<JToken, Task> sendNotification)
        {
            return sendNotification(methodParam);
        }
    }
}
