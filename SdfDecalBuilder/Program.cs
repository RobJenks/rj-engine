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
            string sdfgenex = "";
            int minchar = 32;
            int maxchar = 255;
            int decalsize = SDFGenerator.DEFAULT_DECAL_SIZE;
            string sdfMode = "";

            // All args are expected in pairs, +1 for the executable path
            if (args.Length % 2 != 0) ExitWithError("Error: expecting arguments {key value}xN");

            // Process each argument pair
            for (int arg = 0; arg < args.Length; arg += 2)
            {
                string key = args[arg];
                string val = args[arg + 1];

                if (key == "-ttf") ttf = val;
                else if (key == "-o") output = val;
                else if (key == "-od") outputData = val;
                else if (key == "-min") minchar = int.Parse(val);
                else if (key == "-max") maxchar = int.Parse(val);
                else if (key == "-sdfEx") sdfgenex = val;
                else if (key == "-size") decalsize = int.Parse(val);
                else if (key == "-sdfmode") sdfMode = val;
            }

            // Verify arguments
            if (ttf.Length == 0) ExitWithError("Error: Source TTF file must be provided");
            if (output.Length == 0) ExitWithError("Error: Output file must be specified");
            if (minchar < MIN_PRINTABLE_CHAR_CODE) ExitWithError("Error: minimum charcode set to " + minchar + "; must be printable and >= " + MIN_PRINTABLE_CHAR_CODE + "");
            if (minchar > maxchar) ExitWithError("Error: minimum charcode " + minchar + " > maximum charcode " + maxchar + "");
            if (decalsize <= 0) ExitWithError("Error: Invalid decal size of " + decalsize + " was specified");

            // Invoke the generator and return success
            SDFGenerator generator = new SDFGenerator();
            
            generator.setDecalSize(decalsize);
            if (sdfgenex.Length != 0) generator.setSdfGenExPath(sdfgenex);
            if (sdfMode.Length != 0) generator.setSdfMode(sdfMode);
            
            generator.GenerateSDFTexture(ttf, output, outputData, minchar, maxchar);

            Console.ReadKey(true);
            Environment.Exit(0);
        }
    }
}
