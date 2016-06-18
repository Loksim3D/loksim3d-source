#pragma once

namespace l3d
{
namespace files
{
static const auto FILE_GENERAL_PROPS = L"Props";
static const auto FILE_GENERAL_AUTHOR = L"FileAuthor";
static const auto FILE_GENERAL_INFO = L"FileInfo";
static const auto FILE_GENERAL_PICTURE = L"FilePicture";
static const auto FILE_GENERAL_DOC = L"FileDoku";
static const auto FILE_GENERAL_EDITOR_VERSION = L"FileEditorVersion";
static const auto FILE_GENERAL_WRITE_TIME = L"WriteTime";

static const auto FILE_GENERAL_RECT_BOTTOM = L"y2";
static const auto FILE_GENERAL_RECT_LEFT = L"x1";
static const auto FILE_GENERAL_RECT_RIGHT = L"x2";
static const auto FILE_GENERAL_RECT_TOP = L"y1";

static const auto FILE_OBJ_ROOT = L"OBJECT";
static const auto FILE_OBJ_AUTO_ROTATE = L"AutoRotate";
static const auto FILE_OBJ_LIGHTED = L"Beleuchtet";
static const auto FILE_OBJ_DRAW_BACK = L"Drawrueckseiten";
static const auto FILE_OBJ_IS_AREA_NORMAL = L"NormalePerFlaeche";
static const auto FILE_OBJ_TEXTURE = L"Texture";
static const auto FILE_OBJ_TEXTURESIZE = L"TextureSize";
static const auto FILE_OBJ_TRANS_TEXTURE = L"TransTexture";
static const auto FILE_OBJ_TRANS_TYPE = L"TransparentTyp";
static const auto FILE_OBJ_TRANS_LEGACY = L"Transparent";
static const auto FILE_OBJ_POINT = L"Point";
static const auto FILE_OBJ_POINT_VECTOR = L"Vekt";
static const auto FILE_OBJ_POINT_NORMAL = L"Normal";
static const auto FILE_OBJ_AREA = L"Flaeche";
static const auto FILE_OBJ_AREA_POINTS = L"Points";
static const auto FILE_OBJ_AREA_TEXTURE_COORDS = L"Texture";
static const auto FILE_OBJ_AREA_NORMAL = L"Normal";

static const auto FILE_GRPOBJ_ROOT = L"GRUPPENOBJECT";
static const auto FILE_GRPOBJ_OBJECT = L"Object";
static const auto FILE_GRPOBJ_ENTRY_PATH = L"Name";
static const auto FILE_GRPOBJ_ENTRY_POSITION = L"Position";
static const auto FILE_GRPOBJ_ENTRY_ROTATION = L"Rotation";
static const auto FILE_GRPOBJ_ENTRY_SCALE = L"Scale";
static const auto FILE_GRPOBJ_ENTRY_BLINK = L"BlinkIntervall";
static const auto FILE_GRPOBJ_ENTRY_DYNVISIBILITY = L"DynamicVisibility";
static const auto FILE_GRPOBJ_ENTRY_FIXEDDYNVISIBILITY = L"FixedDynamicVisibility";

// <LegacyStuff>
static const auto FILE_GRPOBJ_ENTRY_HIDEON = L"HideOn";
static const auto FILE_GRPOBJ_ENTRY_SHOWON = L"ShowOn";
static const auto FILE_GRPOBJ_ENTRY_ONLYIFBRIGHTNESS = L"ShowIfBrightness";
static const auto FILE_GRPOBJ_ENTRY_BRIGHTNESS_VALUE = L"Brightness";
static const auto FILE_GRPOBJ_ENTRY_BRIGHTNESS_GREATERAS = L"GreaterAs";
// </LegacyStuff>

static const auto FILE_GRPOBJ_OBJECT_ZOOM = L"ZoomFaktor";
static const auto FILE_GRPOBJ_OBJECT_WEITSICHTBAR = L"WeitSichtbar";
static const auto FILE_GRPOBJ_FONT = L"Font";
static const auto FILE_GRPOBJ_FONT_ALIGNHORZ = L"AlignHorz";
static const auto FILE_GRPOBJ_FONT_ALIGNVERT = L"AlignVert";
static const auto FILE_GRPOBJ_FONT_COLOR = L"Color";
static const auto FILE_GRPOBJ_FONT_DEFAULTTEXT = L"DefaultText";
static const auto FILE_GRPOBJ_FONT_HEIGHT = L"Fonthoehe";
static const auto FILE_GRPOBJ_FONT_GROUPPROP = L"GroupProperty";
static const auto FILE_GRPOBJ_FONT_LIGHTED = L"Leuchtend";
static const auto FILE_GRPOBJ_FONT_NORM_BOTTOMLEFT = L"NormBottomLeft";
static const auto FILE_GRPOBJ_FONT_NORM_BOTTOMRIGHT = L"NormBottomRight";
static const auto FILE_GRPOBJ_FONT_NORM_TOPLEFT = L"NormTopLeft";
static const auto FILE_GRPOBJ_FONT_NORM_TOPRIGHT = L"NormTopRight";
static const auto FILE_GRPOBJ_FONT_VERTIKAL = L"Vertikal";


static const auto FILE_FONT_ROOT = L"FONT";
static const auto FILE_FONT_TEXTURE = L"Texture";
static const auto FILE_FONT_HEIGHT = L"Fonthoehe";
static const auto FILE_FONT_CHARNODE = L"Zeichen";
static const auto FILE_FONT_CHAR = L"Zeichen";
static const auto FILE_FONT_WIDTH = L"Width";
static const auto FILE_FONT_POSX = L"X";
static const auto FILE_FONT_POSY = L"Y";

static const auto FILE_RAIL_ROOT = L"RAIL";
static const auto FILE_RAIL_TEXTURE = L"Texture";
static const auto FILE_RAIL_TRANSPARENCY = L"TransparencyType";
static const auto FILE_RAIL_SCHWARZ_TRANSPARENT = L"Transparent";
static const auto FILE_RAIL_SPURBREITE = L"Spurbreite";
static const auto FILE_RAIL_SCHIENE_HOEHE = L"Schienenhoehe";
static const auto FILE_RAIL_SCHIENE_BREITE = L"Schienenbreite";
static const auto FILE_RAIL_LEGACY_NORMALS = L"LegacyNormals"; // Normalen bei jeder Flaeche 0/1/0 und nicht senkrecht
static const auto FILE_RAIL_NULLRAIL = L"NullRail"; // Rail wird nicht gerendert
static const auto FILE_RAIL_BETTUNG_BREITE = L"Bettungsbreite";
static const auto FILE_RAIL_TEX_SCHIENE_OBEN = L"TexSchieneOben";
static const auto FILE_RAIL_TEX_SCHIENE_SEITE = L"TexSchieneSeite";
static const auto FILE_RAIL_TEX_BETTUNG = L"TexBettung";
static const auto FILE_RAIL_BETTUNG_HOEHE_NODE = L"Hoehe";
static const auto FILE_RAIL_BETTUNG_HOEHE_ATTR = L"Hoehe";

static const auto FILE_BACKUPINFO_ROOT = L"BackupInfo";
static const auto FILE_BACKUPINFO_FILE = L"SavedFile";
static const auto FILE_BACKUPINFO_FILE_ORIGNAME = L"OrigFilename";
static const auto FILE_BACKUPINFO_FILE_BACKUPNAME = L"BackupFilename";
}
}