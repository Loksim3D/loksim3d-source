using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.LogicWrappers;
using System.Linq.Expressions;

namespace Loksim3D.WetterEdit.FileWrappers
{
    public abstract class UndoAwareFile : LoksimFile, IUndoManager
    {
        private Stack<Action> _undoActions;
        private Stack<Action> _redoActions;
        private bool _undoInProgress;
        private bool _redoInProgress;
        private int _noUndoCnt;

        protected UndoAwareFile()
        {
            _undoActions = new Stack<Action>();
            _redoActions = new Stack<Action>();
            _undoInProgress = false;
            _redoInProgress = false;
            _noUndoCnt = 0;
        }


        /// <summary>
        /// Call PropertyChanged event if not null
        /// </summary>
        /// <param name="property">Property that has changed</param>
        protected void NotifyPropertyChanged(string property, Action undoAction)
        {
            AddUndoAction(undoAction);
            NotifyPropertyChanged(property);
         }

        protected void NotifyPropertyChanged<T>(Expression<Func<T>> propertyExpression,  Action undoAction)
        {
            AddUndoAction(undoAction);
            NotifyPropertyChanged(propertyExpression);
        }

        public void AddUndoAction(Action undoAction)
        {
            if (_noUndoCnt == 0)
            {
                if (!_undoInProgress)
                {
                    _undoActions.Push(undoAction);
                    if (!_redoInProgress)
                    {
                        _redoActions.Clear();
                    }
                }
                else
                {
                    _redoActions.Push(undoAction);
                }
            }
        }

        public void ClearUndoRedo()
        {
            _undoActions.Clear();
            _redoActions.Clear();
        }

        public bool UndoPossible
        {
            get { return _undoActions.Count > 0; }
        }

        public bool RedoPossible
        {
            get { return _redoActions.Count > 0; }
        }

        public void Undo()
        {
            if (_undoActions.Count > 0)
            {
                _undoInProgress = true;
                try
                {
                    _undoActions.Pop().Invoke();
                }
                finally
                {
                    _undoInProgress = false;
                }
            }
        }

        public void Redo()
        {
            if (_redoActions.Count > 0)
            {
                _redoInProgress = true;
                try
                {
                    _redoActions.Pop().Invoke();
                }
                finally
                {
                    _redoInProgress = false;
                }
            }
        }

        public IDisposable EnterNoUndoSection()
        {
            _noUndoCnt++;
            return new NoUndoToken(() => _noUndoCnt--);
        }
    }
}
