using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace SdfDecalBuilder
{
    class MsdfgenEx
    {
        public static string determineDefaultPath()
        {
            // Executable only builds under x86
            String current = Directory.GetCurrentDirectory();
            current = current.Replace("\\x64", "");

            // Append default filename and return
            return (current + "\\msdfgen.exe");
        }
    }
}
