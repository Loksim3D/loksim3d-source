using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Globalization;

namespace Loksim3D.WetterEdit.Views.Rules
{
    /// <summary>
    /// Regel welche VariablenNamen überprüft
    /// </summary>
    public class VariableNameRule : ValidationRule
    {
        public override ValidationResult Validate(object value, CultureInfo cultureInfo)
        {
            var str = value as string;
            if (string.IsNullOrWhiteSpace(str))
            {
                return new ValidationResult(false, Resources.Strings.VarValidationEmpty);
            }

            if (Char.IsNumber(str[0]))
            {
                return new ValidationResult(false, Resources.Strings.VarValidationNumber);
            }

            foreach (char c in str)
            {
                if (!(Char.IsLetter(c) || Char.IsNumber(c)))
                {
                    return new ValidationResult(false, String.Format(CultureInfo.CurrentCulture, Resources.Strings.VarValidation_0_Illegal, c));
                }
            }

            return new ValidationResult(true, null);
        }
    }
}
