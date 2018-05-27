using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;
using System.Drawing;
using System.Xml;

namespace SdfDecalBuilder
{
    class SDFGenerator
    {
        public class SdfMode
        {
            public static readonly string SignedDistanceField = "sdf";
            public static readonly string SignedPseudoDistanceField = "psdf";
            public static readonly string MultiChannelSignedDistanceField = "msdf";
        }

        public static int DEFAULT_DECAL_SIZE = 16;
        public static string DEFAULT_SDF_MODE = SdfMode.SignedDistanceField;

        private bool retainIntermediateFiles = false;
        private string sdfGenExPath = MsdfgenEx.determineDefaultPath();
        private int decalSize = 16;
        private string sdfMode = SdfMode.SignedDistanceField;

        public void GenerateSDFTexture(string sourceTTF, string outputFile, string outputDataFile, int minchar, int maxchar)
        {
            // Determine a target directory and a place to store intermediate images
            FileInfo output = new FileInfo(outputFile);
            FileInfo outputData = new FileInfo(outputDataFile);
            string tempDirPath = output.Directory.FullName + "\\sdfgen_tmp";
            PrepareTemporaryDirectory(tempDirPath);

            // Create the xml data document for this font
            var xmlDocument = createXmlDataDocument();
            var fontNode = writeXmlDataHeader(xmlDocument);

            // Path to sdfgen executable
            string exPath = getSdfGenExPath();

            // Determine the dimensions and size of the target texture; closest to square as possible is 
            // optimum due to power-of-two optimisations
            int count = (maxchar - minchar + 1);
            double exactDimension = Math.Sqrt(count);
            int exactCountPerSide = (int)Math.Ceiling(exactDimension);
            int exactPixelsPerSide = (exactCountPerSide * decalSize);
            int actualDimension = getClosestPower2Fit(exactPixelsPerSide);

            // Create the combined texture map for this SDF data
            System.Console.Out.WriteLine("\nOutput SDF texture resource will be of size " + actualDimension + "x" + actualDimension);
            Bitmap combined = new Bitmap(actualDimension, actualDimension);
            using (Graphics g = Graphics.FromImage(combined))
            {
                // Iterate through all characters for the SDF texture map
                int row = 0, col = 0;
                for (int ch = minchar; ch <= maxchar; ++ch)
                {
                    // Determine location in the target texture
                    if (++col == exactCountPerSide)
                    {
                        col = 0;
                        ++row;
                    }

                    // Invoke SDF generation.  Example: .\msdfgen.exe sdf -autoframe -size 64 64 -o outputfile.png -font .\tahoma.ttf 'z'
                    System.Console.Out.WriteLine("Generating SDF component for '" + (char)ch + "' (" + ch + ")");
                    string componentPath = componentFilePath(tempDirPath, ch);

                    var process = new Process();
                    process.StartInfo = new ProcessStartInfo(exPath,
                        sdfMode + " -autoframe -size " + decalSize + " " + decalSize +
                        " -o " + ("\"" + componentPath + "\"") +
                        " -font " + sourceTTF + " " + ch)
                    {
                        UseShellExecute = false
                    };

                    process.Start();
                    process.WaitForExit();

                    // Make sure file is present
                    if (!(new FileInfo(componentPath).Exists))
                    {
                        System.Console.Out.WriteLine("Error: Component not present for \"" + componentPath + "\"");
                        Environment.Exit(1);
                    }

                    // Add to combined texture
                    Bitmap source = new Bitmap(Bitmap.FromFile(componentPath));
                    Point location = new Point((col * decalSize), (row * decalSize));
                    g.DrawImage(source, location.X, location.Y);

                    // Determine actual glyph size (TODO: analyse component image to allow variable-width font)
                    Size size = new Size(decalSize, decalSize);

                    // Write an entry to the xml data file 
                    addFontGlyphDataEntry(xmlDocument, fontNode, ch, location, size);
                }
            }

            // Save the consolidated SDF texture map
            System.Console.Out.WriteLine("\nSaving consolidated SDF texture map to \"" + output.FullName + "\"");
            if (output.Exists) output.Delete();
            combined.Save(output.FullName);

            // Save the xml font data file
            System.Console.Out.WriteLine("\nSaving SDF font data file to \"" + outputData.FullName + "\"");
            if (outputData.Exists) outputData.Delete();
            saveXmlDataDocument(xmlDocument, outputData.FullName);

            // Report success and exit
            System.Console.Out.WriteLine("\nSDF generation complete");
        }

        private XmlDocument createXmlDataDocument()
        {
            return new XmlDocument();
        }

        private XmlNode writeXmlDataHeader(XmlDocument doc)
        {
            var root = doc.CreateElement("GameData");
            doc.AppendChild(root);

            var font = root.AppendChild(doc.CreateElement("Font"));

            var code = font.AppendChild(doc.CreateElement("Code"));
            code.InnerText = "(CODE)";

            var texture = font.AppendChild(doc.CreateElement("Texture"));
            texture.InnerText = "(TEXTURE)";

            return font;
        }

        private void addFontGlyphDataEntry(XmlDocument document, XmlNode fontNode, int character, Point location, Size size)
        {
            var node = fontNode.AppendChild(document.CreateElement("Glyph"));
            node.Attributes.Append(createAttribute(document, "ch", character));
            node.Attributes.Append(createAttribute(document, "x", location.X));
            node.Attributes.Append(createAttribute(document, "y", location.Y));
            node.Attributes.Append(createAttribute(document, "sx", size.Width));
            node.Attributes.Append(createAttribute(document, "sy", size.Height));
        }

        private XmlAttribute createAttribute(XmlDocument document, string name, object value)
        {
            var attr = document.CreateAttribute(name);
            attr.Value = value.ToString();

            return attr;
        }

        private void saveXmlDataDocument(XmlDocument document, string filename)
        {
            document.Save(filename);
        }

        private string componentFilePath(String componentDirectory, int character)
        {
            return (componentDirectory + "\\sdf-" + character + ".png");
        }

        private int getClosestPower2Fit(int exactSize)
        {
            // No point starting lower than decalSize.  Limit to <= 4k textures
            for (int size = decalSize; size <= 4096; size *= 2)
            {
                if (size >= exactSize) return size;
            }

            throw new ArgumentException("Desired SDF output exceeds 4k texture size limit");            
        }


        private void PrepareTemporaryDirectory(String path)
        {
            DeleteTemporaryDirectory(path);

            new DirectoryInfo(path).Create();
        }

        private void DeleteTemporaryDirectory(String path)
        { 
            DirectoryInfo directory = new DirectoryInfo(path);
            if (directory.Exists)
            {
                // Clear all .png files before attempting to delete
                FileInfo[] files = directory.GetFiles("*.png");
                foreach (FileInfo png in files)
                {
                    png.Delete();
                }

                directory.Delete();
            }
        }

        public bool isRetainingIntermediateFile() { return retainIntermediateFiles; }
        public void setRetainIntermediateFiles(bool retainFiles) { retainIntermediateFiles = retainFiles; }

        public string getSdfGenExPath() { return sdfGenExPath; }
        public void setSdfGenExPath(string path) { sdfGenExPath = path; }

        public int getDecalSize() { return decalSize; }
        public void setDecalSize(int size) { decalSize = size; }

        public string getSdfMode() { return sdfMode; }
        public void setSdfMode(String mode) { sdfMode = mode; }
    }
}
