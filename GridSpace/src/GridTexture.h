//
//  GridTexture.h
//
//  Created by Soso Limited on 5/20/15.
//
//

#pragma once

namespace soso {

class GridTexture;
using GridTextureRef = std::shared_ptr<GridTexture>;

///
/// A 2d Texture grid.
/// Useful when we don't have access to 3d Textures (OpenGL ES2)
///
class GridTexture
{
public:
	GridTexture( int iWidth, int iHeight, int iSubdivisions );

	/// Add a surface to the grid. The surface should be cellSize.
	void update( const ci::Surface &iSurface );
	/// Returns the underlying texture.
	ci::gl::TextureRef getTexture() const { return texture; }
	/// Returns the index of the most recent texture in the grid.
	int		getCurrentIndex() const { return index; }
	ci::ivec2 getIndexOffset( const ci::ivec2 &iCellDimensions, int iIndex ) const;
	/// Returns the pixel dimensions of a single cell.
	const ci::ivec2& getCellDimensions() const { return cellDimensions; }
private:
	ci::gl::TextureRef	texture;
	int									index = 0;
	int									cells = 1;
	int									columns = 1;
	ci::ivec2						cellDimensions;
};

} // namespace soso
