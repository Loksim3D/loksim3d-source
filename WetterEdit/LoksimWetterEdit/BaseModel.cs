using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Diagnostics;
using System.Reflection;
using System.Linq.Expressions;
using Loksim3D.WetterEdit.Utils;

namespace Loksim3D.WetterEdit
{
    /// <summary>
    /// Basisklasse für alle Klassen welche INotifyPropertyChanged implementieren
    /// </summary>
    public class BaseModel : INotifyPropertyChanged
    {
        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Call PropertyChanged event if not null
        /// </summary>
        /// <param name="property">Property that has changed</param>
        protected void NotifyPropertyChanged(string property)
        {
            VerifyPropertyExists(property);
            PropertyChangedEventHandler handler = this.PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(property));
            }

        }

        protected void NotifyPropertyChanged(params string[] propertyNames)
        {
            if (propertyNames == null) throw new ArgumentNullException("propertyNames");

            foreach (var name in propertyNames)
            {
                this.NotifyPropertyChanged(name);
            }
        }

        protected void NotifyPropertyChanged<T>(Expression<Func<T>> propertyExpression)
        {
            var propertyName = PropertySupport.ExtractPropertyName(propertyExpression);
            this.NotifyPropertyChanged(propertyName);
        }

        [Conditional("DEBUG")]
        private void VerifyPropertyExists(string propertyName)
        {
            PropertyInfo currentProperty = GetType().GetProperty(propertyName);
            Debug.Assert(currentProperty != null, string.Format("Property Name \"{0}\" does not exist in {1}", propertyName, GetType()));
        }

        #endregion

    }
}

