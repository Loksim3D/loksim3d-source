#pragma once

#include "XpOpenFileDlg.h"
#include "VistaOpenFileDlg.h"
#include <memory>

/**
 * Factory-Methode f�r OpenFileDialog Objekt
 * Liefert unter Vista oder h�her den VistaOpenFileDlg und sonst XpOpenFileDlg
 */
std::unique_ptr<OpenFileDlg> GetOpenFileDlg();

