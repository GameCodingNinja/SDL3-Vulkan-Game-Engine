
/************************************************************************
*    FILE NAME:       settings.cpp
*
*    DESCRIPTION:     game settings class
************************************************************************/

// Physical component dependency
#include <utilities/settings.h>

// Game lib dependencies
#include <common/worldvalue.h>

// Boost lib dependencies
#include <boost/lexical_cast.hpp>

// Standard lib dependencies
#include <cstring>

// SDL lib dependencies
#include <SDL3/SDL_mixer.h>
#include <SDL3/SDL.h>

/************************************************************************
*    DESC:  Constructor
************************************************************************/
CSettings::CSettings() :
    m_filePath("data/settings/settings.cfg"),
    m_debugMode(true),
    m_autoplay(false),
    m_debugAsMobile(false),
    m_mobileDevice(false),
    m_size(1280,768),
    m_default_size(1280,768),
    m_orientation(EOrientation::LANDSCAPE),
    m_projectionScale(1),
    m_fullScreen(false),
    m_vSync(false),
    m_major(1),
    m_minor(0),
    m_validationLayers(false),
    m_viewAngle(45.f),
    m_minZdist(5.f),
    m_maxZdist(1000.f),
    m_gamepadEnabled(false),
    m_gamepadStickDeadZone(2500),
    m_frequency(44100),
    m_sound_channels(2), // MIX_DEFAULT_CHANNELS
    m_mix_channels(8), // MIX_CHANNELS
    m_chunksize(1024),
    m_activateDepthBuffer(false),
    m_activateStencilBuffer(false),
    m_minThreadCount(2),
    m_maxThreadCount(2),
    m_sectorSize(512),
    m_sectorSizeHalf(256),
    m_anisotropicLevel(ETextFilter::ANISOTROPIC_0X),
    m_projectionType(EProjectionType::PERSPECTIVE),
    m_debugStrVisible(false),
    m_tripleBuffering(false),
    m_saveByteCode(false),
    m_loadByteCode(false),
    m_stripDebugInfo(false)
{
    CWorldValue::setSectorSize( 512 );
    
    #if defined(__IOS__) || defined(__ANDROID__)
    m_mobileDevice = true;
    #endif
}

/************************************************************************
*    DESC:  destructor
************************************************************************/
CSettings::~CSettings()
{
}

/************************************************************************
*    DESC:  Init the file path for loading the settings file
************************************************************************/
void CSettings::initFilePath( const std::string & filePath )
{
    m_filePath = filePath;
}

