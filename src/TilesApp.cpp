#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;


Perlin getPerlin() {
    static Perlin p = Perlin( 8 );
    return p;
}

class Tile;
typedef std::shared_ptr<Tile> TileRef;

class Tile {
  public:
    static TileRef create( const ivec2 &position ) {
        return TileRef( new Tile( position ) );
    };

    Tile( const ivec2 &position ) : mPosition( position )
    {
        Rand rando = Rand( ( mPosition.x << 8 ) + mPosition.y );
        mValue = rando.nextUint( 16 );

        float p = getPerlin().fBm( vec2( position ) * vec2( 0.1, 0.1 ) );
        mColor = Color( ci::CM_HSV, 0.5 + p, 1.0, 1.0 );
    }

    const ivec2&    position() const { return mPosition; }
    u_int8_t        value() const { return mValue; }
    const ColorA&   color() const { return mColor; }

  protected:
    ivec2 mPosition;
    u_int8_t mValue;
    ColorA mColor;
};


class TileMap {
  public:
    typedef std::list<TileRef> Row;
    typedef std::list<Row> Board;

    TileMap( u_int8_t width, uint8_t height, ivec2 tileSize = ivec2( 50 ) )
    : mXCount( width ), mYCount( height ), mOffset( 0 ), mTileSize( tileSize )
    {
        for ( u_int8_t y = 0; y < mYCount; ++y ) {
            Row row;
            for ( u_int8_t x = 0; x < mXCount; ++x ) {
                row.push_back( Tile::create( ivec2( x, y ) ) );
            }
            mBoard.push_back( row );
        }
    }

    ivec2 tileSize() const { return mTileSize; }

    void move( ivec2 adjustment ) {
        int quot;
        double remain;

        mOffset += adjustment;

        remain = remquo( mOffset.y, mTileSize.y, &quot );
        while ( quot != 0 ) {
            mOffset.y = remain;
            if ( quot > 0 ) {
                --quot;
                moveUp();
            } else {
                ++quot;
                moveDown();
            }
        }

        remain = remquo( mOffset.x, mTileSize.x, &quot );
        while ( quot != 0 ) {
            mOffset.x = remain;
            if ( quot > 0 ) {
                --quot;
                moveRight();
            }
            else {
                ++quot;
                moveLeft();
            }
        }
    }

    void moveUp() {
        TileRef prev = mBoard.front().front();
        Row row;
        for ( u_int8_t x = 0; x < mXCount; ++x ) {
            row.push_back( Tile::create( prev->position() + ivec2( x, -1 ) ) );
        }
        mBoard.pop_back();
        mBoard.push_front( row );
    }

    void moveDown() {
        TileRef prev = mBoard.back().front();
        Row row;
        for ( u_int8_t x = 0; x < mXCount; ++x ) {
            row.push_back( Tile::create( prev->position() + ivec2( x, +1 ) ) );
        }
        mBoard.pop_front();
        mBoard.push_back( row );
    }

    void moveRight() {
        for ( auto &row : mBoard ) {
            row.pop_back();
            TileRef prev = row.front();
            row.push_front( Tile::create( prev->position() + ivec2( -1, 0 ) ) );
        }
    }

    void moveLeft() {
        for ( auto &row : mBoard ) {
            row.pop_front();
            TileRef prev = row.back();
            row.push_back( Tile::create( prev->position() + ivec2( +1, 0 ) ) );
        }
    }

    void draw() {
        vec2 scaling = vec2( mTileSize ) / vec2( 2 );

        u_int8_t y = 0;
        for ( const auto& row : mBoard ) {
            u_int8_t x = 0;
            for ( const auto& tile : row ) {
                vec2 at = ( ivec2( x, y ) * mTileSize ) + mOffset;

                gl::color( tile->color() );
                gl::drawSolidRect( Rectf( at + vec2( -1, -1 ) * scaling, at + vec2( 1, 1 ) * scaling ) );

//                gl::drawString(
//                    std::to_string(tile->position().x), at + vec2(0, -0.25) * scaling,
//                    Color( 1, 0, 0 ),
//                    Font( "Arial", 12 )
//                );
//                gl::drawString(
//                    std::to_string(tile->position().y), at + vec2(0, +0.25) * scaling,
//                    Color( 0, 0, 1 ),
//                    Font( "Arial", 12 )
//                );

//                gl::color( 0, 0, 0 );
//                // if ( ( tile->value() & 1 ) == 1 ) gl::drawLine( at, at + vec2( 0, -1) * scaling);
//                if ( ( tile->value() & 2 ) == 2 ) gl::drawLine( at, at + vec2( 2, 0) * scaling);
//                if ( ( tile->value() & 4 ) == 4 ) gl::drawLine( at, at + vec2( 0, 2) * scaling);
//                // if ( ( tile->value() & 8 ) == 8 ) gl::drawLine( at, at + vec2( -1, 0) * scaling);

                x++;
            }
            y++;
        }
    }

  protected:
    const ivec2 mTileSize;
    const u_int8_t mXCount;
    const u_int8_t mYCount;

    ivec2 mOffset;
    Board mBoard;
};

class TilesApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
    void touchesMoved( TouchEvent event ) override;

	void update() override;
	void draw() override;

    ivec2 mLastMouse;

    TileMap mTiles = TileMap( 15, 15, vec2( 50 ) );
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
