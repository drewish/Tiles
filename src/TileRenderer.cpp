//
//  TileRenderer.cpp
//  Tiles
//
//  Created by andrew morton on 10/15/16.
//
//

#include "TileRenderer.h"

using namespace ci;

TileRenderer::TileRenderer( const uvec2 &boardSize, const uvec2 &tileSize )
    : mColumns( boardSize.x ), mRows( boardSize.y ), mCenteredOn( 0 ), mTileSize( tileSize ), mPerlin( 8 )
{
    assert( boardSize.x > 0 && boardSize.y > 0 );

    for ( u_int8_t y = 0; y < mRows; ++y ) {
        Row row;
        for ( u_int8_t x = 0; x < mColumns; ++x ) {
            ivec2 position = ivec2( x, y );
            row.push_back( Tile::create( position, valueFor( position ) ) );
        }
        mBoard.push_back( std::move( row ) );
    }
}

TileRenderer& TileRenderer::operator=( const TileRenderer &rhs )
{
    mColumns = rhs.mColumns;
    mRows = rhs.mRows;
    mTileSize = rhs.mTileSize;
    mPerlin = rhs.mPerlin;
    mCenteredOn = rhs.mCenteredOn;

    ivec2 cornerPos = mCenteredOn - centerOffset();

    mBoard.clear();
    for ( u_int8_t y = 0; y < mRows; ++y ) {
        Row row;
        for ( u_int8_t x = 0; x < mColumns; ++x ) {
            ivec2 position = cornerPos + ivec2( x, y );
            row.push_back( Tile::create( position, valueFor( position ) ) );
        }
        mBoard.push_back( std::move( row ) );
    }

    return *this;
}

float TileRenderer::valueFor( const ivec2 &position )
{
    return mPerlin.fBm( ci::vec2( position ) * ci::vec2( 0.1, 0.1 ) );
}

void TileRenderer::jumpTo( const vec2 &p ) {
    // TODO: if the jump point is on the board it maybe be more efficent to move
    // there rather than recreating the board.
    mCenteredOn = tilePosForPoint( p );
    ivec2 cornerTile = mCenteredOn - centerOffset();

    mBoard.clear();
    for ( u_int8_t y = 0; y < mRows; ++y ) {
        Row row;
        for ( u_int8_t x = 0; x < mColumns; ++x ) {
            ivec2 position = cornerTile + ivec2( x, y );
            row.push_back( Tile::create( position, valueFor( position ) ) );
        }
        mBoard.push_back( std::move( row ) );
    }
}

void TileRenderer::move( const vec2 &to ) {
    ivec2 previousCenter = mCenteredOn;
    mCenteredOn = tilePosForPoint( to );
    ivec2 distance = previousCenter - mCenteredOn;

    while ( distance.y != 0 ) {
        if ( distance.y > 0 ) {
            --distance.y;
            moveUp();
        } else {
            ++distance.y;
            moveDown();
        }
    }

    while ( distance.x != 0 ) {
        if ( distance.x > 0 ) {
            --distance.x;
            moveRight();
        }
        else {
            ++distance.x;
            moveLeft();
        }
    }
}

void TileRenderer::moveUp() {
    ivec2 cornerPosition = topLeftPosition();
    Row row;

    for ( u_int8_t x = 0; x < mColumns; ++x ) {
        ivec2 pos = cornerPosition + ivec2( x, -1 );
        row.push_back( Tile::create( pos, valueFor( pos ) ) );
    }
    mBoard.pop_back();
    mBoard.push_front( std::move( row ) );
}

void TileRenderer::moveDown() {
    ivec2 cornerPosition = bottomLeftPosition();
    Row row;

    for ( u_int8_t x = 0; x < mColumns; ++x ) {
        ivec2 pos = cornerPosition + ivec2( x, +1 );
        row.push_back( Tile::create( pos, valueFor( pos ) ) );
    }
    mBoard.pop_front();
    mBoard.push_back( std::move( row ) );
}

void TileRenderer::moveRight() {
    for ( auto &row : mBoard ) {
        ivec2 pos = row.front()->position() + ivec2( -1, 0 );
        row.pop_back();
        row.push_front( Tile::create( pos, valueFor( pos ) ) );
    }
}

void TileRenderer::moveLeft() {
    for ( auto &row : mBoard ) {
        ivec2 pos = row.back()->position() + ivec2( +1, 0 );
        row.pop_front();
        row.push_back( Tile::create( pos, valueFor( pos ) ) );
    }
}

void TileRenderer::draw() {
    for ( const auto& row : mBoard ) {
        for ( const auto& tile : row ) {
            tile->draw( mTileSize );
        }
    }
}

void TileRenderer::Tile::draw( const ivec2 &tileSize ) const {
    gl::ScopedColor color( mColor );
    gl::ScopedModelMatrix innner;
    gl::translate( vec3( tileSize * position(), value() * 200 ) );


    gl::drawSolidRect( Rectf( vec2( 0 ), vec2( tileSize ) ) );
}
