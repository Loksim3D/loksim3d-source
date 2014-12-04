using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace Loksim3D.WetterEdit.Utils.Attributes
{
    class LocalizedDescriptionAttribute : DescriptionAttribute
    {
        private readonly string _description;
        public LocalizedDescriptionAttribute(string description)
            : base()
        {
            if (description == null)
            {
                throw new ArgumentException("description must not be null", "description");
            }
            _description = description;
        }

        public override string Description
        {
            get
            {
                return Resources.Strings.ResourceManager.GetString(_description, Resources.Strings.Culture) ?? _description;
            }
        }
    }
}
