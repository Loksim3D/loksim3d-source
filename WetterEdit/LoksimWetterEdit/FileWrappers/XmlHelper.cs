using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace Loksim3D.WetterEdit.FileWrappers
{
    /// <summary>
    /// Extension-Methods zum einfachen Lesen von XML-Attributen
    /// </summary>
    public static class XmlHelper
    {
        /// <summary>
        /// Liefert Zeitpunkt von XML-Attribut
        /// <br>XML-Attribut muss dabei zu einem Double konvertierbar sein und das Attribut wird in Sekunden (seit 00:00 Uhr) interpretiert</br>
        /// </summary>
        /// <param name="attr">Attribut aus welchem gelesen wird (kann null sein)</param>
        /// <param name="defaultTime">Rückgabewert falls nicht gelesen/konvertiert werden kann</param>
        /// <returns>Gelesener Zeitpunkt oder default Wert</returns>
        public static TimeSpan ReadTime(this XAttribute attr, TimeSpan defaultTime = default (TimeSpan))
        {
            double secs;
            if (attr != null && Double.TryParse(attr.Value, out secs))
            {
                return new TimeSpan(0, 0, 0, 0, (int)(secs * 1000));
            }
            return defaultTime;
        }

        /// <summary>
        /// Liest einen "Standard-Typ" aus XML-Attribut
        /// <br>Zur Konvertiert wird System.Convert.ChangeType verwendet</br>
        /// </summary>
        /// <typeparam name="T">Typ des zu lesenden Attributs</typeparam>
        /// <param name="attr">Attribut (kann null sein)</param>
        /// <param name="defaultValue">Rückgabewert falls nicht gelesen/konvertiert werden kann</param>
        /// <returns>Gelesenes Attribut oder default Wert</returns>
        public static T Read<T>(this XAttribute attr, T defaultValue = default(T))
        {
            if (attr != null)
            {
                try
                {
                    return (T)Convert.ChangeType(attr.Value, typeof (T), System.Globalization.CultureInfo.InvariantCulture);
                }
                catch (Exception)
                {
                }
            }
            return defaultValue;
        }
    }
}
