using System;
using System.ComponentModel.Composition;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.PlatformUI;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Utilities;
using Newtonsoft.Json.Linq;
using StreamJsonRpc;

namespace UEF.ProjectLanguage.VisualStudio
{
    // Kept as an implementation option, but deliberately not exported: the
    // Visual Studio LSP hover owns this surface so hover providers do not stack.
    internal sealed class UETDQuickInfoSourceProvider : IAsyncQuickInfoSourceProvider
    {
        private readonly ITextDocumentFactoryService _documents;

        [ImportingConstructor]
        public UETDQuickInfoSourceProvider(ITextDocumentFactoryService documents)
        {
            _documents = documents;
            UETDProjectLanguageClient.Log("Visual Studio loaded the UEF-PL Quick Info provider.");
        }

        public IAsyncQuickInfoSource TryCreateQuickInfoSource(ITextBuffer textBuffer)
        {
            UETDProjectLanguageClient.Log("Created a UEF-PL Quick Info source for an editor buffer.");
            return textBuffer.Properties.GetOrCreateSingletonProperty(
                () => new UETDQuickInfoSource(textBuffer, _documents));
        }
    }

    /// <summary>
    /// Uses the same live Python language server as completion and navigation,
    /// then renders its symbol and diagnostic data with Visual Studio's native
    /// Quick Info presenter. The links stay live while the pointer is over them.
    /// </summary>
    internal sealed class UETDQuickInfoSource : IAsyncQuickInfoSource
    {
        private static int _loggedFirstSession;
        private static readonly Regex Bold = new Regex(@"\*\*(.*?)\*\*", RegexOptions.Compiled);
        private static readonly Regex InlineCode = new Regex(@"`([^`]*)`", RegexOptions.Compiled);
        private static readonly Regex Heading = new Regex(@"(?m)^\s{0,3}#{1,6}\s*", RegexOptions.Compiled);
        private static readonly Regex Bullet = new Regex(@"(?m)^\s*[-*]\s+", RegexOptions.Compiled);
        private readonly ITextBuffer _textBuffer;
        private readonly ITextDocumentFactoryService _documents;

        public UETDQuickInfoSource(ITextBuffer textBuffer, ITextDocumentFactoryService documents)
        {
            _textBuffer = textBuffer;
            _documents = documents;
        }

        public async Task<QuickInfoItem> GetQuickInfoItemAsync(
            IAsyncQuickInfoSession session,
            CancellationToken cancellationToken)
        {
            if (Interlocked.Exchange(ref _loggedFirstSession, 1) == 0)
            {
                UETDProjectLanguageClient.Log("Visual Studio invoked the UEF-PL Quick Info source.");
            }
            ITextSnapshot snapshot = _textBuffer.CurrentSnapshot;
            SnapshotPoint? trigger = session.GetTriggerPoint(snapshot);
            ITextDocument document;
            if (!trigger.HasValue || !_documents.TryGetTextDocument(_textBuffer, out document))
            {
                return null;
            }

            SnapshotPoint point = trigger.Value;
            SnapshotSpan tokenSpan;
            string token = GetIdentifierAt(point, out tokenSpan);
            if (String.IsNullOrEmpty(token))
            {
                // Diagnostics often underline literals and punctuation, which
                // are not identifiers. Still request Quick Info so the native
                // card can expose the diagnostic and its copy/select actions.
                ITextSnapshotLine line = point.GetContainingLine();
                int position = Math.Max(line.Start.Position,
                    Math.Min(point.Position, Math.Max(line.Start.Position, line.End.Position - 1)));
                tokenSpan = new SnapshotSpan(snapshot, new Microsoft.VisualStudio.Text.Span(position,
                    position < snapshot.Length ? 1 : 0));
            }

            ITextSnapshotLine triggerLine = point.GetContainingLine();
            // Quick Info sessions can report the caret at the right edge of a
            // token. LSP treats positions as half-open, so send the token's
            // start when we successfully identified one; the shared server
            // then resolves the same word for hover and definition.
            int requestPosition = String.IsNullOrEmpty(token) ? point.Position : tokenSpan.Start.Position;
            JObject request = new JObject
            {
                ["textDocument"] = new JObject { ["uri"] = new Uri(document.FilePath).AbsoluteUri },
                ["position"] = new JObject
                {
                    ["line"] = triggerLine.LineNumber,
                    ["character"] = requestPosition - triggerLine.Start.Position
                }
            };
            JObject payload = await UETDQuickInfoRpc.RequestAsync(request, cancellationToken).ConfigureAwait(false);
            if (payload == null)
            {
                return null;
            }

            string content = payload.Value<string>("content");
            if (String.IsNullOrWhiteSpace(content) && payload["diagnostic"] == null)
            {
                return null;
            }

            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            cancellationToken.ThrowIfCancellationRequested();
            UIElement panel = BuildPanel(token ?? "", content ?? "", payload, document.FilePath);
            ITrackingSpan applicable = snapshot.CreateTrackingSpan(tokenSpan.Span, SpanTrackingMode.EdgeInclusive);
            return new QuickInfoItem(applicable, panel);
        }

