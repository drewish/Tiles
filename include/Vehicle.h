//
//  Vehicle.hpp
//  Cityscape
//
//  Created by andrew morton on 7/3/16.
//
//

#pragma once

class Vehicle;

// TODO:
// - only closed polylines should loop
class FlightPlan {
  public:
    FlightPlan( Vehicle *vehicle = nullptr, const ci::PolyLine2f &path = {} );

    void setup( const ci::PolyLine2f &path );
    void draw() const;
    void update();

    ci::vec2 getStartingPosition() const { return ci::vec2( mPath.front() ); }

    ci::vec2 getPreviousTarget() const { return mPath[mPrevPoint]; }
    ci::vec2 getCurrentTarget() const { return mPath[mCurrPoint]; }
    ci::vec2 getNextTarget() const { return mPath[mNextPoint]; }

    ci::vec2 computeSteeringForce( const ci::vec2 &position, const ci::vec2 &velocity );

    void moveToNextSegment();
    float calcSlowingDistance();

  protected:
    Vehicle *mVehicle;
    std::vector<ci::vec2> mPath;
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
class Vehicle {
  public:
    Vehicle( float mass = 1 ) : mMass( mass ) {};

    void setup( const ci::vec2 &position );
    void update( const ci::vec2 &steeringForce );
    void draw() const;

    ci::vec2 getPosition() const { return mPosition; }
    ci::vec2 getLastPosition() const { return mLastPosition; }
    ci::vec2 getVelocity() const { return mVelocity; }


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
    ci::vec2 mPosition; // of the center of gravity?
    ci::vec2 mLastPosition;
    ci::vec2 mVelocity;
    float mMass;
    mutable float mAngle;

    ci::gl::BatchRef mBatch;
    ci::Sphere mBoundingSphere;
};
