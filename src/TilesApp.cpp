#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "TileRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TilesApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
    void touchesMoved( TouchEvent event ) override;

	void update() override;
	void draw() override;

    ivec2 mLastMouse;

    TileRenderer mTiles = TileRenderer( 15, 15, vec2( 50 ) );
};

void TilesApp::setup()
{
}

void TilesApp::mouseDown( MouseEvent event )
{
    mLastMouse = event.getPos();
}

void TilesApp::mouseDrag( MouseEvent event )
{
    mTiles.move( event.getPos() - mLastMouse );
    mLastMouse = event.getPos();
}

void TilesApp::touchesMoved( TouchEvent event )
{
    vec2 mDelta1, mDelta2;

    const vector<TouchEvent::Touch>&touches = event.getTouches();
    if (touches.size() == 2) {
        mDelta1 = touches[0].getPos() - touches[0].getPrevPos();
        mDelta2 = touches[1].getPos() - touches[1].getPrevPos();

        mTiles.move( mDelta1 + mDelta2 / vec2( 2 ) );
    }
}

void TilesApp::update()
{

}

void TilesApp::draw()
{
	gl::clear( Color::white() );
    gl::ScopedMatrices matrixScope;


//    gl::translate( getWindowCenter() );
    gl::translate( mTiles.tileSize() );


    mTiles.draw();
}

void prepareSettings( App::Settings *settings )
{
    settings->setMultiTouchEnabled( true );
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize( 800, 800 );
}

CINDER_APP( TilesApp, RendererGl, prepareSettings )