/************************************************************************
*    DESC:  Load settings data from xml file
*
*    param: string & _filePath - path to file
*
*    ret: string - class name
************************************************************************/
void CSettings::loadXML()
{
    // Open and parse the XML file:
    m_mainNode = XMLNode::openFileHelper( m_filePath.c_str(), "settings" );

    if( !m_mainNode.isEmpty() )
    {
        const XMLNode debugNode = m_mainNode.getChildNode("debug");
        if( !debugNode.isEmpty() )
        {
            // Do we enable debug mode
            if( debugNode.isAttributeSet("debugMode") )
                m_debugMode = ( std::strcmp( debugNode.getAttribute("debugMode"), "true" ) == 0 );
            
            // Do we enable autoplay
            if( debugNode.isAttributeSet("autoPlay") )
                m_autoplay = ( std::strcmp( debugNode.getAttribute("autoPlay"), "true" ) == 0 );
            
            // Do we enable mobile debug mode
            if( debugNode.isAttributeSet("debugAsMobile") )
                m_debugAsMobile = ( m_debugMode && std::strcmp( debugNode.getAttribute("debugAsMobile"), "true" ) == 0 );
            
            // Pretend this is a mobile device if we are to debug as one on the PC
            if( m_debugAsMobile )
                m_mobileDevice = true;
        }

        const XMLNode infoNode = m_mainNode.getChildNode("info");
        if( !infoNode.isEmpty() )
        {
            if( infoNode.isAttributeSet("gameName") )
                m_gameName = infoNode.getAttribute("gameName");
            
            if( infoNode.isAttributeSet("engineName") )
                m_engineName = infoNode.getAttribute("engineName");
            
            if( infoNode.isAttributeSet("gameVersion") )
                m_gameVersion = std::atoi(infoNode.getAttribute("gameVersion"));
            
            if( infoNode.isAttributeSet("engineVersion") )
                m_engineVersion = std::atoi(infoNode.getAttribute("engineVersion"));
        }

        const XMLNode displayListNode = m_mainNode.getChildNode("display");
        if( !displayListNode.isEmpty() )
        {
            if (m_mobileDevice)
            {
                SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
                SDL_DisplayMode dm = *SDL_GetCurrentDisplayMode(displayID);
                m_size.w = dm.w;
                m_size.h = dm.h;
            }
            else
            {
                // Get the attributes from the "resolution" node
                const XMLNode resolutionNode = displayListNode.getChildNode("resolution");
                if( !resolutionNode.isEmpty() )
                {
                    m_size.w = std::atoi(resolutionNode.getAttribute("width"));
                    m_size.h = std::atoi(resolutionNode.getAttribute("height"));

                    m_fullScreen = ( std::strcmp( resolutionNode.getAttribute("fullscreen"), "true" ) == 0 );
                }
            }

            // Get the attributes from the "defaultHeight" node
            const XMLNode defResNode = displayListNode.getChildNode("default");

            m_native_size.h = m_default_size.h = std::atof(defResNode.getAttribute("height"));
            if( defResNode.isAttributeSet("width") )
                m_native_size.w = m_default_size.w = std::atof(defResNode.getAttribute("width"));

            if( defResNode.isAttributeSet("orientation") )
                if( std::strcmp( defResNode.getAttribute("orientation"), "portrait" ) == 0 )
                    m_orientation = EOrientation::PORTRAIT;
        }

        calcRatio();

        const XMLNode deviceNode = m_mainNode.getChildNode("device");
        if( !deviceNode.isEmpty() )
        {
            // Get the attribute for OpenGL node
            const XMLNode vulkanNode = deviceNode.getChildNode("Vulkan");
            if( !vulkanNode.isEmpty() )
            {
                if( vulkanNode.isAttributeSet("major") )
                    m_major = std::atoi( vulkanNode.getAttribute("major") );
                
                if( vulkanNode.isAttributeSet("minor") )
                    m_minor = std::atoi( vulkanNode.getAttribute("minor") );
                
                // Android validation layers need to be loaded on the device
                //#if !defined(__ANDROID__)
                if( vulkanNode.isAttributeSet("validationLayers") )
                    m_validationLayers = ( std::strcmp( vulkanNode.getAttribute("validationLayers"), "true" ) == 0 );
                //#endif
            }

            // Get the projection info
            const XMLNode projNode = deviceNode.getChildNode("projection");
            if( !projNode.isEmpty() )
            {
                if( projNode.isAttributeSet("minZDist") )
                    m_minZdist = std::atof(projNode.getAttribute("minZDist"));

                if( projNode.isAttributeSet("maxZDist") )
                    m_maxZdist = std::atof(projNode.getAttribute("maxZDist"));

                if( projNode.isAttributeSet("view_angle") )
                    m_viewAngle = std::atof(projNode.getAttribute("view_angle"));

                if( projNode.isAttributeSet("projectType") &&
                    std::strcmp( projNode.getAttribute("projectType"), "orthographic" ) == 0 )
                    m_projectionType = EProjectionType::ORTHOGRAPHIC;

                if( projNode.isAttributeSet("scale") )
                    m_projectionScale = std::atof(projNode.getAttribute("scale"));
            }

            // Convert to radians
            m_viewAngle *= (float)defs_DEG_TO_RAD;

            // Get the attribute from the "textureFiltering" node
            const XMLNode textFilterNode = deviceNode.getChildNode("anisotropicFiltering");
            if( !textFilterNode.isEmpty() )
            {
                const char * pAttr = textFilterNode.getAttribute("level");

                if( std::strcmp( pAttr, "anisotropic_0X" ) == 0 )
                    m_anisotropicLevel = ETextFilter::ANISOTROPIC_0X;

                else if( std::strcmp( pAttr, "anisotropic_2X" ) == 0 )
                    m_anisotropicLevel = ETextFilter::ANISOTROPIC_2X;

                else if( std::strcmp( pAttr, "anisotropic_4X" ) == 0 )
                    m_anisotropicLevel = ETextFilter::ANISOTROPIC_4X;

                else if( std::strcmp( pAttr, "anisotropic_8X" ) == 0 )
                    m_anisotropicLevel = ETextFilter::ANISOTROPIC_8X;

                else if( std::strcmp( pAttr, "anisotropic_16X" ) == 0 )
                    m_anisotropicLevel = ETextFilter::ANISOTROPIC_16X;
            }

            // Get the attribute from the "backbuffer" node
            const XMLNode backBufferNode = deviceNode.getChildNode("backbuffer");
            if( !backBufferNode.isEmpty() )
            {
                if( m_mobileDevice )
                {
                    m_vSync = true;
                }
                else
                {
                    m_tripleBuffering = ( std::strcmp( backBufferNode.getAttribute("tripleBuffering"), "true" ) == 0 );
                    m_vSync = ( std::strcmp( backBufferNode.getAttribute("VSync"), "true" ) == 0 );
                }                
            }

            const XMLNode joypadNode = deviceNode.getChildNode("joypad");
            if( !joypadNode.isEmpty() )
            {
                m_gamepadEnabled = true;
                m_gamepadStickDeadZone = std::atoi(joypadNode.getAttribute("stickDeadZone"));
            }

            const XMLNode threadNode = deviceNode.getChildNode("threads");
            if( !threadNode.isEmpty() )
            {
                if( threadNode.isAttributeSet("minThreadCount") )
                    m_minThreadCount = std::atoi(threadNode.getAttribute("minThreadCount"));

                if( threadNode.isAttributeSet("maxThreadCount") )
                    m_maxThreadCount = std::atoi(threadNode.getAttribute("maxThreadCount"));
            }


            // Get the attribute from the "depthStencilBuffer" node
            const XMLNode depthStencilBufferNode = deviceNode.getChildNode("depthStencilBuffer");
            if( !depthStencilBufferNode.isEmpty() )
            {
                // Do we enable the depth buffer
                if( depthStencilBufferNode.isAttributeSet("activateDepthBuffer") )
                    m_activateDepthBuffer = ( std::strcmp( depthStencilBufferNode.getAttribute("activateDepthBuffer"), "true" ) == 0 );

                // Do we enable the stencil buffer
                if( depthStencilBufferNode.isAttributeSet("activateStencilBuffer") )
                    m_activateStencilBuffer = ( std::strcmp( depthStencilBufferNode.getAttribute("activateStencilBuffer"), "true" ) == 0 );
            }
            
            // Get the scripting settings
            const XMLNode scriptNode = m_mainNode.getChildNode("scripting");
            if( !scriptNode.isEmpty() )
            {
                if( scriptNode.isAttributeSet("scriptListTable") )
                    m_scriptListTable = scriptNode.getAttribute("scriptListTable");
                
                if( scriptNode.isAttributeSet("group") )
                    m_scriptGroup = scriptNode.getAttribute("group");
                
                if( scriptNode.isAttributeSet("mainFunction") )
                    m_scriptMain = scriptNode.getAttribute("mainFunction");

                if( scriptNode.isAttributeSet("saveByteCode") )
                    m_saveByteCode = ( std::strcmp( scriptNode.getAttribute("saveByteCode"), "true" ) == 0 );

                if( scriptNode.isAttributeSet("loadByteCode") )
                    m_loadByteCode = ( std::strcmp( scriptNode.getAttribute("loadByteCode"), "true" ) == 0 );

                if( scriptNode.isAttributeSet("stripDebugInfo") )
                    m_stripDebugInfo = ( std::strcmp( scriptNode.getAttribute("stripDebugInfo"), "true" ) == 0 );
            }

            // Get the sound settings
            const XMLNode soundNode = m_mainNode.getChildNode("sound");
            if( !soundNode.isEmpty() )
            {
                if( soundNode.isAttributeSet("frequency") )
                    m_frequency = std::atoi(soundNode.getAttribute("frequency"));

                if( soundNode.isAttributeSet("sound_channels") )
                    m_sound_channels = std::atoi(soundNode.getAttribute("sound_channels"));

                if( soundNode.isAttributeSet("mix_channels") )
                    m_mix_channels = std::atoi(soundNode.getAttribute("mix_channels"));

                if( soundNode.isAttributeSet("chunksize") )
                    m_chunksize = std::atoi(soundNode.getAttribute("chunksize"));
            }

            // Get world settings
            const XMLNode worldNode = m_mainNode.getChildNode("world");
            if( !worldNode.isEmpty() )
            {
                if( worldNode.isAttributeSet("sectorSize") )
                {
                    m_sectorSize = std::atof(worldNode.getAttribute("sectorSize"));
                    m_sectorSizeHalf = m_sectorSize / 2;
                    CWorldValue::setSectorSize( m_sectorSize );
                }
            }
        }
    }
}

