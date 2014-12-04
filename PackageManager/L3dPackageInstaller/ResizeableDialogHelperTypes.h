#pragma once

#include "stdafx.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{
namespace resizeabledlghelper
{

typedef enum ANCHOR_FLAGS
{
	AF_LEFT             = 0x01,
	AF_RIGHT            = 0x02,
	AF_LEFT_AND_RIGHT   = 0x03,
	AF_TOP              = 0x04,
	AF_BOTTOM           = 0x08,
	AF_TOP_AND_BOTTOM   = 0x0C,
} ANCHOR_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS(ANCHOR_FLAGS);

struct ANCHOR
{
	ANCHOR()
	{
	}

	ANCHOR(DWORD _idControl, ANCHOR_FLAGS _aff) : idControl(_idControl), aff(_aff)
	{
	}

	DWORD idControl;
	ANCHOR_FLAGS aff;
};

}
}
}
}