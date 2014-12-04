using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Loksim3D.WetterEdit.FileWrappers
{
    /// <summary>
    /// Klasse welche die Element und Attributsnamen aller Loksim-Dateien enthält
    /// </summary>
    public static class FileDescriptions
    {
        #region General (all) Files
        public const string FILE_GENERAL_EL_PROPS    = "Props";
            public const string FILE_GENERAL_AT_AUTHOR       = "FileAuthor";
            public const string FILE_GENERAL_AT_INFO         = "FileInfo";
            public const string FILE_GENERAL_AT_PICTURE      = "FilePicture";
            public const string FILE_GENERAL_AT_DOKU = "FileDoku";
        #endregion

        #region Sky Files
        public const string FILE_SKY_EL_ROOT     = "Sky";
            public const string FILE_SKY_EL_WEATHER_SETS = "WeatherSets";
                public const string FILE_SKY_EL_WEATHER      = "Weather";
                    public const string FILE_SKY_EL_WEATHER_PROPS    = "Props";
                        public const string FILE_SKY_AT_WEATHER_FILE = "WeatherFile";
                        public const string FILE_SKY_AT_WEATHER_PROBABILITY = "Probability";
        #endregion

        #region Weather Files
        public const string FILE_WEATHER_EL_ROOT = "Weather";
            public const string FILE_WEATHER_AT_PROPS_HEADER = "Header";
            public const string FILE_WEATHER_EL_ILLUMINATION = "AmbientIllumination";
                public const string FILE_WEATHER_EL_ILLUMINATION_PROPS = "Props";
                    public const string FILE_WEATHER_AT_ILLUMINATION_DAWN_BEGIN = "DawnBegin";
                    public const string FILE_WEATHER_AT_ILLUMINATION_DAWN_END = "DawnEnd";
                    public const string FILE_WEATHER_AT_ILLUMINATION_DUSK_BEGIN = "DuskBegin";
                    public const string FILE_WEATHER_AT_ILLUMINATION_DUSK_END = "DuskEnd";
                    public const string FILE_WEATHER_AT_ILLUMINATION_BRIGHTNESS_DAY = "BrightnessDay";
                    public const string FILE_WEATHER_AT_ILLUMINATION_BRIGHTNESS_NIGHT = "BrightnessNight";
            public const string FILE_WEATHER_EL_WEATHER_SET = "WeatherSet";
                public const string FILE_WEATHER_EL_TIME_RANGE = "TimeRange";
                    public const string FILE_WEATHER_EL_TIME_RANGE_PROPS = "Props";
                        public const string FILE_WEATHER_AT_TIME_RANGE_BEGIN = "Begin";
                        public const string FILE_WEATHER_AT_TIME_RANGE_END = "End";
                        public const string FILE_WEATHER_AT_TIME_RANGE_SIGHT_DISTANCE = "SightDistance";
                        public const string FILE_WEATHER_AT_TIME_RANGE_BRIGHTNESS_FACTOR = "BrightnessFactor";
                    public const string File_WEATHER_EL_TIME_RANGE_VARIABLE = "Variable";
                        public const string File_WEATHER_AT_TIME_RANGE_VARIABLE_NAME = "Name";
                    public const string FILE_WEATHER_EL_TIME_RANGE_SKYBOX = "Skybox";
                        public const string FILE_WEATHER_EL_TIME_RANGE_SKYBOX_TEXTURES = "Props";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH = "TextureNorth";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST = "TextureEast";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH = "TextureSouth";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST = "TextureWest";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE = "TextureAbove";

                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH_TILE_U = "TextureNorthTileU";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH_TILE_V = "TextureNorthTileV";        
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST_TILE_U = "TextureEastTileU";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST_TILE_V = "TextureEastTileV";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH_TILE_U = "TextureSouthTileU";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH_TILE_V = "TextureSouthTileV";        
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST_TILE_U = "TextureWestTileU";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST_TILE_V = "TextureWestTileV";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE_TILE_U = "TextureAboveTileU";
                            public const string FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE_TILE_V = "TextureAboveTileV";
                    public const string FILE_WEATHER_EL_TIME_RANGE_PRECIPITATION = "Precipitation";
                        public const string FILE_WEATHER_EL_TIME_RANGE_PRECIPITATION_TYPE = "Type";    
        

        #endregion

    }
}
