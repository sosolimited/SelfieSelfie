//
//  GridTexture.h
//
//  Created by Soso Limited on 5/20/15.
//
//

#pragma once

#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"

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
  /// Create a grid texture containing iRows * iRows cells of iCellSize.
  GridTexture( const ci::ivec2 &iCellSize, int iRows );

	/// Add a surface to the grid. The surface should be cellSize.
	void addCameraImage( const ci::Capture &iCapture );
	/// Returns the underlying texture.
	ci::gl::TextureRef getTexture() const { return focusedBuffer->getColorTexture(); }
	/// Returns a blurred version of the underlying texture.
	ci::gl::TextureRef getBlurredTexture() const { return blurredBuffer->getColorTexture(); }

	/// Returns the index of the most recent texture in the grid.
	int       getCurrentIndex() const { return index; }
	ci::ivec2 getIndexOffset( const ci::ivec2 &iCellDimensions, int iIndex ) const;
	/// Returns the pixel dimensions of a single cell.
	const ci::ivec2& getCellDimensions() const { return cellDimensions; }

  ci::vec2 getGridDimensions() const { return ci::vec2( columns ); }
	/// Returns the pixel size of the entire grid.
	ci::vec2 getGridSize() const { return getGridDimensions() * ci::vec2( getCellDimensions() ); }
private:
	ci::gl::FboRef			blurredBuffer;
  ci::gl::FboRef			focusedBuffer;
  ci::gl::FboRef      intermediateBlurBuffer;
	ci::gl::GlslProgRef	blurProg;
  ci::gl::GlslProgRef passthroughProg;

  ci::gl::TextureRef  intermediateTexture;

	int									index = 0;
	int									cells = 1;
	int									columns = 1;
  ci::ivec2						cellDimensions = ci::ivec2( 320, 240 );
  ci::ivec2           blurredCellDimensions = cellDimensions / 4;

  void renderClearTexture();
  void renderBlurredTexture();

	void update();
};

} // namespace soso
