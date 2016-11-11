#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "TileRenderer.h"
#include "Vehicle.h"

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
    CameraPersp         mViewCamera;
    CameraUi            mViewCameraUI;
    TileRenderer mTiles = TileRenderer( 7, 7, vec2( 100 ) );
    Vehicle mMover;
};



ci::PolyLine2f polyLineCircle( float radius, u_int8_t subdivisions )
{
    ci::PolyLine2f result;
    const ci::vec2 center( 0 );
    // iterate the segments
    const float tDelta = 1 / (float) subdivisions * 2.0f * M_PI;
    float t = 0;
    for( int s = 0; s <= subdivisions; s++ ) {
        ci::vec2 unit( ci::math<float>::cos( t ), ci::math<float>::sin( t ) );
        result.push_back( center + unit * radius );
        t += tDelta;
    }
    result.setClosed();

    return result;
}

void TilesApp::setup()
{
    mViewCamera.setPerspective( 80.0f, getWindowAspectRatio(), 10.0f, 4000.0f );
    // eyePoint, target, up
//    mViewCamera.lookAt( vec3( 200, -200, 400 ), vec3( 0, 0, 0 ), vec3( 0, 0, 1 ) );
//    mViewCameraUI = CameraUi( &mViewCamera, getWindow() );
//    mViewCameraUI.setMouseWheelMultiplier( 1 );

    PolyLine2f circle = polyLineCircle( 300, 8 );
    circle.offset( vec2( 300 ) );
    mMover.setup( circle );
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
    mMover.update( 0 );
    mTiles.move( mMover.getLastPosition() - mMover.getPosition() );
    // eyePoint, target, up
//    mMover.getAngle()
//mMover.getPosition()
    mViewCamera.lookAt( vec3( 400, -200, 400 ), vec3( 400, 200, 1), vec3( 0, 0, 1 ) );
}

void TilesApp::draw()
{
    gl::ScopedMatrices matrixScope;
    gl::setMatrices( mViewCamera );

	gl::clear( Color::gray( 0.5 ) );
    gl::color( Color::white() );

    {
        gl::ScopedMatrices matrixScope2;
//        gl::translate(  );
//        gl::translate( mMover.getPosition()-mTiles.boardCenter() );
        mTiles.draw();
    }

    {
        gl::ScopedMatrices matrixScope3;
        mMover.draw();
    }
}

void prepareSettings( App::Settings *settings )
{
    settings->setMultiTouchEnabled( true );
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize( 800, 800 );
}

CINDER_APP( TilesApp, RendererGl, prepareSettings )
