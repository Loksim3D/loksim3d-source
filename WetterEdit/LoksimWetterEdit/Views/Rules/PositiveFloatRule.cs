using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Globalization;

namespace Loksim3D.WetterEdit.Views.Rules
{
    /// <summary>
    /// Regel welche prüft ob es ein positiver float Wert ist (>= 0)
    /// </summary>
    public class PositiveFloatRule : ValidationRule
    {
        public override ValidationResult Validate(object value, CultureInfo cultureInfo)
        {
            var str = value as string;
            float o;
            if (!float.TryParse(str, out o))
            {
                return new ValidationResult(false, Resources.Strings.PositiveIntegerValidation);
            }
            return new ValidationResult(true, null);
        }
    }
}