/************************************************************************
*    DESC:  Get debug info
************************************************************************/
bool CSettings::isDebugMode() const
{
    return m_debugMode;
}

bool CSettings::isAutoplay() const
{
    return m_autoplay;
}

bool CSettings::isDebugAsMobile() const
{
    return m_debugAsMobile;
}

bool CSettings::isMobileDevice() const
{
    return m_mobileDevice;
}

/************************************************************************
*    DESC:  Get game info
************************************************************************/
const std::string & CSettings::getGameName() const
{
    return m_gameName;
}

const std::string & CSettings::getEngineName() const
{
    return m_engineName;
}

uint32_t CSettings::getGameVersion() const
{
    return m_gameVersion;
}

uint32_t CSettings::getEngineVersion() const
{
    return m_engineVersion;
}

/************************************************************************
*    DESC:  Calculate the ratios
************************************************************************/
void CSettings::calcRatio()
{
    // Height and width screen ratio for perspective projection
    m_screenAspectRatio.w = m_size.w / m_size.h;
    m_screenAspectRatio.h = m_size.h / m_size.w;

    if( m_orientation == EOrientation::PORTRAIT )
    {
        // NOTE: The default height is based on the current aspect ratio
        // NOTE: Make sure the height does not have a floating point component
        m_default_size.h = (float)(int)std::ceil((m_screenAspectRatio.h * m_default_size.w) + 0.5);
    }
    else
    {
        // NOTE: The default width is based on the current aspect ratio
        // NOTE: Make sure the width does not have a floating point component
        m_default_size.w = (float)(int)std::ceil((m_screenAspectRatio.w * m_default_size.h) + 0.5);
    }

    // Get half the size for use with screen boundaries
    m_default_size_half = m_default_size / 2.f;

    // Screen size divided by two
    m_size_half = m_size / 2.f;

    // Pre-calculate the aspect ratios for orthographic projection
    m_orthoAspectRatio.h = m_size.h / m_default_size.h;
    m_orthoAspectRatio.w = m_size.w / m_default_size.w;
}

