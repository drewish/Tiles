#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
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
    vec2 calcMouseOnPlane( vec2 mouse );

    void update() override;
    void draw() override;

    params::InterfaceGlRef	mParams;
    ivec2 mLastMouse;
    CameraPersp         mViewCamera;
    CameraUi            mViewCameraUI;
    vec2 mTileSize = vec2( 100 );
    ivec2 mBoardSize = ivec2( 7 );
    TileRenderer mTiles;
    Vehicle mMover;
    FlightPlan mPlan;
};

vec2 TilesApp::calcMouseOnPlane( vec2 mouse )
{
    float u = mouse.x / (float) getWindowWidth();
    float v = ( getWindowHeight() - mouse.y ) / (float) getWindowHeight();
    Ray ray = mViewCamera.generateRay( u, v, mViewCamera.getAspectRatio() );
    float distance = 0.0f;
    if ( ! ray.calcPlaneIntersection( glm::zero<ci::vec3>(), vec3( 0, 0, 1 ), &distance ) ) {
        return vec2( 0 );
    }
    vec3 intersection = ray.calcPosition( distance );
    return vec2( intersection.x, intersection.y );
}


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
    // Create the interface and give it a name.
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 400 ) ) );

    auto update = [this] { mTiles = TileRenderer( mBoardSize, mTileSize ); };
    update();

    // Setup some basic parameters.
    mParams->addParam( "Board Width", &mBoardSize.x ).min( 1 ).max( 20 ).step( 1 ).updateFn( update );
    mParams->addParam( "Board Height", &mBoardSize.y ).min( 1 ).max( 20 ).step( 1 ).updateFn( update );
    mParams->addParam( "Tile Width", &mTileSize.x ).min( 20 ).max( 200 ).step( 5 ).updateFn( update );
    mParams->addParam( "Tile Height", &mTileSize.y ).min( 20 ).max( 200 ).step( 5 ).updateFn( update );


    mViewCamera.setPerspective( 80.0f, getWindowAspectRatio(), 10.0f, 4000.0f );
    // eyePoint, target, up
    mViewCamera.lookAt( vec3( 200, -200, 400 ), vec3( 0, 0, 0 ), vec3( 0, 0, 1 ) );
    mViewCameraUI = CameraUi( &mViewCamera, getWindow() );
//    mViewCameraUI.setMouseWheelMultiplier( 1 );

    PolyLine2f circle = polyLineCircle( 300, 8 );
    circle.offset( vec2( 300 ) );

    mPlan = FlightPlan( &mMover, circle );

    mTiles.jumpTo( mMover.getPosition() );
}

void TilesApp::mouseDown( MouseEvent event )
{
    mLastMouse = event.getPos();
//    mTiles.jumpTo( calcMouseOnPlane( mLastMouse ) );
}

void TilesApp::mouseDrag( MouseEvent event )
{
//    mTiles.move( event.getPos() - mLastMouse );
//    mLastMouse = event.getPos();
}

void TilesApp::touchesMoved( TouchEvent event )
{
//    vec2 mDelta1, mDelta2;
//
//    const vector<TouchEvent::Touch>&touches = event.getTouches();
//    if (touches.size() == 2) {
//        mDelta1 = touches[0].getPos() - touches[0].getPrevPos();
//        mDelta2 = touches[1].getPos() - touches[1].getPrevPos();
//
//        mTiles.move( mDelta1 + mDelta2 / vec2( 2 ) );
//    }
}

void TilesApp::update()
{
    mPlan.update();
//    mMover.update( mPlan.computeSteeringForce( mMover.getPosition(), mMover.getVelocity() ) );
    mTiles.move( mMover.getPosition() );
}

void TilesApp::draw()
{
    gl::ScopedMatrices matrixScope;
    gl::setMatrices( mViewCamera );

	gl::clear( Color::gray( 0.5 ) );
    gl::color( Color::white() );

    mTiles.draw();
    {
        gl::ScopedModelMatrix inner;
        gl::translate( 0, 0, 100 );
        mPlan.draw();
        mMover.draw();
    }

    mParams->draw();
}

void prepareSettings( App::Settings *settings )
{
    settings->setMultiTouchEnabled( true );
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize( 800, 800 );
}

CINDER_APP( TilesApp, RendererGl, prepareSettings )
