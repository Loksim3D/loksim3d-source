#pragma once

namespace l3d
{
namespace packageinstaller
{
	/**
	 * Diverse Commands für Agenten
	 */
	enum AgentCommand
	{
		/**
		 * Arbeit normal fortsetzen
		 */
		AgentContinueWork, 
		/**
		 * Sofort abbrechen aber bisher durchgeführte Änderungen commiten
		 */
		AgentCancelAndCommit, 
		/**
		 * Sofort abbrechen und Änderungen zurückrollen
		 */
		AgentCancelAndRollback, 
		/**
		 * Arbeit fortsetzen bis keine neue Arbeit mehr da ist, dann Änderungen commiten und Agent stoppen
		 */
		AgentStopAndCommit, 
		/**
		 * Arbeit fortsetzen bis keine neue Arbeit mehr da ist, dann Änderungen zurückrollen und Agent stoppen
		 */
		AgentStopAndRollback
	};

}
}