using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace SdfDecalBuilder
{
    class Program
    {
        static readonly int MIN_PRINTABLE_CHAR_CODE = 32;       // == ' '

        static void ExitWithError(string error)
        {
            System.Console.Out.WriteLine(error);
            Environment.Exit(1);
        }

        

        static void Main(string[] args)
        {
            // Commandline data to be read, and defaults
            string ttf = "";
            string output = "sdf-output.png";
            string outputData = "sdf-output.xml";
            int minchar = 32;
            int maxchar = 255;

            // All args are expected in pairs, +1 for the executable path
            if (args.Length % 2 != 0) ExitWithError("Error: expecting arguments {key value}xN");

            // Process each argument pair and configure the generator with any non-mandatory parameters
            SDFGenerator generator = new SDFGenerator();
            for (int arg = 0; arg < args.Length; arg += 2)
            {
                string key = args[arg];
                string val = args[arg + 1];

                // Mandatory
                if (key == "-ttf") ttf = val;
                else if (key == "-o") output = val;
                else if (key == "-od") outputData = val;
                else if (key == "-min") minchar = int.Parse(val);
                else if (key == "-max") maxchar = int.Parse(val);

                // Optional
                else if (key == "-sdfEx") generator.setSdfGenExPath(val);
                else if (key == "-size") generator.setDecalSize(int.Parse(val));
                else if (key == "-texsize") generator.setTextureMapSize(int.Parse(val));
                else if (key == "-sdfmode") generator.setSdfMode(val);
                else if (key == "-space") generator.setSpaceWidth(int.Parse(val));
            }

            // Verify arguments
            if (ttf.Length == 0) ExitWithError("Error: Source TTF file must be provided");
            if (output.Length == 0) ExitWithError("Error: Output file must be specified");
            if (minchar < MIN_PRINTABLE_CHAR_CODE) ExitWithError("Error: minimum charcode set to " + minchar + "; must be printable and >= " + MIN_PRINTABLE_CHAR_CODE + "");
            if (minchar > maxchar) ExitWithError("Error: minimum charcode " + minchar + " > maximum charcode " + maxchar + "");

            // Invoke the generator and return success
            generator.GenerateSDFTexture(ttf, output, outputData, minchar, maxchar);

            Console.ReadKey(true);
            Environment.Exit(0);
        }
    }
}
