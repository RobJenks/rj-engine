﻿using System;
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

        private static int COMPONENT_DECAL_SCALE_FACTOR = 3;
        private bool retainIntermediateFiles = false;
        private string sdfGenExPath = MsdfgenEx.determineDefaultPath();
        private int decalSize = 16;
        private string sdfMode = SdfMode.SignedDistanceField;

        public void GenerateSDFTexture(string sourceTTF, string outputFile, string outputDataFile, int minchar, int maxchar)
        {
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

            // Determine the dimensions and size of the target texture; closest to square as possible is 
            // optimum due to power-of-two optimisations
            int count = (maxchar - minchar + 1);
            double exactDimension = Math.Sqrt(count);
            int exactCountPerSide = (int)Math.Ceiling(exactDimension);
            
            // Iterate through all characters for the SDF texture map
            for (int ch = minchar; ch <= maxchar; ++ch)
            {
                // Invoke SDF generation.  Example: .\msdfgen.exe sdf -autoframe -size 64 64 -o outputfile.png -font .\tahoma.ttf 'z'
                System.Console.Out.WriteLine("Generating SDF component for '" + (char)ch + "' (" + ch + ")");
                string componentPath = componentFilePath(tempDirPath, ch);

                runGenProcess(
                    sdfMode +
                    " -size " + componentDecalSize + " " + componentDecalSize +
                    " -translate " + decalSize + " " + decalSize +
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
                Bitmap source = new Bitmap(Bitmap.FromFile(componentPath));
                Rectangle bounds = determineActualGlyphBounds(source, new Rectangle(new Point(0, 0), new Size(componentDecalSize, componentDecalSize)), defaultDecalBounds);

                // Store an entry in the glyph collection
                glyphs.Add(new GlyphData(ch, componentPath, bounds));
                
                // Maintain a record of cross-glyph sizes
                minY = Math.Min(minY, bounds.Y);
                maxY = Math.Max(maxY, bounds.Y + bounds.Height);
            }

            // Glyphs will be arranged as optimally as possible on a 2D grid within the pow2 texture
            int row = 0, col = 0;
            Point currentLocation = new Point(0, 0);
            int glyphHeight = (maxY - minY);
            System.Console.Out.WriteLine("Actual glyph height will be (" + maxY + " - " + minY + ") = " + glyphHeight + " to account for leading/trailing elements");

            // Now generate the consolidated texture map based on this glyph data
            int maxPixelsPerSide = (exactCountPerSide * glyphHeight);
            int actualDimension = getClosestPower2Fit(maxPixelsPerSide);
            System.Console.Out.WriteLine("\nGenerating consolidated SDF texture resource with size " + actualDimension + "x" + actualDimension);

            Bitmap combined = new Bitmap(actualDimension, actualDimension);
            using (Graphics g = Graphics.FromImage(combined))
            {
                foreach (GlyphData glyph in glyphs)
                {
                    // Glyphs will be allowed variable-width but will all have a consistent y-range
                    Bitmap source = new Bitmap(Bitmap.FromFile(glyph.Filename));
                    Rectangle glyphBounds = new Rectangle(
                        new Point(glyph.Bounds.Location.X, minY), 
                        new Size(glyph.Bounds.Width, glyphHeight));

                    // Render glyph within the combined texture
                    g.DrawImage(source, currentLocation.X, currentLocation.Y, glyphBounds, GraphicsUnit.Pixel);

                    // Add an entry to the glyph data file
                    addFontGlyphDataEntry(xmlDocument, fontNode, glyph.Character, 
                        new Rectangle(new Point(currentLocation.X, currentLocation.Y), glyphBounds.Size));

                    // Move the current rendering position on
                    currentLocation.X += glyphBounds.Width;

                    // Update grid position in the consolidated texture
                    if (++col == exactCountPerSide)
                    {
                        // Move to the first column of the next row
                        col = 0;
                        ++row;

                        // Calculate our new starting position and reset the glyph height record
                        currentLocation = new Point(0, row * glyphHeight);
                    }
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
            tempGlyph.Refresh();
            if (!tempGlyph.Exists)
            {
                throw new Exception("Failed to generate reference glyph output; cannot proceed");
            }

            // Determine the bounds of this reference glyph and use it to scale all other glyphs in the set
            Rectangle bounds = determineActualGlyphBounds(new Bitmap(Bitmap.FromFile(tempGlyph.FullName)), new Rectangle(0, 0, fullSize, fullSize), new Rectangle(0, 0, fullSize, fullSize));
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