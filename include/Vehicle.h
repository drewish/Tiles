//
//  Vehicle.hpp
//  Cityscape
//
//  Created by andrew morton on 7/3/16.
//
//

#pragma once


class FlightPlan {
  public:
    void setup( const ci::PolyLine2f &path, float mass );
	void draw() const;

	ci::vec2 getStartingPosition() { return ci::vec2( mPath.getPoints().front() ); }
	ci::vec2 computeSteeringForce( const ci::vec2 &position, const ci::vec2 &velocity );

    void moveToNextSegment();
    float calcSlowingDistance();

    ci::PolyLine2f mPath;
    size_t mPrevPoint;
    size_t mCurrPoint;
    size_t mNextPoint;

    const float mTurnDistance = 10;
    float mNextTurnRadius;
    float mNextTurnSpeed;
    float mSlowingDistance;

    float mMaxSpeed;
    float mMaxForce;
    float mMass;
};

// TODO:
// - interpolate the rotation angle turning
// - only closed polylines should loop 
// - move this to its own files
// - draw a custom shape to indicate direction
// - build path from highways in city
// - use instanced drawing
// - move into the citydata structure
class Vehicle {
  public:

    void setup( const ci::PolyLine2f &path );
    void update( double dt );
    void draw() const;

    ci::vec2 getPosition() const { return mPosition; }
    ci::vec2 getLastPosition() const { return mLastPosition; }

    float getAngle() const
    {
        if ( glm::length2( mVelocity ) > 0.001 ) {
            mAngle = std::atan2( mVelocity.y, mVelocity.x );
        }
        return mAngle;
    }


  protected:
    ci::Color mColor;
    ci::vec2 mPosition; // of the center of gravity?
    ci::vec2 mLastPosition;
    ci::vec2 mVelocity;
    float mMass;
    mutable float mAngle;

	FlightPlan mPlan;
    ci::gl::BatchRef mBatch;
    ci::Sphere mBoundingSphere;
};
