using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace SdfDecalBuilder
{
    class GlyphData
    {
        public int Character { get; set; }
        public string Filename { get; set; }
        public Rectangle Bounds { get; set; }

        public GlyphData(int character, string filename, Rectangle bounds)
        {
            Character = character;
            Filename = filename;
            Bounds = bounds;
        }
    }
}
