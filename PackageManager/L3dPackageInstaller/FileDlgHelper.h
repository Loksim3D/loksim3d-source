#pragma once

#include "XpOpenFileDlg.h"
#include "VistaOpenFileDlg.h"
#include <memory>

/**
 * Factory-Methode für OpenFileDialog Objekt
 * Liefert unter Vista oder höher den VistaOpenFileDlg und sonst XpOpenFileDlg
 */
std::unique_ptr<OpenFileDlg> GetOpenFileDlg();

