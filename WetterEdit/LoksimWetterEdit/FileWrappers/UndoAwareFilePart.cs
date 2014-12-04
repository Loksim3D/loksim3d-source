using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.LogicWrappers;
using System.Linq.Expressions;

namespace Loksim3D.WetterEdit.FileWrappers
{
    public class UndoAwareFilePart : BaseModel
    {
        IUndoManager _undoManager;

        public UndoAwareFilePart(IUndoManager undoManager)
        {
            _undoManager = undoManager;
        }

        /// <summary>
        /// Call PropertyChanged event if not null
        /// </summary>
        /// <param name="property">Property that has changed</param>
        protected void NotifyPropertyChanged(string property, Action undoAction)
        {
            _undoManager.AddUndoAction(undoAction);
            NotifyPropertyChanged(property);
        }

        protected void NotifyPropertyChanged<T>(Expression<Func<T>> propertyExpression, Action undoAction)
        {
            _undoManager.AddUndoAction(undoAction);
            NotifyPropertyChanged(propertyExpression);
        }
    }
}
