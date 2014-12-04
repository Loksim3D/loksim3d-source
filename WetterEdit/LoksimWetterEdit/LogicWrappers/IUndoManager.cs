using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Loksim3D.WetterEdit.LogicWrappers
{
    /// <summary>
    /// Interface welches für undo/redo benötigte Methoden bereitstellt
    /// </summary>
    public interface IUndoManager
    {
        /// <summary>
        /// Fügt Action hinzu welche die gerade ausgeführt Änderung rückgängig machen kann
        /// </summary>
        /// <param name="undoAction">Aktion welche gerade ausgeführte Änderung rückgängig machen kann
        /// <br>Beim Rückgängigmachen sollte ebenfalls AddUndoAction aufgerufen werden, damit ein Redo möglich ist</br></param>
        void AddUndoAction(Action undoAction);

        /// <summary>
        /// Löscht alle Undo und Redo Aktionen
        /// </summary>
        void ClearUndoRedo();

        /// <summary>
        /// Beginnt Abschnitt in welchem keine Undo / Redo Informationen gesammelt werden
        /// </summary>
        /// <returns>Token bei welchem Dispose aufgerufen werden muss, sobald wieder Undo / Redo Infos gesammelt werden sollen</returns>
        IDisposable EnterNoUndoSection();

        /// <summary>
        /// Führt zuletzt hinzugefügte Undo-Action aus
        /// </summary>
        void Undo();

        /// <summary>
        /// Führt die zuletzte hinzugefügte Undo-Action aus, die beim Ausführen einer Undo-Action zum Manager hinzgefügt wurde
        /// <br>Mit anderen Worten, führt Redo aus</br>
        /// </summary>
        void Redo();

        /// <summary>
        /// Liefert true falls eine Undo Aktion möglich ist
        /// </summary>
        bool UndoPossible { get; }


        /// <summary>
        /// Liefert true falls eine Redo Aktion möglich ist
        /// </summary>
        bool RedoPossible { get; }
    }
}
