#pragma once

#include "Dialog.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Dialog welcher Hintergrund Operationen ausführt
 */
class BackgroundOperatorDlg : virtual public Dialog
{
public:
	/*
	 * Returns if operation is currently running
	 * @return true if running
	 */
	virtual bool IsOperationRunning() const = 0;

	/**
	 * Cancels the running operation; may ask user to test if he really wants to cancel
	 * @return true if operation was cancelled
	 */
	virtual bool SoftCancel() = 0;

};

}
}
}