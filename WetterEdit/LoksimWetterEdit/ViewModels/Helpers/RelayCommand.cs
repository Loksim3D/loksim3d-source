using System.Windows.Input;
using System;
using System.Diagnostics;

namespace Loksim3D.WetterEdit.ViewModels.Helpers
{
    /// <summary>
    /// Vereinfacht Handhabung von Commands
    /// </summary>
    public class RelayCommand : ICommand
    {
        #region Fields

        readonly Action<object> _execute;
        readonly Predicate<object> _canExecute;

        private EventHandler myCanExecuteHandlers;


        #endregion // Fields

        #region Constructors

        /// <summary>
        /// Neuen Command erzeugen
        /// </summary>
        /// <param name="execute">Delegate welches beim Ausführen des Commands ausgeführt wird</param>
        public RelayCommand(Action<object> execute)
            : this(execute, null)
        {
        }

        /// <summary>
        /// Neuen Command erzeugen
        /// </summary>
        /// <param name="execute">Delegate welches beim Ausführen des Commands ausgeführt wird</param>
        /// <param name="canExecute">Delegate welches aufgerufen wird ob zu prüfen ob Command ausgeführt werden kann</param>
        public RelayCommand(Action<object> execute, Predicate<object> canExecute)
        {
            if (execute == null)
                throw new ArgumentNullException("execute");

            _execute = execute;
            _canExecute = canExecute;
        }
        #endregion // Constructors

        #region ICommand Members

        [DebuggerStepThrough]
        public bool CanExecute(object parameter)
        {
            return _canExecute == null ? true : _canExecute(parameter);
        }

        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; myCanExecuteHandlers += value; }
            remove { CommandManager.RequerySuggested -= value; myCanExecuteHandlers -= value; }
        }

        public void Execute(object parameter)
        {
            _execute(parameter);
        }

        #endregion // ICommand Members

        public void FireCanExecuteChanged(object sender, EventArgs e)
        {
            if (myCanExecuteHandlers != null)
            {
                myCanExecuteHandlers(sender, e);
            }
        }

    }



}