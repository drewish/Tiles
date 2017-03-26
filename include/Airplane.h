//
//  Airplane.h
//  Tiles
//
//  Created by Andrew Morton on 3/26/17.
//
//

#pragma once


class Airplane;

// TODO:
// - only closed polylines should loop
class FlightPlan {
  public:
    FlightPlan( Airplane *airplane = nullptr, const ci::PolyLine2f &path = {} );

    void setup( const ci::PolyLine2f &path );
    void draw() const;
    void update();

    ci::vec3 getStartingPosition() const { return mPath.front(); }

    ci::vec3 getPreviousTarget() const { return mPath[mPrevPoint]; }
    ci::vec3 getCurrentTarget() const { return mPath[mCurrPoint]; }
    ci::vec3 getNextTarget() const { return mPath[mNextPoint]; }

    ci::vec3 computeSteeringForce( const ci::vec3 &position, const ci::vec3 &velocity );

    void moveToNextSegment();
    float calcSlowingDistance();

  protected:
    Airplane *mAirplane;
    std::vector<ci::vec3> mPath;
    bool isPathClosed;
    size_t mPrevPoint;
    size_t mCurrPoint;
    size_t mNextPoint;

    float mTurnDistance = 10;
    float mNextTurnRadius;
    float mNextTurnSpeed;
    float mSlowingDistance;

    float mMaxSpeed;
    float mMaxForce;
};

// TODO:
// - interpolate the rotation angle turning
// - draw a custom shape to indicate direction
class Airplane {
  public:
    Airplane( float mass = 1 ) : mMass( mass ) {};

    void setup( const ci::vec3 &position );
    void update( const ci::vec3 &steeringForce );
    void draw() const;

    ci::vec3 getPosition() const { return mPosition; }
    ci::vec3 getLastPosition() const { return mLastPosition; }
    ci::vec3 getVelocity() const { return mVelocity; }


    float getAngle() const
    {
        if ( glm::length2( mVelocity ) > 0.001 ) {
            mAngle = std::atan2( mVelocity.y, mVelocity.x );
        }
        return mAngle;
    }
    float getMass() const { return mMass; }


  protected:
    ci::Color mColor;
    ci::vec3 mPosition; // of the center of gravity?
    ci::vec3 mLastPosition;
    ci::vec3 mVelocity;
    float mMass;
    mutable float mAngle;

    ci::gl::BatchRef mBatch;
    ci::Sphere mBoundingSphere;
};