        private static string GetIdentifierAt(SnapshotPoint point, out SnapshotSpan tokenSpan)
        {
            ITextSnapshotLine line = point.GetContainingLine();
            string text = line.GetText();
            int local = Math.Max(0, Math.Min(point.Position - line.Start.Position, text.Length));
            int probe = local;
            if (probe == text.Length && probe > 0)
            {
                probe--;
            }
            if (probe < text.Length && !IsIdentifierPart(text[probe]) && probe > 0 && IsIdentifierPart(text[probe - 1]))
            {
                probe--;
            }
            if (probe < 0 || probe >= text.Length || !IsIdentifierPart(text[probe]))
            {
                tokenSpan = new SnapshotSpan(point, 0);
                return null;
            }

            int start = probe;
            int end = probe + 1;
            while (start > 0 && IsIdentifierPart(text[start - 1])) start--;
            while (end < text.Length && IsIdentifierPart(text[end])) end++;
            tokenSpan = new SnapshotSpan(line.Snapshot,
                new Microsoft.VisualStudio.Text.Span(line.Start.Position + start, end - start));
            return text.Substring(start, end - start);
        }

        private static bool IsIdentifierPart(char value)
        {
            return value == '_' || Char.IsLetterOrDigit(value);
        }

        private static UIElement BuildPanel(string token, string markdown, JObject payload, string sourcePath)
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            string sourceMarkdown = Regex.Replace(markdown ?? String.Empty,
                @"(?s)```[^\r\n]*\r?\n(.*?)```", "$1");
            string cleaned = CleanMarkdown(sourceMarkdown);
            string[] paragraphs = Regex.Split(sourceMarkdown, @"\r?\n\s*\r?\n");
            string[] headingLines = paragraphs.Length > 0
                ? paragraphs[0].Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries)
                : new string[0];
            string metadata = headingLines.Length > 1
                ? String.Join("  ·  ", Array.ConvertAll(headingLines, line => line.Trim()).Skip(1)
                    .Where(line => !String.IsNullOrWhiteSpace(line)))
                : String.Empty;
            string body = paragraphs.Length > 1
                ? String.Join(Environment.NewLine + Environment.NewLine, paragraphs, 1, paragraphs.Length - 1).Trim()
                : String.Empty;

            var card = new StackPanel();
            var accent = new Border { Height = 3 };
            accent.SetResourceReference(Border.BackgroundProperty, EnvironmentColors.EditorExpansionLinkBrushKey);
            card.Children.Add(accent);

            var content = new StackPanel { MaxWidth = 494, Margin = new Thickness(12, 9, 12, 10) };
            AddTitle(content, headingLines.Length > 0 ? headingLines[0] : token);

