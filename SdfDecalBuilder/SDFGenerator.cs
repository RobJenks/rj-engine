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

        public static int DEFAULT_TEXTURE_MAP_SIZE = 512;
        public static int DEFAULT_DECAL_SIZE = 16;
        public static string DEFAULT_SDF_MODE = SdfMode.SignedDistanceField;
        public static int DEFAULT_SPACE_WIDTH = 8;

        private static int COMPONENT_DECAL_SCALE_FACTOR = 3;    // Scale factor to desired decal size, to ensure leading/trailing glyph data is captured
        private static int GLYPH_MAP_SEPARATION = 2;            // Separation (px) between components on the glyph map, to prevent texture sampling bleed
        private string sdfGenExPath = MsdfgenEx.determineDefaultPath();
        private int textureSize = DEFAULT_TEXTURE_MAP_SIZE;
        private int decalSize = DEFAULT_DECAL_SIZE;
        private int spaceWidth = DEFAULT_SPACE_WIDTH;
        private string sdfMode = SdfMode.SignedDistanceField;

        public void GenerateSDFTexture(string sourceTTF, string outputFile, string outputDataFile, int minchar, int maxchar)
        {
            int glyphCount = (maxchar - minchar + 1);
            List<GlyphData> glyphs = new List<GlyphData>();

            // Determine a target directory and a place to store intermediate images
            FileInfo output = new FileInfo(outputFile);
            FileInfo outputData = new FileInfo(outputDataFile);
            string tempDirPath = output.Directory.FullName + "\\sdfgen_tmp";
            PrepareTemporaryDirectory(tempDirPath);

            // Create the xml data document for this font
            var xmlDocument = createXmlDataDocument();
            var fontNode = writeXmlDataHeader(xmlDocument);

            // Scale factor for TTF glyphs will be determined based upon a reference character and desired glyph size
            float scale = determineGlyphScaleFactor(sourceTTF, tempDirPath, decalSize);

            // Component glyphs will be scaled up by a factor to ensure they are fully-covered, e.g. where they
            // extend further in some directions that the reference character
            int componentDecalSize = decalSize * COMPONENT_DECAL_SCALE_FACTOR;
            int translate = (componentDecalSize / 2) - decalSize;
            Rectangle defaultDecalBounds = new Rectangle(translate, translate, decalSize, decalSize);   // Expected size within the larger area

            // Keep track of cross-glyph sizes so we can generate a consistently-arranged font
            int minY = int.MaxValue;
            int maxY = int.MinValue;
            
            // Iterate through all characters for the SDF texture map
            for (int ch = minchar; ch <= maxchar; ++ch)
            {
                // Invoke SDF generation.  Example: .\msdfgen.exe sdf -autoframe -size 64 64 -o outputfile.png -font .\tahoma.ttf 'z'
                System.Console.Out.WriteLine("Generating SDF component for '" + (char)ch + "' (" + ch + ")");
                string componentPath = componentFilePath(tempDirPath, ch);

                runGenProcess(
                    sdfMode +
                    " -size " + componentDecalSize + " " + componentDecalSize +
                    " -translate " + translate + " " + translate +
                    " -scale " + scale.ToString() +
                    " -o " + ("\"" + componentPath + "\"") +
                    " -font " + sourceTTF + " " + ch);

                // Make sure file is present
                if (!(new FileInfo(componentPath).Exists))
                {
                    System.Console.Out.WriteLine("Error: Component not present for \"" + componentPath + "\"");
                    Environment.Exit(1);
                }

                // Determine actual glyph size
                Bitmap source = new Bitmap(Image.FromFile(componentPath));
                Rectangle bounds = determineActualGlyphBounds(source, new Rectangle(new Point(0, 0), new Size(componentDecalSize, componentDecalSize)), defaultDecalBounds);
                source.Dispose();

                // Store an entry in the glyph collection
                glyphs.Add(new GlyphData(ch, componentPath, bounds));
                
                // Maintain a record of cross-glyph sizes
                minY = Math.Min(minY, bounds.Y);
                maxY = Math.Max(maxY, bounds.Y + bounds.Height);
            }

            // Glyphs will be arranged as optimally as possible on a 2D grid within the pow2 texture
            int row = 0;
            Point currentLocation = new Point(0, 0);
            int glyphHeight = (maxY - minY);
            System.Console.Out.WriteLine("Actual glyph height will be (" + maxY + " - " + minY + ") = " + glyphHeight + " to account for leading/trailing elements");

            // Special case: set the ' ' glyph size based upon the input space width, since we can't determine it from TTF pixel data
            var space = glyphs.Find(x => (x.Character == (int)' '));
            if (space == null) throw new Exception("No ' ' entry in glyph map, font is invalid");
            space.Bounds = new Rectangle(space.Bounds.X, space.Bounds.Y, spaceWidth, space.Bounds.Height);

            // Now generate the consolidated texture map based on this glyph data
            System.Console.Out.WriteLine("\nGenerating consolidated SDF texture resource with size " + textureSize + "x" + textureSize);
            Bitmap combined = new Bitmap(textureSize, textureSize);
            using (Graphics g = Graphics.FromImage(combined))
            {
                foreach (GlyphData glyph in glyphs)
                {
                    // Glyphs will be allowed variable-width but will all have a consistent y-range
                    Bitmap source = new Bitmap(Image.FromFile(glyph.Filename));
                    Rectangle glyphBounds = new Rectangle(
                        new Point(glyph.Bounds.Location.X, minY), 
                        new Size(glyph.Bounds.Width, glyphHeight));

                    // Test whether we need to move to a new row, if this glyph will not fit in the remaining space
                    if ((currentLocation.X + glyph.Bounds.Width + GLYPH_MAP_SEPARATION) >= textureSize)
                    {
                        // Calculate our new starting position in the next row of the texture map
                        ++row;
                        currentLocation = new Point(0, row * glyphHeight);
                    }

                    // Render glyph within the combined texture
                    g.DrawImage(source, currentLocation.X, currentLocation.Y, glyphBounds, GraphicsUnit.Pixel);

                    // Dispose of the component resources
                    source.Dispose();

                    // Add an entry to the glyph data file
                    addFontGlyphDataEntry(xmlDocument, fontNode, glyph.Character, 
                        new Rectangle(new Point(currentLocation.X, currentLocation.Y), glyphBounds.Size));

                    // Move the current rendering position on and add some padding to avoid texture sampling bleed
                    currentLocation.X += (glyphBounds.Width + GLYPH_MAP_SEPARATION);
                }
            }

            // Save the consolidated SDF texture map
            System.Console.Out.WriteLine("\nSaving consolidated SDF texture map to \"" + output.FullName + "\"");
            if (output.Exists) output.Delete();
            combined.Save(output.FullName);

            // Save the xml font data file
            System.Console.Out.WriteLine("Saving SDF font data file to \"" + outputData.FullName + "\"");
            if (outputData.Exists) outputData.Delete();
            saveXmlDataDocument(xmlDocument, outputData.FullName);

            // Report success and exit
            System.Console.Out.WriteLine("\nSDF generation complete\n");
        }

        private float determineGlyphScaleFactor(string sourceTTF, string tempDirPath, int decalSize)
        {
            // Use the height of a capital "I" as the reference full-height for a glyph
            string tempGlyphPath = tempDirPath + "\\_scale_adj_glyph.png";
            FileInfo tempGlyph = new FileInfo(tempGlyphPath);
            if (tempGlyph.Exists) tempGlyph.Delete();

            char referenceChar = 'I';
            int fullSize = 256;
            int translate = (fullSize / 2) - decalSize;

            // Run glyph generation and make sure the file is created
            runGenProcess("sdf -font \"" + sourceTTF + "\" " + (int)referenceChar + " -size " + fullSize + " " + fullSize + 
                " -translate " + translate + " " + translate + " -o \"" + tempGlyph.FullName + "\"");
            System.Threading.Thread.Sleep(250); // Allow filesystem actions to complete
            tempGlyph.Refresh();
            if (!tempGlyph.Exists)
            {
                throw new Exception("Failed to generate reference glyph output; cannot proceed");
            }

            // Determine the bounds of this reference glyph and use it to scale all other glyphs in the set
            Bitmap glyph = new Bitmap(Image.FromFile(tempGlyph.FullName));
            Rectangle bounds = determineActualGlyphBounds(glyph, new Rectangle(0, 0, fullSize, fullSize), new Rectangle(0, 0, fullSize, fullSize));
            glyph.Dispose();
            if (bounds.Height <= 0) throw new Exception("Invalid bounds generated by reference image");

            float scale = (float)decalSize / (float)bounds.Height;
            System.Console.Out.WriteLine("Reference image generated bounds of [" + rectToStringShort(bounds) + "]");
            System.Console.Out.WriteLine("Glyph scale factor = " + (float)decalSize + " / " + (float)bounds.Height + " = " + scale);

            return scale;
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

        private void addFontGlyphDataEntry(XmlDocument document, XmlNode fontNode, int character, Rectangle bounds)
        {
            var node = fontNode.AppendChild(document.CreateElement("Glyph"));
            node.Attributes.Append(createAttribute(document, "ch", character));
            node.Attributes.Append(createAttribute(document, "x", bounds.X));
            node.Attributes.Append(createAttribute(document, "y", bounds.Y));
            node.Attributes.Append(createAttribute(document, "sx", bounds.Width));
            node.Attributes.Append(createAttribute(document, "sy", bounds.Height));
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

        private string componentFilePath(string componentDirectory, int character)
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

        private void runGenProcess(String arguments)
        {
            var process = new Process();
            process.StartInfo = new ProcessStartInfo(getSdfGenExPath(), arguments)
            {
                UseShellExecute = false
            };

            process.Start();
            process.WaitForExit();
        }

        private void PrepareTemporaryDirectory(String path)
        {
            DeleteTemporaryDirectory(path);
            System.Threading.Thread.Sleep(250); // Allow filesystem actions to complete

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
                    try
                    {
                        if (png.Exists) png.Delete();
                    }
                    catch (Exception ex)
                    {
                        System.Console.Out.WriteLine("Warning: Unable to delete temporary glyph component \"" + png.Name + "\": " + ex.Message);
                    }
                }

                try
                {
                    directory.Delete();
                }
                catch (Exception ex)
                {
                    System.Console.Out.WriteLine("Warning: Unable to delete temporary glyph directory: " + ex.Message);
                }
            }
        }

        private Rectangle determineActualGlyphBounds(Bitmap glyph, Rectangle originalBounds, Rectangle defaultBounds)
        {
            int left = -1, top = -1, right = -1, bottom = -1;
            Size size = new Size(Math.Min(originalBounds.Width, glyph.Width), Math.Min(originalBounds.Height, glyph.Height));
            int black = Color.FromArgb(255, 0, 0, 0).ToArgb();

            // Get left-most point
            bool terminate = false;
            for (int x = 0; x < size.Width && !terminate; ++x)
            {
                for (int y = 0; y < size.Height && !terminate; ++y)
                {
                    if (glyph.GetPixel(x, y).ToArgb() != black)
                    {
                        left = x; terminate = true;
                    }
                }
            }

            // Top-most point
            terminate = false;
            for (int y = 0; y < size.Height && !terminate; ++y)
            {
                for (int x = 0; x < size.Width && !terminate; ++x)
                {
                    if (glyph.GetPixel(x, y).ToArgb() != black)
                    {
                        top = y; terminate = true;
                    }
                }
            }

            // Right-most point
            terminate = false;
            for (int x = size.Width - 1; x >= 0 && !terminate; --x)
            {
                for (int y = 0; y < size.Height && !terminate; ++y)
                {
                    if (glyph.GetPixel(x, y).ToArgb() != black)
                    {
                        right = x; terminate = true;
                    }
                }
            }

            // Bottom-most point
            terminate = false;
            for (int y = size.Height - 1; y >= 0 && !terminate; --y)
            {
                for (int x = 0; x < size.Width && !terminate; ++x)
                {
                    if (glyph.GetPixel(x, y).ToArgb() != black)
                    {
                        bottom = y; terminate = true;
                    }
                }
            }

            // Verify
            if (left == -1 || top == -1 || right == -1 || bottom == -1)
            {
                System.Console.Out.WriteLine("Warning: Could not determine actual glyph bounds (x=" + left + ", y=" + top + ", r=" + right + ", b=" + bottom + "); using defaults");
                return defaultBounds;
            }

            // Return a set of actual bounds for this glyph
            return new Rectangle(left, top, right - left, bottom - top);
        }

        string rectToStringShort(Rectangle rect)
        {
            return "x=" + rect.Left + ", y=" + rect.Top + ", w=" + rect.Width+ ", h=" + rect.Height;
        }
        
        public string getSdfGenExPath() { return sdfGenExPath; }
        public void setSdfGenExPath(string path) { sdfGenExPath = path; }

        public int getTextureMapSize() { return textureSize; }
        public void setTextureMapSize(int size) { if (size > 0) textureSize = size; }

        public int getDecalSize() { return decalSize; }
        public void setDecalSize(int size) { if (size > 0) decalSize = size; }

        public string getSdfMode() { return sdfMode; }
        public void setSdfMode(String mode) { sdfMode = mode; }

        public int getSpaceWidth() { return spaceWidth; }
        public void setSpaceWidth(int spacewidth) { spaceWidth = spacewidth; }
    }
}
