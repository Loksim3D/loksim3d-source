using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Markup;
using System.Windows.Data;

namespace Loksim3D.WetterEdit.Views.Converters
{
    /// <summary>
    /// Konvertiert object zu boolean
    /// <br>Falls Object != null ist, wird true geliefert, ansonsten false</br>
    /// <br>Konverzierung zurück ist nicht implementiert</br>
    /// </summary>
    public class ReferenceToBoolConverter : MarkupExtension, IValueConverter
    {
        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }

        #region IValueConverter Members

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value != null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
