//
//  TileRenderer.hpp
//  Tiles
//
//  Created by andrew morton on 10/15/16.
//
//

#ifndef TileRenderer_hpp
#define TileRenderer_hpp

#include "cinder/Perlin.h"
#include "cinder/Rand.h"


class TileRenderer {
  public:

    class Tile;
    typedef std::shared_ptr<Tile> TileRef;

    typedef std::list<TileRef> Row;
    typedef std::list<Row> Board;

    class Tile {
      public:
        static TileRef create( const ci::ivec2 &position, float value ) {
            return TileRef( new Tile( position, value ) );
        };

        Tile( const ci::ivec2 &position, float value )
            : mPosition( position ), mValue( value )
        {
            mColor = ci::Color( ci::CM_HSV, 0.5 + mValue, 1.0, 1.0 );
        }

        void draw( const ci::ivec2 &tileSize ) const;

        const ci::ivec2&    position() const { return mPosition; }
        const ci::ColorA&   color() const { return mColor; }
        float               value() const { return mValue; }

      protected:
        ci::ivec2 mPosition;
        ci::ColorA mColor;
        float mValue;
    };


    TileRenderer( const ci::uvec2 &boardSize = ci::uvec2( 1 ), const ci::uvec2 &tileSize = ci::uvec2( 50 ) );
    TileRenderer& operator=( const TileRenderer &rhs );

    // Dimentions of an individual tile.
    ci::ivec2 tileSize() const { return mTileSize; }
    // Size of the board in tiles.
    ci::ivec2 boardSize() const { return ci::ivec2( mColumns, mRows ); }

    ci::ivec2 centerOffset() const {
        return glm::floor( ci::vec2( mColumns, mRows ) / ci::vec2( 2.0 ) );
    }
    ci::ivec2 centerPosition() const { return mCenteredOn; }
    ci::ivec2 topLeftPosition() const { return mBoard.front().front()->position(); }
    ci::ivec2 bottomLeftPosition() const { return mBoard.back().front()->position(); }

    // Recreate the tiles so the board is centered over this point.
    void jumpTo( const ci::vec2 &point );
    // Move the center to this point creating tiles along the way.
    void move( const ci::vec2 &to );
    void moveUp();
    void moveDown();
    void moveRight();
    void moveLeft();

    void draw();

  protected:

    ci::ivec2 tilePosForPoint( const ci::vec2 &point ) const {
        return ci::ivec2( glm::floor( point / ci::vec2( mTileSize ) ) );
    }
    float valueFor( const ci::ivec2 &position );

    ci::ivec2 mCenteredOn;
    ci::ivec2 mTileSize;
    u_int8_t mColumns;
    u_int8_t mRows;
    ci::Perlin mPerlin;
    Board mBoard;
};

#endif /* TileRenderer_hpp */
