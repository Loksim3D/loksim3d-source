using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace Loksim3D.WetterEdit.Utils.Attributes
{
    class LocalizedDisplayNameAttribute : DisplayNameAttribute
    {
        private readonly string _displayName;
        public LocalizedDisplayNameAttribute(string displayName)
            : base()
        {
            if (displayName == null)
            {
                throw new ArgumentException("displayName must not be null", "displayName");
            }
            _displayName = displayName;
        }

        public override string DisplayName
        {
            get
            {
                return Resources.Strings.ResourceManager.GetString(_displayName, Resources.Strings.Culture) ?? _displayName;
            }
        }
    }
}