/************************************************************************
*    DESC:  Get/Set game window size
************************************************************************/
const CSize<float> & CSettings::getSize() const
{
    return m_size;
}

void CSettings::setSize( const CSize<float> & size )
{
    m_size = size;
}

/************************************************************************
*    DESC:  Get game window size / 2
*
*    ret: CSize - size of game window / 2
************************************************************************/
const CSize<float> & CSettings::getSizeHalf() const
{
    return m_size_half;
}

/************************************************************************
*    DESC:  Get native size specified in config file
************************************************************************/
const CSize<float> & CSettings::getNativeSize() const
{
    return m_native_size;
}

/************************************************************************
*    DESC:  Get native size specified in config file and calc width
************************************************************************/
const CSize<float> & CSettings::getDefaultSize() const
{
    return m_default_size;
}

/************************************************************************
*    DESC:  Get default size in half
************************************************************************/
const CSize<float> & CSettings::getDefaultSizeHalf() const
{
    return m_default_size_half;
}

/************************************************************************
*    DESC:  Height and width screen ratio for orthographic objects
*           The difference between screen and the default size
************************************************************************/
const CSize<float> & CSettings::getOrthoAspectRatio() const
{
    return m_orthoAspectRatio;
}

float CSettings::getOrthoAspectRatioOrientation() const
{
    if( m_orientation == EOrientation::PORTRAIT )
        return m_orthoAspectRatio.w;
    else
        return m_orthoAspectRatio.h;
}

