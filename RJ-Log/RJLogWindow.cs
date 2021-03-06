﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace RJ_Log
{
    public partial class RJLogWindow : Form
    {
        [DllImport("user32.dll")] static extern bool SetForegroundWindow(IntPtr hWnd);
        [DllImport("user32.dll")] static extern IntPtr SetActiveWindow(IntPtr hWnd);
        [DllImport("user32.dll")] static extern IntPtr SetFocus(IntPtr hWnd);
        [DllImport("user32.dll")] static extern IntPtr FindWindow(string sClass, string sWindow);

        private static readonly String LOG_FILENAME = "log.txt";
        private static readonly int WINDOW_HEIGHT = 320;
        private static readonly int REMOVE_WINDOW_MARGIN = 10;

        private static readonly int UPDATE_TIMER_INTERVAL = 500;    // ms
        private static readonly int TERMINATION_CHECK_CYCLES = 5;   // # of update timer ticks between termination checks

        private static readonly Color COLOUR_ERROR = Color.Red;
        private static readonly Color COLOUR_WARNING = Color.Yellow;
        private static readonly Color COLOUR_INFO = Color.White;
        private static readonly Color COLOUR_DEBUG = Color.LightBlue;
        private static readonly Color COLOUR_DEFAULT = COLOUR_INFO;
        
        private String logDirectory = null;
        private String logFile = null;
        private String parentWindowClass = null;
        private String parentWindowName = null;
        private long logSize = 0L;
        private long updateCount = 0L;
        private int terminationCheckCounter = 0;

        public RJLogWindow(Program.ArgumentData argumentData)
        {
            InitializeComponent();

            this.logDirectory = argumentData.logDirectory;
            this.logFile = (logDirectory + "\\" + LOG_FILENAME);
            this.parentWindowClass = argumentData.parentWindowClass;
            this.parentWindowName = argumentData.parentWindowName;
        }

        private void RJLogWindow_Load(object sender, EventArgs e)
        {
            InitialiseWindow();
            InitialiseLogWatcherComponents();
            ScrollToEnd();
        }

        private void InitialiseWindow()
        {
            
            Rectangle workingArea = Screen.GetWorkingArea(this);

            this.Left = 0 - REMOVE_WINDOW_MARGIN;
            this.Width = workingArea.Width + (2 * REMOVE_WINDOW_MARGIN);

            this.Height = WINDOW_HEIGHT;
            this.Top = (workingArea.Height - this.Height) + REMOVE_WINDOW_MARGIN;

            // If we have a valid parent hwnd, focus that window now so it is on top of us
            if (parentWindowClass != null && parentWindowClass != String.Empty && 
                parentWindowName != null && parentWindowName != String.Empty)
            {
                IntPtr hwnd = FindWindow(parentWindowClass, parentWindowName);
                if (hwnd != null && hwnd != IntPtr.Zero)
                {
                    SetFocus(hwnd);
                    SetActiveWindow(hwnd);
                    SetForegroundWindow(hwnd);
                }
            }
        }
        
        private void InitialiseLogWatcherComponents()
        {
            updateTimer.Interval = UPDATE_TIMER_INTERVAL;
            updateTimer.Start();
        }

        private void PerformUpdate()
        {
            // We are always appending, so determine the change since last update
            String line = String.Empty;
            List<String> lines = new List<String>();
            try
            {
                ++updateCount;

                FileStream stream = File.Open(logFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                StreamReader streamReader = new StreamReader(stream);
                
                while ((line = streamReader.ReadLine()) != null)
                {
                    lines.Add(line);
                }

                streamReader.Close();
            }
            catch (Exception ex)
            {
                SetStatus("LOG RETRIEVAL FAILED [" + ex.Message + "]");
                return;
            }

            // Shouldn't happen, but check to make sure something was added
            if (lines.Count == LogData.Lines.Length)
            {
                SetStatus("Didn't find any changes");
            }
            
            // See if we can do an incremental update
            bool incremental = true;
            int startLine = LogData.Lines.Length;
            if (lines.Count < LogData.Lines.Length)
            {
                incremental = false;
                startLine = 0;
            }
            else
            {
                for (int i = 0; i < LogData.Lines.Length; ++i)
                {
                    if (LogData.Lines[i] != lines[i])
                    {
                        // This line is different.  However if it is the final line, and was previously empty, then still perform an incremental update
                        if (i == (LogData.Lines.Length - 1) && LogData.Lines[i].Trim() == String.Empty)
                        {
                            incremental = true;
                            startLine = LogData.Lines.Length - 1;
                            break;
                        }

                        // This is a difference in the file that prevents us from doing an incremental update
                        incremental = false;
                        startLine = 0;
                        break;
                    }
                }
            }

            // Determine current position and whether we are scrolling to end of the textbox before making any modifications
            int currentPosition = LogData.SelectionStart;
            int lineIndex = LogData.GetLineFromCharIndex(currentPosition);
            bool scrollToEnd = (LogData.Text.Trim().Length == 0 || (lineIndex >= (LogData.Lines.Length - 1)));

            Console.Out.Write(incremental ? ("Incremental " + (scrollToEnd ? "TO-END" : "Normal")) + "\n" : "Full refresh\n");

            // Only clear the output if we are not performing an incremental update
            if (!incremental)
            {
                LogData.Clear();
            }
            
            // Add all new log lines to the output
            for (int i = startLine; i < lines.Count; ++i)
            {
                AddLogLine(lines[i]);
            }

            // Attempt to restore the selection point, if it remains on the same line as before the update
            int newline = LogData.GetLineFromCharIndex(currentPosition);
            if (newline == lineIndex) LogData.SelectionStart = currentPosition;

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
            // Test for changes to the target file and update if necessary
            try
            {
                FileInfo info = new FileInfo(logFile);
                if (info.Length != logSize)
                {
                    logSize = info.Length;
                    updateTimer.Enabled = false;
                    PerformUpdate();
                    updateTimer.Enabled = true;
                }
            }
            catch (Exception ex)
            {
                SetStatus("Exception while attempting to check for updates [" + ex.Message + "]");
            }

            // Also check on a periodic basis whether our parent process has terminated, to ensure we can account
            // for e.g. force-termination or crashes in the IDE
            if (++terminationCheckCounter > TERMINATION_CHECK_CYCLES)
            {
                terminationCheckCounter = 0;
                PerformTerminationCheck();
            }
        }

        // Check whether our parent process has terminated, to ensure we can account
        // for e.g. force-termination or crashes in the IDE
        private void PerformTerminationCheck()
        {
            // Only check for our parent if we have a valid reference
            if (parentWindowClass == null || parentWindowClass == String.Empty ||
                parentWindowName == null || parentWindowName == String.Empty) return;

            // Make sure we can still locate the window
            IntPtr hwnd = FindWindow(parentWindowClass, parentWindowName);
            if (hwnd == null || hwnd == IntPtr.Zero)
            {
                Application.Exit();
            }
        }
    }
}
