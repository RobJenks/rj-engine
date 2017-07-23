using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RJ_Log
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            String logDirectory = GetLogDirectory(Environment.GetCommandLineArgs());
            if (logDirectory == null)
            {
                MessageBox.Show("Log directory must be passed to application as a commandline argument");
                return;
            }

            Application.Run(new RJLogWindow(logDirectory));
        }

        private static String GetLogDirectory(string[] args)
        {
            if (args == null || args.Length < 2) return null;

            return (args[1] != null ? args[1].Trim() : null);
        }
    }
}
