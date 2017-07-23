using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RJ_Log
{
    public static class Program
    {
        public struct ArgumentData
        {
            public String logDirectory;
            public String parentWindowClass;
            public String parentWindowName;

            public static ArgumentData Empty()
            {
                ArgumentData empty = new ArgumentData();
                empty.logDirectory = String.Empty;
                empty.parentWindowClass = String.Empty;
                empty.parentWindowName = String.Empty;
                return empty;
            }
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            ArgumentData argumentData = ProcessCommandLine(Environment.GetCommandLineArgs());
            if (argumentData.logDirectory == null || argumentData.logDirectory == String.Empty)
            {
                MessageBox.Show("Log directory must be passed to application as a commandline argument");
                return;
            }

            Application.Run(new RJLogWindow(argumentData));
        }

        private static ArgumentData ProcessCommandLine(string[] args)
        {
            // Argument count must be odd (length % 2 == 1) since, after accounting for args[0] which
            // is the process executable name, all other arguments MUST be specified in pairs
            if (args == null || (args.Length % 2 != 1)) return ArgumentData.Empty();

            // Process arguments in pairs
            ArgumentData data = ArgumentData.Empty();
            for (int i = 1; i < args.Length; i += 2)
            {
                if (args[i].Equals("-logDirectory", StringComparison.OrdinalIgnoreCase))
                    data.logDirectory = args[i + 1];

                else if (args[i].Equals("-parentWindowClass", StringComparison.OrdinalIgnoreCase))
                    data.parentWindowClass = args[i + 1];

                else if (args[i].Equals("-parentWindowName", StringComparison.OrdinalIgnoreCase))
                    data.parentWindowName = args[i + 1];
            }

            return data;
        }
    }
}





