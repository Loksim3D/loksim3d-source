using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Windows.Markup;

namespace Loksim3D.WetterEdit.Views.Converters
{
    /// <summary>
    /// Konvertiert DateTime in TimeSpan und zurück
    /// <br>Wichtig ist nur "Zeit des Tages" und nicht Datum</br>
    /// </summary>
    class TimeSpanDateTimeConverter : MarkupExtension, IValueConverter
    {
        #region IValueConverter Members

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return ChangeType(value, targetType, parameter, culture);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return ChangeType(value, targetType, parameter, culture);
        }

        #endregion

        private object ChangeType(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if ((value is DateTime || value is DateTime?) && targetType == typeof(TimeSpan))
            {
                if (value != null)
                {
                    return ((DateTime)value).TimeOfDay;
                }
                else
                {
                    return new TimeSpan(0, 0, 0);
                }
            }
            if (value is TimeSpan && (targetType == typeof(DateTime) || targetType == typeof(DateTime?)))
            {
                TimeSpan t = (TimeSpan)(value ?? new TimeSpan());
                return new DateTime(2000, 1, 1, t.Hours, t.Minutes, t.Seconds, t.Milliseconds, culture.Calendar);
            }
            return System.Convert.ChangeType(value, targetType, culture);
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }
    }
}
