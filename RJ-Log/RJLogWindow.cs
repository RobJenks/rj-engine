using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RJ_Log
{
    public partial class RJLogWindow : Form
    {
        private static readonly String LOG_FILENAME = "log.txt";

        private static readonly Color COLOUR_ERROR = Color.Red;
        private static readonly Color COLOUR_WARNING = Color.Yellow;
        private static readonly Color COLOUR_INFO = Color.White;
        private static readonly Color COLOUR_DEBUG = Color.LightBlue;
        private static readonly Color COLOUR_DEFAULT = COLOUR_INFO;

        private FileSystemWatcher watcher = null;
        private String logDirectory = null;
        private String logFile = null;
        private bool updateRequired = false;
        private long updateCount = 0L;

        public RJLogWindow(String logDirectory)
        {
            InitializeComponent();

            this.logDirectory = logDirectory;
            this.logFile = (logDirectory + "\\" + LOG_FILENAME);
        }

        private void RJLogWindow_Load(object sender, EventArgs e)
        {
            InitialiseFilesystemListeners();
            InitialiseLogWatcherComponents();
            ScrollToEnd();
            SetUpdateRequired();
        }

        private void InitialiseFilesystemListeners()
        {
            watcher = new FileSystemWatcher(logDirectory, LOG_FILENAME);
            watcher.Changed += Watcher_Changed;
            watcher.NotifyFilter = NotifyFilters.Size;
            watcher.EnableRaisingEvents = true;
            watcher.SynchronizingObject = this;
        }

        private void InitialiseLogWatcherComponents()
        {
            updateTimer.Start();
        }

        private void SetUpdateRequired()
        {
            updateRequired = true;
        }

        private void Watcher_Changed(object sender, FileSystemEventArgs e)
        {
            SetUpdateRequired();
        }

        private void PerformUpdate()
        {
            bool scrollToEnd = false;
            updateRequired = false;

            // Avoid any issues with LogData.lines.length == 0
            if (LogData.TextLength == 0) LogData.AppendText(" ");

            // We are always appending, so determine the change since last update
            string[] lines;
            try
            {
                ++updateCount;
                lines = File.ReadAllLines(logFile);
            }
            catch (Exception ex)
            {
                SetStatus("LOG RETRIEVAL FAILED [" + ex.Message + "]");
                return;
            }

            // Shouldn't happen, but check to make sure something was added
            if (lines.Length == LogData.Lines.Length)
            {
                SetStatus("Didn't find any changes");
            }

            // See if we can do an incremental update
            bool incremental = true;
            if (lines.Length < LogData.Lines.Length) incremental = false;
            for (int i = 0; i < lines.Length; ++i)
            {
                if (LogData.Lines[i] != lines[i])
                {
                    incremental = false;
                    break;
                }
            }

            // Determine whether we are scrolling to end of the textbox before making any modifications
            int lineIndex = LogData.GetLineFromCharIndex(LogData.SelectionStart);
            scrollToEnd = (LogData.Text.Trim().Length == 0 || (lineIndex == (LogData.Lines.Length - 1)));

            // Now perform the update
            int startLine = 0;
            if (!incremental)
            {
                LogData.Clear();
                startLine = 0;
            }
            else
            {
                startLine = LogData.Lines.Length;
            }

            for (int i = startLine; i < lines.Length; ++i)
            {
                AddLogLine(lines[i]);
            }

            // Scroll to the new end point if we are at the end of the text box, otherwise stay in the same position
            if (scrollToEnd)
            {
                LogData.SelectionStart = LogData.TextLength;
                LogData.ScrollToCaret();
            }

            SetStatus("Monitoring");
        }

        private void AddLogLine(String line)
        {
            AppendText(line, DetermineLogStatementColour(line));
        }

        private Color DetermineLogStatementColour(String line)
        {
            if (line == null) return COLOUR_DEFAULT;

            if (line.StartsWith("ERROR", StringComparison.OrdinalIgnoreCase)) return COLOUR_ERROR;
            else if (line.StartsWith("WARNING", StringComparison.OrdinalIgnoreCase)) return COLOUR_WARNING;
            else if (line.StartsWith("INFO", StringComparison.OrdinalIgnoreCase)) return COLOUR_INFO;
            else if (line.StartsWith("DEBUG", StringComparison.OrdinalIgnoreCase)) return COLOUR_DEBUG;

            else return COLOUR_DEFAULT;
        }

        private void AppendText(String text, Color colour)
        {
            int currentSelection = LogData.SelectionStart;

            LogData.SelectionStart = LogData.TextLength;
            LogData.SelectionLength = 0;
            LogData.SelectionColor = colour;

            LogData.AppendText(text + Environment.NewLine);

            LogData.SelectionColor = LogData.ForeColor;
            LogData.SelectionStart = currentSelection;
        }

        private void SetStatus(String status)
        {
            this.Text = Application.ProductName + ": " + status + " [" + updateCount + " updates]";
        }

        private void ScrollToEnd()
        {
            LogData.SelectionStart = LogData.TextLength;
            LogData.ScrollToCaret();
        }

        private void updateTimer_Tick(object sender, EventArgs e)
        {
            if (updateRequired) PerformUpdate();
        }
    }
}