/************************************************************************
*    DESC: Height and width screen ratio for perspective projection
************************************************************************/
const CSize<float> & CSettings::getScreenAspectRatio() const
{
    return m_screenAspectRatio;
}

/************************************************************************
*    DESC:  Get/Set vSync?
************************************************************************/
bool CSettings::getVSync() const
{
    return m_vSync;
}

void CSettings::setVSync( bool value )
{
    m_vSync = value;
}

/************************************************************************
*    DESC:  Get the Vulkan major version
************************************************************************/
int CSettings::getMajorVersion() const
{
    return m_major;
}

/************************************************************************
*    DESC:  Get the Vulkan minor version
************************************************************************/
int CSettings::getMinorVersion() const
{
    return m_minor;
}

/************************************************************************
*    DESC:  Do we want validation layers
************************************************************************/
bool CSettings::isValidationLayers() const
{
    return m_validationLayers;
}

/************************************************************************
*    DESC:  Get the view angle
************************************************************************/
float CSettings::getViewAngle() const
{
    return m_viewAngle;
}

/************************************************************************
*    DESC:  Get the minimum z distance
************************************************************************/
float CSettings::getMinZdist() const
{
    return m_minZdist;
}

/************************************************************************
*    DESC:  Get the maximum z distance
************************************************************************/
float CSettings::getMaxZdist() const
{
    return m_maxZdist;
}

/************************************************************************
*    DESC:  Is the gamepad enabled
************************************************************************/
bool CSettings::isGamePadEnabled() const
{
    return m_gamepadEnabled;
}

/************************************************************************
*    DESC:  Get the gamepad stick dead zone
************************************************************************/
int CSettings::getGamePadStickDeadZone() const
{
    return m_gamepadStickDeadZone;
}

/************************************************************************
*    DESC:  Set the gamepad stick dead zone
************************************************************************/
void CSettings::setGamePadStickDeadZone( int value )
{
    m_gamepadStickDeadZone = value;
}

/************************************************************************
*    DESC:  Get/Set full screen
************************************************************************/
bool CSettings::getFullScreen() const
{
    return m_fullScreen;
}

void CSettings::setFullScreen( bool value )
{
    m_fullScreen = value;
}

/************************************************************************
*    DESC:  Is the depth buffer activated by default
************************************************************************/
bool CSettings::activateDepthBuffer() const
{
    return m_activateDepthBuffer;
}

/************************************************************************
*    DESC:  Is the stencil buffer activated by default
************************************************************************/
bool CSettings::activateStencilBuffer() const
{
    return m_activateStencilBuffer;
}

/************************************************************************
*    DESC:  Get the script members
************************************************************************/
const std::string & CSettings::getScriptListTable() const
{
    return m_scriptListTable;
}

const std::string & CSettings::getScriptGroup() const
{
    return m_scriptGroup;
}

const std::string & CSettings::getScriptMain() const
{
    return m_scriptMain;
}

bool CSettings::getSaveByteCode() const
{
    return m_saveByteCode;
}

bool CSettings::getLoadByteCode() const
{
    return m_loadByteCode;
}

bool CSettings::getStripDebugInfo() const
{
    return m_stripDebugInfo;
}

/************************************************************************
*    DESC:  Get the sound frequency
************************************************************************/
int CSettings::getFrequency() const
{
    return m_frequency;
}

/************************************************************************
*    DESC:  Get the sound channels: mono, stero, quad, suround, etc
************************************************************************/
int CSettings::getSoundChannels() const
{
    return m_sound_channels;
}

/************************************************************************
*    DESC:  Get the number of channels used for mixing
************************************************************************/
int CSettings::getMixChannels() const
{
    return m_mix_channels;
}

