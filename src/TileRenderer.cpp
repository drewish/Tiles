//
//  TileRenderer.cpp
//  Tiles
//
//  Created by andrew morton on 10/15/16.
//
//

#include "TileRenderer.h"

using namespace ci;
//
//    class Tile {
//      public:
//        static TileRef create( const ivec2 &position ) {
//            return TileRef( new Tile( position ) );
//        };
//        static TileRef create( const TileRef neighbor, const ivec2 &offset ) {
//            return TileRef( new Tile( neighbor->position() + offset ) );
//        };
//
//        Tile( const ivec2 &position ) : mPosition( position )
//        {
//            Rand rando = Rand( ( mPosition.x << 8 ) + mPosition.y );
//            mValue = rando.nextUint( 16 );
//
//            float p = getPerlin().fBm( vec2( position ) * vec2( 0.1, 0.1 ) );
//            mColor = Color( ci::CM_HSV, 0.5 + p, 1.0, 1.0 );
//        }
//
//        const ivec2&    position() const { return mPosition; }
//        u_int8_t        value() const { return mValue; }
//        const ColorA&   color() const { return mColor; }
//
//      protected:
//        ivec2 mPosition;
//        u_int8_t mValue;
//        ColorA mColor;
//    };


TileRenderer::TileRenderer( u_int8_t width, uint8_t height, ivec2 tileSize )
: mXCount( width ), mYCount( height ), mOffset( 0 ), mTileSize( tileSize ), mPerlin( 8 )
{
    for ( u_int8_t y = 0; y < mYCount; ++y ) {
        Row row;
        for ( u_int8_t x = 0; x < mXCount; ++x ) {
            ivec2 position = ivec2( x, y );
            row.push_back( Tile::create( position, valueFor( position ) ) );
        }
        mBoard.push_back( row );
    }
}

float TileRenderer::valueFor( const ivec2 &position )
{
    return mPerlin.fBm( ci::vec2( position ) * ci::vec2( 0.1, 0.1 ) );
}

void TileRenderer::move( ivec2 adjustment ) {
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

void TileRenderer::moveUp() {
    TileRef prev = mBoard.front().front();
    Row row;
    for ( u_int8_t x = 0; x < mXCount; ++x ) {
        ivec2 pos = prev->position() + ivec2( x, -1 );
        row.push_back( Tile::create( pos, valueFor( pos ) ) );
    }
    mBoard.pop_back();
    mBoard.push_front( row );
}

void TileRenderer::moveDown() {
    TileRef prev = mBoard.back().front();
    Row row;
    for ( u_int8_t x = 0; x < mXCount; ++x ) {
        ivec2 pos= prev->position() + ivec2( x, +1 );
        row.push_back( Tile::create( pos, valueFor( pos ) ) );
    }
    mBoard.pop_front();
    mBoard.push_back( row );
}

void TileRenderer::moveRight() {
    for ( auto &row : mBoard ) {
        row.pop_back();
        ivec2 pos = row.front()->position() + ivec2( -1, 0 );
        row.push_front( Tile::create( pos, valueFor( pos ) ) );
    }
}

void TileRenderer::moveLeft() {
    for ( auto &row : mBoard ) {
        row.pop_front();
        ivec2 pos = row.back()->position() + ivec2( +1, 0 );
        row.push_back( Tile::create( pos, valueFor( pos ) ) );
    }
}

void TileRenderer::draw() {
    vec2 scaling = vec2( mTileSize ) / vec2( 2 );

    u_int8_t y = 0;
    for ( const auto& row : mBoard ) {
        u_int8_t x = 0;
        for ( const auto& tile : row ) {
            vec2 at = ( ivec2( x, y ) * mTileSize ) + mOffset;

            gl::color( tile->color() );
            gl::drawSolidRect( Rectf( at + vec2( -1, -1 ) * scaling, at + vec2( 1, 1 ) * scaling ) );

//            gl::drawString(
//                std::to_string(tile->position().x), at + vec2(0, -0.25) * scaling,
//                Color( 1, 0, 0 ),
//                Font( "Arial", 12 )
//            );
//            gl::drawString(
//                std::to_string(tile->position().y), at + vec2(0, +0.25) * scaling,
//                Color( 0, 0, 1 ),
//                Font( "Arial", 12 )
//            );
//
//            gl::color( 0, 0, 0 );
//            // if ( ( tile->value() & 1 ) == 1 ) gl::drawLine( at, at + vec2( 0, -1) * scaling);
//            if ( ( tile->value() & 2 ) == 2 ) gl::drawLine( at, at + vec2( 2, 0) * scaling);
//            if ( ( tile->value() & 4 ) == 4 ) gl::drawLine( at, at + vec2( 0, 2) * scaling);
//            // if ( ( tile->value() & 8 ) == 8 ) gl::drawLine( at, at + vec2( -1, 0) * scaling);

            x++;
        }
        y++;
    }
}
