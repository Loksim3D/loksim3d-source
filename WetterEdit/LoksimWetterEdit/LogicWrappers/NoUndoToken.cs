using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Loksim3D.WetterEdit.LogicWrappers
{
    /// <summary>
    /// Token welches beim Undo verwendet wird um die NoUndoSection wieder zu verlassen
    /// </summary>
    public sealed class NoUndoToken : IDisposable
    {
        private bool _disposed = false; // to detect redundant calls
        private Action _onDispose;

        public NoUndoToken(Action onDispose)
        {
            _onDispose = onDispose;
        }

        public void Dispose()
        {
            if (!_disposed)
            {
                if (_onDispose != null)
                {
                    _onDispose.Invoke();
                }
                _disposed = true;
            }
        }

        ~NoUndoToken()
        {
            Dispose();
        }

    }
}
