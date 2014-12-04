using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;

namespace Loksim3D.WetterEdit.LogicWrappers
{
    /// <summary>
    /// Hilfsmethoden für Bilder
    /// </summary>
    public static class ImageHelper
    {
        /// <summary>
        /// Skaliert Bild so, dass es in den Bereich <paramref name="maxWidth"/> x <paramref name="maxHeight"/> passt
        /// <br>Behält Größenverhältnis bei</br>
        /// </summary>
        /// <param name="originalImage">Bild welches skaliert werden soll</param>
        /// <param name="maxWidth">Maximale Breite</param>
        /// <param name="maxHeight">Maximale Höhe</param>
        /// <returns>Skaliertes Bild</returns>
        public static Bitmap ScaleBitmap(Image originalImage, int maxWidth, int maxHeight)
        {
            int width = originalImage.Width;
            int height = originalImage.Height;
            if (width > maxWidth)
            {
                double factor = (double)maxWidth / width;
                width = (int)(width * factor - 1.0);
                height = (int)(height * factor - 1.0);
            }
            if (height > maxHeight)
            {
                double factor = (double)maxHeight / height;
                width = (int)(width * factor - 1.0);
                height = (int)(height * factor - 1.0);
            }

            return new Bitmap(originalImage, width, height);
        }
    }
}
