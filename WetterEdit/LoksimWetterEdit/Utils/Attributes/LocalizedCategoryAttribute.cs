using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace Loksim3D.WetterEdit.Utils.Attributes
{
    class LocalizedCategoryAttribute : CategoryAttribute
    {
        public LocalizedCategoryAttribute(string category)
            : base(category)
        {      
        }

        protected override string GetLocalizedString(string value)
        {
            return Resources.Strings.ResourceManager.GetString(value, Resources.Strings.Culture) ?? value;
        }
    }
}
