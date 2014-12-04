#pragma once

namespace l3d
{
namespace packageinstaller
{
	/**
	 * Diverse Commands f�r Agenten
	 */
	enum AgentCommand
	{
		/**
		 * Arbeit normal fortsetzen
		 */
		AgentContinueWork, 
		/**
		 * Sofort abbrechen aber bisher durchgef�hrte �nderungen commiten
		 */
		AgentCancelAndCommit, 
		/**
		 * Sofort abbrechen und �nderungen zur�ckrollen
		 */
		AgentCancelAndRollback, 
		/**
		 * Arbeit fortsetzen bis keine neue Arbeit mehr da ist, dann �nderungen commiten und Agent stoppen
		 */
		AgentStopAndCommit, 
		/**
		 * Arbeit fortsetzen bis keine neue Arbeit mehr da ist, dann �nderungen zur�ckrollen und Agent stoppen
		 */
		AgentStopAndRollback
	};

}
}