            if (!String.IsNullOrWhiteSpace(metadata))
            {
                var details = MakeMarkdownText(metadata, false);
                details.FontSize = 10.5;
                details.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.ToolTipHintTextBrushKey);
                details.Margin = new Thickness(0, 3, 0, 7);
                content.Children.Add(details);
            }
            AddHoverBody(content, body);

            JObject definition = payload["definition"] as JObject;
            if (definition != null)
            {
                string targetUri = definition.Value<string>("uri");
                int targetLine = definition.Value<int?>("line") ?? 0;
                int targetCharacter = definition.Value<int?>("character") ?? 0;
                string targetPath = FilePathFromUri(targetUri);
                if (!String.IsNullOrWhiteSpace(targetPath))
                {
                    var actions = MakeFooter();
                    string label = String.Equals(Path.GetFullPath(sourcePath), Path.GetFullPath(targetPath), StringComparison.OrdinalIgnoreCase)
                        ? "Go to declaration"
                        : "Open UETD/source declaration";
                    AddLink(actions, label + "  " + Path.GetFileName(targetPath) + ":" + (targetLine + 1),
                        () => UETDQuickInfoNavigation.Navigate(targetPath, targetLine, targetCharacter));
                    content.Children.Add(actions);
                }
            }

            JObject diagnostic = payload["diagnostic"] as JObject;
            if (diagnostic != null)
            {
                string message = diagnostic.Value<string>("message") ?? String.Empty;
                string code = diagnostic.Value<string>("code") ?? String.Empty;
                int severity = diagnostic.Value<int?>("severity") ?? 3;
                int startLine = diagnostic.Value<int?>("line") ?? 0;
                int startCharacter = diagnostic.Value<int?>("character") ?? 0;
                int endLine = diagnostic.Value<int?>("endLine") ?? startLine;
                int endCharacter = diagnostic.Value<int?>("endCharacter") ?? startCharacter;

                var separator = MakeRule();
                content.Children.Add(separator);
                var diagnosticTitle = MakeText((severity == 1 ? "Error" : severity == 2 ? "Warning" : "Diagnostic") +
                    (String.IsNullOrWhiteSpace(code) ? String.Empty : "  " + code), true);
                diagnosticTitle.FontSize = 10.0;
                diagnosticTitle.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.EditorExpansionLinkBrushKey);
                diagnosticTitle.Margin = new Thickness(0, 0, 0, 2);
                content.Children.Add(diagnosticTitle);
                var diagnosticText = MakeMarkdownText(message, false);
                diagnosticText.MaxHeight = 66;
                diagnosticText.ToolTip = message;
                content.Children.Add(diagnosticText);

                var diagnosticActions = MakeFooter();
                AddLink(diagnosticActions, "Copy error", () => CopyText(
                    String.IsNullOrWhiteSpace(code) ? message : code + ": " + message));
                AddSeparator(diagnosticActions);
                AddLink(diagnosticActions, "Select error in source", () => UETDQuickInfoNavigation.Navigate(
                    sourcePath, startLine, startCharacter, endLine, endCharacter, true));
                content.Children.Add(diagnosticActions);
            }
            else
            {
                var copy = MakeFooter();
                AddLink(copy, "Copy information", () => CopyText(cleaned));
                content.Children.Add(copy);
            }
            card.Children.Add(content);

            var wrapper = new UETDInteractiveQuickInfoPanel
            {
                Child = card,
                MaxWidth = 520,
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(5),
                Padding = new Thickness(0),
                Focusable = true,
                ClipToBounds = true
            };
            wrapper.SetResourceReference(Border.BackgroundProperty, EnvironmentColors.ToolTipBrushKey);
            wrapper.SetResourceReference(Border.BorderBrushProperty, EnvironmentColors.ToolTipBorderBrushKey);
            return wrapper;
        }

        private static void AddTitle(StackPanel panel, string markdown)
        {
            Match typedTitle = Regex.Match(markdown ?? String.Empty,
                @"^\*\*(?<kind>[^*]+)\*\*\s*(?:·|\|)\s*`(?<name>[^`]*)`");
            if (!typedTitle.Success)
            {
                var title = MakeMarkdownText(markdown, true);
                title.FontSize = 14.0;
                title.Margin = new Thickness(0, 0, 0, 2);
                panel.Children.Add(title);
                return;
            }

            var row = new DockPanel { LastChildFill = true, Margin = new Thickness(0, 0, 0, 2) };
            var badgeText = MakeText(typedTitle.Groups["kind"].Value.ToUpperInvariant(), true);
            badgeText.FontSize = 9.0;
            badgeText.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.EditorExpansionLinkBrushKey);
            var badge = new Border
            {
                Child = badgeText,
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(8),
                Padding = new Thickness(7, 2, 7, 2),
                Margin = new Thickness(10, 1, 0, 0),
                VerticalAlignment = VerticalAlignment.Top
            };
            badge.SetResourceReference(Border.BorderBrushProperty, EnvironmentColors.ToolTipBorderBrushKey);
            DockPanel.SetDock(badge, Dock.Right);
            row.Children.Add(badge);

            var titleName = MakeMarkdownText("`" + typedTitle.Groups["name"].Value + "`", true);
            titleName.FontSize = 14.0;
            row.Children.Add(titleName);
            panel.Children.Add(row);
        }

        private static Border MakeRule()
        {
            var rule = new Border { Height = 1, Margin = new Thickness(0, 8, 0, 7), Opacity = 0.8 };
            rule.SetResourceReference(Border.BackgroundProperty, EnvironmentColors.ToolTipBorderBrushKey);
            return rule;
        }

        private static TextBlock MakeFooter()
        {
            var footer = new TextBlock
            {
                TextWrapping = TextWrapping.Wrap,
                FontSize = 10.5,
                Margin = new Thickness(0, 7, 0, 0)
            };
            footer.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.ToolTipHintTextBrushKey);
            return footer;
        }

        private static TextBlock MakeText(string value, bool heading)
        {
            var block = new TextBlock
            {
                Text = value,
                TextWrapping = TextWrapping.Wrap,
                FontSize = heading ? 12.5 : 12.0,
                FontWeight = heading ? FontWeights.SemiBold : FontWeights.Normal,
                MaxWidth = 470
            };
            block.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.ToolTipTextBrushKey);
            return block;
        }

        private static TextBlock MakeMarkdownText(string value, bool heading)
        {
            var block = new TextBlock
            {
                TextWrapping = TextWrapping.Wrap,
                FontSize = heading ? 12.5 : 12.0,
                FontWeight = heading ? FontWeights.SemiBold : FontWeights.Normal,
                MaxWidth = 470
            };
            block.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.ToolTipTextBrushKey);
            string text = value ?? String.Empty;
            MatchCollection matches = Regex.Matches(text, @"\*\*(?<bold>.+?)\*\*|`(?<code>[^`]*)`");
            int cursor = 0;
            foreach (Match match in matches)
            {
                if (match.Index > cursor)
                {
                    block.Inlines.Add(new Run(text.Substring(cursor, match.Index - cursor)));
                }
                if (match.Groups["bold"].Success)
                {
                    block.Inlines.Add(new Run(match.Groups["bold"].Value) { FontWeight = FontWeights.SemiBold });
                }
                else
                {
                    var code = new Run(match.Groups["code"].Value)
                    {
                        FontFamily = new FontFamily("Consolas"),
                        FontWeight = FontWeights.Medium
                    };
                    code.SetResourceReference(TextElement.ForegroundProperty, EnvironmentColors.EditorExpansionLinkBrushKey);
                    block.Inlines.Add(code);
                }
                cursor = match.Index + match.Length;
            }
            if (cursor < text.Length)
            {
                block.Inlines.Add(new Run(text.Substring(cursor)));
            }
            return block;
        }

        private static void AddHoverBody(StackPanel panel, string markdown)
        {
            if (String.IsNullOrWhiteSpace(markdown)) return;
            var paragraph = new System.Collections.Generic.List<string>();
            Action flushParagraph = () =>
            {
                if (paragraph.Count == 0) return;
                string rawText = String.Join(Environment.NewLine, paragraph).Trim();
                paragraph.Clear();
                if (String.IsNullOrWhiteSpace(CleanMarkdown(rawText))) return;
                var block = MakeMarkdownText(rawText, false);
                block.Margin = new Thickness(0, 1, 0, 4);
                panel.Children.Add(block);
            };

            foreach (string rawLine in Regex.Replace(markdown, @"(?m)^\s{0,3}#{1,6}\s*", String.Empty).Split(new[] { '\r', '\n' }))
            {
                string line = rawLine.Trim();
                if (line.Length == 0)
                {
                    flushParagraph();
                    continue;
                }

                Match section = Regex.Match(line, @"^\*\*(?<title>[^*]+)\*\*$");
                if (section.Success)
                {
                    flushParagraph();
                    var label = MakeMarkdownText(section.Groups["title"].Value, true);
                    label.FontSize = 9.5;
                    label.FontWeight = FontWeights.Bold;
                    label.Margin = new Thickness(0, 7, 0, 3);
                    label.SetResourceReference(TextBlock.ForegroundProperty, EnvironmentColors.EditorExpansionLinkBrushKey);
                    panel.Children.Add(label);
                    continue;
                }

                if (Regex.IsMatch(line, @"^[-*]\s+"))
                {
                    flushParagraph();
                    string itemText = Regex.Replace(line, @"^[-*]\s+", String.Empty);
                    var row = new DockPanel { LastChildFill = true, Margin = new Thickness(2, 0, 0, 2) };
                    var marker = new Border
                    {
                        Width = 4,
                        Height = 4,
                        CornerRadius = new CornerRadius(2),
                        Margin = new Thickness(1, 7, 8, 0),
                        VerticalAlignment = VerticalAlignment.Top
                    };
                    marker.SetResourceReference(Border.BackgroundProperty, EnvironmentColors.ToolTipHintTextBrushKey);
                    DockPanel.SetDock(marker, Dock.Left);
                    row.Children.Add(marker);
                    row.Children.Add(MakeMarkdownText(itemText, false));
                    panel.Children.Add(row);
                    continue;
                }

                paragraph.Add(rawLine);
            }
            flushParagraph();
        }

        private static void AddLink(TextBlock block, string label, Action action)
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            var link = new Hyperlink(new Run(label));
            link.SetResourceReference(TextElement.ForegroundProperty, EnvironmentColors.EditorExpansionLinkBrushKey);
            link.Click += (sender, args) =>
            {
                ThreadHelper.ThrowIfNotOnUIThread();
                args.Handled = true;
                action();
            };
            block.Inlines.Add(link);
        }

        private static void AddSeparator(TextBlock block)
        {
            var separator = new Run("   ·   ");
            separator.SetResourceReference(TextElement.ForegroundProperty, EnvironmentColors.ToolTipHintTextBrushKey);
            block.Inlines.Add(separator);
        }

        private static void CopyText(string value)
        {
            try
            {
                Clipboard.SetText(value ?? String.Empty);
            }
            catch (Exception exception)
            {
                Trace.WriteLine("[UEF-PL Quick Info] Clipboard copy failed: " + exception);
            }
        }

        private static string FilePathFromUri(string value)
        {
            Uri uri;
            return Uri.TryCreate(value, UriKind.Absolute, out uri) && uri.IsFile ? uri.LocalPath : null;
        }

        private static string CleanMarkdown(string value)
        {
            string cleaned = value ?? String.Empty;
            cleaned = Regex.Replace(cleaned, @"(?s)```[^\r\n]*\r?\n(.*?)```", "$1");
            cleaned = Bold.Replace(cleaned, "$1");
            cleaned = InlineCode.Replace(cleaned, "$1");
            cleaned = Heading.Replace(cleaned, String.Empty);
            cleaned = Bullet.Replace(cleaned, "•  ");
            cleaned = Regex.Replace(cleaned, @"(?m)^\s*\|\s*", "  ·  ");
            cleaned = Regex.Replace(cleaned, @"[ \t]+\r?\n", "\n");
            cleaned = Regex.Replace(cleaned, @"\n{3,}", "\n\n");
            return cleaned.Trim();
        }

        public void Dispose()
        {
            // This source is scoped to its editor buffer and owns no process.
        }
    }

    /// <summary>WPF content accepted by the Windows Quick Info presenter.</summary>
    internal sealed class UETDInteractiveQuickInfoPanel : Border, IInteractiveQuickInfoContent
    {
        // Let Visual Studio dismiss the card as soon as its normal trigger
        // tracking says the pointer has left. Interactive content itself is
        // enough to keep links clickable while the pointer is over the card;
        // forcing this flag true pins the whole Quick Info session indefinitely.
        public bool KeepQuickInfoOpen => false;

        public bool IsMouseOverAggregated => IsMouseOver;
    }

    /// <summary>Bridge for one custom request on Visual Studio's active LSP connection.</summary>
    internal static class UETDQuickInfoRpc
    {
        private static readonly object Gate = new object();
        private static JsonRpc _rpc;

        internal static void Attach(JsonRpc rpc)
        {
            lock (Gate)
            {
                _rpc = rpc;
            }
            UETDProjectLanguageClient.Log(rpc == null
                ? "Quick Info RPC detached from the language-server connection."
                : "Quick Info RPC attached to the language-server connection.");
        }

        internal static async Task<JObject> RequestAsync(JObject parameters, CancellationToken cancellationToken)
        {
            JsonRpc rpc;
            lock (Gate)
            {
                rpc = _rpc;
            }
            if (rpc == null)
            {
                UETDProjectLanguageClient.Log("Quick Info request skipped: no custom LSP connection is attached.");
                return null;
            }
            var stopwatch = Stopwatch.StartNew();
            try
            {
                JToken result = await rpc.InvokeWithCancellationAsync<JToken>(
                    "uetd/quickInfo", new object[] { parameters }, cancellationToken).ConfigureAwait(false);
                JObject payload = result as JObject;
                if (stopwatch.ElapsedMilliseconds >= 250)
                {
                    UETDProjectLanguageClient.Log(String.Format(
                        "Quick Info server request took {0} ms.", stopwatch.ElapsedMilliseconds));
                }
                return payload;
            }
            catch (OperationCanceledException)
            {
                throw;
            }
            catch (Exception exception)
            {
                UETDProjectLanguageClient.Log(String.Format("Quick Info server request failed after {0} ms: {1}",
                    stopwatch.ElapsedMilliseconds, exception));
                Trace.WriteLine("[UEF-PL Quick Info] Server request failed: " + exception);
                return null;
            }
        }
    }

    /// <summary>Opens a declaration or selects a diagnostic range in Visual Studio.</summary>
    internal static class UETDQuickInfoNavigation
    {
        internal static void Navigate(string path, int line, int character,
            int? endLine = null, int? endCharacter = null, bool selectRange = false)
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            try
            {
                DTE2 dte = Package.GetGlobalService(typeof(DTE)) as DTE2;
                if (dte == null || String.IsNullOrWhiteSpace(path) || !File.Exists(path))
                {
                    return;
                }
                dte.ItemOperations.OpenFile(path);
                Document document = dte.ActiveDocument;
                if (document == null)
                {
                    return;
                }
                var selection = document.Selection as EnvDTE.TextSelection;
                if (selection == null)
                {
                    return;
                }
                selection.MoveToLineAndOffset(Math.Max(1, line + 1), Math.Max(1, character + 1), false);
                if (selectRange && endLine.HasValue && endCharacter.HasValue)
                {
                    selection.MoveToLineAndOffset(Math.Max(1, endLine.Value + 1),
                        Math.Max(1, endCharacter.Value + 1), true);
                }
            }
            catch (Exception exception)
            {
                Trace.WriteLine("[UEF-PL Quick Info] Navigation failed: " + exception);
            }
        }
    }
}
