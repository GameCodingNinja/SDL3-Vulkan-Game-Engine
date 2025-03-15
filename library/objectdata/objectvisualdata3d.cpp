
/************************************************************************
 *    FILE NAME:       ojectvisualdata3d.cpp
 *
 *    DESCRIPTION:     Class containing the 3D object's visual data
 ************************************************************************/

#if !(defined(__IOS__) || defined(__ANDROID__) || defined(__arm__))
// Glew dependencies (have to be defined first)
#include <GL/glew.h>
#endif

// Physical component dependency
#include <objectdata/objectvisualdata3d.h>

// Game lib dependencies
#include <system/device.h>
#include <utilities/xmlparsehelper.h>
#include <utilities/xmlParser.h>
#include <utilities/exceptionhandling.h>

// Boost lib dependencies
#include <boost/format.hpp>

/************************************************************************
 *    DESC:  Constructor
 ************************************************************************/
CObjectVisualData3D::CObjectVisualData3D()
{
}

/************************************************************************
 *    DESC:  Destructor                                                             
 ************************************************************************/
CObjectVisualData3D::~CObjectVisualData3D()
{
}

/************************************************************************
 *    DESC:  Load the object data from node
 ************************************************************************/
void CObjectVisualData3D::loadFromNode( const XMLNode & objectNode )
{
    const XMLNode visualNode = objectNode.getChildNode( "visual" );
    if( !visualNode.isEmpty() )
    {
        // Get the mesh file
        if (visualNode.isAttributeSet("file"))
            m_modelFile = visualNode.getAttribute("file");

        // Get the color
        m_color = NParseHelper::LoadColor( visualNode, m_color );

        // Get the pipeline id
        const XMLNode pipelineNode = visualNode.getChildNode( "pipeline" );
        if( !pipelineNode.isEmpty() )
            m_pipelineID = pipelineNode.getAttribute( "id" );

        // Raise an exception if there's a mesh but no shader id
        if( !m_modelFile.empty() && m_pipelineID.empty() )
        {
            throw NExcept::CCriticalException("Pipeline not set!",
                boost::str( boost::format("Pipeline id missing.\n\n%s\nLine: %s")
                    % __FUNCTION__ % __LINE__ ));
        }
    }
}


/************************************************************************
 *    DESC:  Create the object from data
 ************************************************************************/
void CObjectVisualData3D::createFromData( const std::string & group )
{
    if( !m_modelFile.empty() )
        CDevice::Instance().createModel( group, m_modelFile, m_model );
}


/************************************************************************
 *    DESC:  Get the pipeline ID
 ************************************************************************/
const std::string & CObjectVisualData3D::getPipelineID() const
{
    return m_pipelineID;
}


/************************************************************************
 *    DESC:  Get the color
 ************************************************************************/
const CColor & CObjectVisualData3D::getColor() const
{
    return m_color;
}


/************************************************************************
 *    DESC:  Whether or not the visual tag was specified
 ************************************************************************/
bool CObjectVisualData3D::isActive() const
{
    return !m_modelFile.empty();
}


/************************************************************************
 *    DESC:  Get the mesh3d vector
 ************************************************************************/
const CModel & CObjectVisualData3D::getModel() const
{
    return m_model;
}
