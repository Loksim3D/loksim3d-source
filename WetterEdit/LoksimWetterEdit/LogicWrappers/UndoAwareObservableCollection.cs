using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;

namespace Loksim3D.WetterEdit.LogicWrappers
{
    public class UndoAwareObservableCollection<T> : ObservableCollection<T>
    {
        private IUndoManager _undoManager;

        public UndoAwareObservableCollection(IUndoManager undoManager)
        {
            _undoManager = undoManager;
        }

        public UndoAwareObservableCollection(IUndoManager undoManager, IEnumerable<T> collection) : base(collection)
        {
            _undoManager = undoManager;
        }

        public UndoAwareObservableCollection(IUndoManager undoManager, List<T> list)
            : base(list)
        {
            _undoManager = undoManager;
        }

        protected override void ClearItems()
        {
            var oldVal = this.Items.ToList();
            _undoManager.AddUndoAction(() =>
                {
                    foreach (var it in oldVal)
                    {
                        base.Add(it);
                    }
                });
            base.ClearItems();
        }

        protected override void InsertItem(int index, T item)
        {
            _undoManager.AddUndoAction(() =>
                {
                    this.RemoveAt(index);
                });
            base.InsertItem(index, item);
        }

        protected override void MoveItem(int oldIndex, int newIndex)
        {
            _undoManager.AddUndoAction(() =>
            {
                this.MoveItem(newIndex, oldIndex);
            });
            base.MoveItem(oldIndex, newIndex);
        }

        protected override void RemoveItem(int index)
        {
            var oldVal = base[index];
            _undoManager.AddUndoAction(() =>
            {
                this.InsertItem(index, oldVal);
            });
            base.RemoveItem(index);
        }

        protected override void SetItem(int index, T item)
        {
            T oldItem = base[index];
            _undoManager.AddUndoAction(() =>
                {
                    SetItem(index, oldItem);
                });
            base.SetItem(index, item);
        }
    }
}