/************************************************************************
*    DESC:  Get the chunk size. The amount of memory used for mixing.
*           The higher the number, the more latency in sound responce
*           stopping and starting
************************************************************************/
int CSettings::getChunkSize() const
{
    return m_chunksize;
}

/************************************************************************
*    DESC:  Get the minimum thread count
************************************************************************/
int CSettings::getMinThreadCount() const
{
    return m_minThreadCount;
}

/************************************************************************
*    DESC:  Get the maximum thread count
*           Value of zero means use max hardware threads to no of cores
************************************************************************/
int CSettings::getMaxThreadCount() const
{
    return m_maxThreadCount;
}


/************************************************************************
*    DESC:  Get/Set the Anisotropic setting
************************************************************************/
int CSettings::getAnisotropicLevel() const
{
    return static_cast<int>(m_anisotropicLevel);
}

void CSettings::setAnisotropicLevel( int level )
{
    m_anisotropicLevel = static_cast<ETextFilter>(level);
}

/************************************************************************
*    DESC:  Get the sector size
************************************************************************/
int CSettings::getSectorSize() const
{
    return m_sectorSize;
}

/************************************************************************
*    DESC:  Get half of the sector size
************************************************************************/
int CSettings::getSectorSizeHalf() const
{
    return m_sectorSizeHalf;
}

/************************************************************************
*    DESC:  Get the projection type
************************************************************************/
EProjectionType CSettings::getProjectionType() const
{
    return m_projectionType;
}

/************************************************************************
*    DESC:  Get the projection scale
************************************************************************/
float CSettings::getProjectionScale() const
{
    return m_projectionScale;
}

/************************************************************************
*    DESC:  Set/Get debug string visible
************************************************************************/
void CSettings::setDebugStrVisible( bool value )
{
    m_debugStrVisible = value;
}

bool CSettings::getDebugStrVisible() const
{
    return m_debugStrVisible;
}

/************************************************************************
*    desc:  Do we want triple buffering?
************************************************************************/
bool CSettings::getTripleBuffering() const
{
    return m_tripleBuffering;
}

/************************************************************************
*    DESC:  Save the settings file
************************************************************************/
void CSettings::saveSettings()
{
    if( !m_mainNode.isEmpty() )
    {
        XMLNode displayListNode = m_mainNode.getChildNode("display");

        if( !displayListNode.isEmpty() )
        {
            // Get the attributes from the "resolution" node
            XMLNode resolutionNode = displayListNode.getChildNode("resolution");

            {
                std::string tmpStr = boost::lexical_cast<std::string>( m_size.w );
                resolutionNode.updateAttribute(tmpStr.c_str(), "width", "width");

                tmpStr = boost::lexical_cast<std::string>( m_size.h );
                resolutionNode.updateAttribute(tmpStr.c_str(), "height", "height");
            }

            {
                std::string tmpStr = "false";

                if( m_fullScreen )
                    tmpStr = "true";

                resolutionNode.updateAttribute(tmpStr.c_str(), "fullscreen", "fullscreen");
            }
        }

        XMLNode deviceNode = m_mainNode.getChildNode("device");

        if( !deviceNode.isEmpty() )
        {
            {
                // Get the attribute from the "backbuffer" node
                XMLNode backBufferNode = deviceNode.getChildNode("backbuffer");

                std::string tmpStr = "false";

                if( m_vSync )
                    tmpStr = "true";

                backBufferNode.updateAttribute(tmpStr.c_str(), "VSync", "VSync");
            }
            /*{
                // Get the attribute from the "backbuffer" node
                XMLNode textFilterNode = deviceNode.getChildNode("textureFiltering");

                std::string tmpStr = GetTextFilterString();

                textFilterNode.updateAttribute(tmpStr.c_str(), "filter", "filter");
            }*/
        }

        {
            // Get the attribute from the "joypad" node
            XMLNode joypadNode = deviceNode.getChildNode("joypad");

            std::string tmpStr = boost::lexical_cast<std::string>( m_gamepadStickDeadZone );

            joypadNode.updateAttribute(tmpStr.c_str(), "stickDeadZone", "stickDeadZone");
        }

        // Save the settings file
        m_mainNode.writeToFile(m_filePath.c_str(), "utf-8");
    }
}
