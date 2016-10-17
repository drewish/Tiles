//
//  TileRenderer.cpp
//  Tiles
//
//  Created by andrew morton on 10/15/16.
//
//

#include "TileRenderer.h"

using namespace ci;

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
    gl::ScopedModelMatrix outter;
    gl::translate( mOffset );

    u_int8_t y = 0;
    for ( const auto& row : mBoard ) {
        u_int8_t x = 0;
        for ( const auto& tile : row ) {
            gl::ScopedModelMatrix innner;
            gl::translate( vec2( mTileSize ) * vec2( x, y ) );

            tile->draw( mTileSize );

            x++;
        }
        y++;
    }
}

void TileRenderer::Tile::draw( const vec2 &tileSize ) {
    gl::color( mColor );
    vec2 halfTile = vec2( tileSize ) / vec2( 2 );
    gl::drawSolidRect( Rectf( -halfTile, halfTile ) );
}
