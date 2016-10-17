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

        void draw( const ci::vec2 &tileSize );

        const ci::ivec2&    position() const { return mPosition; }
        const ci::ColorA&   color() const { return mColor; }
        float               value() const { return mValue; }

      protected:
        ci::ivec2 mPosition;
        ci::ColorA mColor;
        float mValue;
    };

    TileRenderer( u_int8_t width, uint8_t height, ci::ivec2 tileSize = ci::ivec2( 50 ) );

    ci::ivec2 tileSize() const { return mTileSize; }

    void move( ci::ivec2 adjustment );
    void moveUp();
    void moveDown();
    void moveRight();
    void moveLeft();

    void draw();

  protected:

    float valueFor( const ci::ivec2 &position );

    const ci::ivec2 mTileSize;
    const u_int8_t mXCount;
    const u_int8_t mYCount;
    ci::Perlin mPerlin;
    ci::vec2 mOffset;
    Board mBoard;
};

#endif /* TileRenderer_hpp */
