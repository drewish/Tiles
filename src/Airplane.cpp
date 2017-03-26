//
//  Airplane.cpp
//  Tiles
//
//  Created by Andrew Morton on 3/26/17.
//
//

#include "Airplane.h"

using namespace ci;

#include <boost/format.hpp>
#include <glm/gtx/vector_angle.hpp>
// TODO:
// - interpolate the rotation angle turning
// - only closed polylines should loop
// - build path from highways in city
// - use instanced drawing
// - move into the citydata structure
// - create a car model

#include "Resources.h"
#include "cinder/ObjLoader.h"


vec3 truncate( vec3 v, float s )
{
    if( glm::length2( v ) >  s * s )
        return glm::normalize( v ) * s;
    return v;
}


FlightPlan::FlightPlan( Airplane *Airplane, const ci::PolyLine2f &path )
    : mAirplane( Airplane )
{
	float height = 1;
	for( auto &point : path.getPoints() ) {
		mPath.push_back( vec3( point, 1 ) );
	}
    isPathClosed = path.isClosed();
    mMaxSpeed = 4;
    mMaxForce = 0.5;

    // If it's closed and the front and back aren't the same point add it
    // to close it up.
    if( path.isClosed() && glm::distance2( mPath.front(), mPath.back() ) > 0.001 ) {
        mPath.push_back( mPath.front() );
    }

    mPrevPoint = mPath.size() > 1 ? mPath.size() - 1 : 0;
    mCurrPoint = 0;
    mNextPoint = mPath.size() > 1 ? 1 : 0;

    if( mAirplane ) {
        moveToNextSegment();
        mAirplane->setup( getStartingPosition() );
    }
};

void FlightPlan::update()
{
    if( mAirplane ) {
        mAirplane->update( computeSteeringForce( mAirplane->getPosition(), mAirplane->getVelocity() ) );
    }
}

// All this is based of Reynold's steering behaviors
// http://www.red3d.com/cwr/steer/gdc99/
vec3 FlightPlan::computeSteeringForce( const ci::vec3 &position, const ci::vec3 &velocity )
{
    if( mPath.size() < 2 ) return vec3( 0 );

    // steer to arrive at next point
    vec3 target_offset = getCurrentTarget() - position;
    float distance = glm::length( target_offset );
    float ramped_speed = mNextTurnSpeed * ( distance / mSlowingDistance );
    float clipped_speed = std::min( ramped_speed, mMaxSpeed );
    vec3 desired_velocity = ( clipped_speed / distance ) * target_offset;
    vec3 steering_direction = desired_velocity - velocity;
    vec3 steering_force = truncate( steering_direction, mMaxForce );

    // When we get close to the target, shift to the next point
    if( distance < mNextTurnRadius && glm::length2( velocity ) < ( mNextTurnSpeed * mNextTurnSpeed ) ) {
        moveToNextSegment();
    }

    return steering_force;
}

void findRadius( float turnDistance, const vec3 &v1, const vec3 &v2, const vec3 &v3, float &r, vec3 &center )
{
    // Put the point at the origin create two normal vectors heading
    // to the adjacent points.
    vec3 prev = glm::normalize( v1 - v2 );
    vec3 next = glm::normalize( v3 - v2 );

    // Find the radius of circle tangent to the vectors leaving this
    // point d distance away.
    /*
                     d
           prev <--+----+ target
                   |  θ/ \
                  r|  /   \
                   | /h    v
                   |/      next
                   +

        θ=inner angle/2
        tan(θ)=r/d => r=d*tan(θ)
        cos(θ)=d/h => h=d/cos(θ)

        I love having to relearn trig for stuff like this.
    */
    float diff = glm::angle( prev, next ) / 2.0;
    r = turnDistance * std::tan( diff );

    // If we want to display the circle we'll need to locate the
    // center which is on the hypotenuse.
    vec3 middle = ( prev + next ) / vec3( 2 );
// TODO: make sure this makes sense in 3 dimensions
    float middleAngle = std::atan2( middle.y, middle.x );
    float h = turnDistance / std::cos( diff );
    center = v2 + vec3( std::cos( middleAngle ), std::sin( middleAngle ), 0 ) * h;
}

void FlightPlan::moveToNextSegment()
{
    mPrevPoint = mCurrPoint;
    mCurrPoint = mNextPoint;
    ++mNextPoint;

    if( mNextPoint > mPath.size() - ( isPathClosed ? 2 : 1 ) ) {
        mNextPoint = 0;
    }

    vec3 p = glm::normalize( mPath[mCurrPoint] - mPath[mPrevPoint] );
    vec3 n = glm::normalize( mPath[mNextPoint] - mPath[mCurrPoint] );
    float diff = glm::angle( n, p );
    // TODO: Come up with a more gradual way of handling this
    if( diff <= M_PI_4 + 0.1 ) {
        mNextTurnSpeed = mMaxSpeed;
    } else if( diff <= M_PI_2 + 0.1 ) {
        mNextTurnSpeed = mMaxSpeed / 2;
    } else {
        mNextTurnSpeed = mMaxSpeed / 3;
    }

    vec3 _center;
    findRadius( mTurnDistance, mPath[mPrevPoint], mPath[mCurrPoint], mPath[mNextPoint], mNextTurnRadius, _center );

    mSlowingDistance = calcSlowingDistance();
}

float FlightPlan::calcSlowingDistance()
{
    float max_accel = mMaxForce / mAirplane->getMass();
    float fudge_factor = 1.1;
    return fudge_factor * std::abs( mMaxSpeed * mMaxSpeed - mNextTurnSpeed * mNextTurnSpeed ) / ( 2.0 * max_accel );
}

void FlightPlan::draw() const
{
    if( true ) {
        gl::draw( mPath );
    }

    // Slowing boundary
    if( true ) {
        gl::ScopedColor color( 1, 0, 0 );
        gl::drawStrokedCircle( vec2( getCurrentTarget() ), 2 * mSlowingDistance );
    }

    // Text debugging info
    if( true ) {
        gl::ScopedMatrices mat;
        gl::setMatricesWindow( cinder::app::getWindowSize() );
        boost::format formatter( "%07.5f" );
        Color color = Color::black();
        gl::drawString( "Turn:  " + (formatter % mNextTurnSpeed).str(), vec2( 10, 25 ), color );
        gl::drawString( "Max:   " + (formatter % mMaxSpeed).str(), vec2( 10, 40 ), color );
        gl::drawString( "Dist:  " + (formatter % mSlowingDistance).str(), vec2( 10, 55 ), color );
    }
}


void Airplane::setup( const vec3 &position )
{
    mAngle = 0;
    mVelocity = vec3( 0 );

    mPosition = position;
    mLastPosition = mPosition;

    // Load the model in so we have something to draw
    ObjLoader loader( app::loadResource( RES_CAR_01_OBJ ) );
    TriMeshRef mesh = TriMesh::create( loader );
    if( ! loader.getAvailableAttribs().count( geom::NORMAL ) ) mesh->recalculateNormals();
    mBatch = gl::Batch::create( *mesh, gl::getStockShader( gl::ShaderDef().color() ) );

    mColor = Color::white();
}

void Airplane::update( const vec3 &steeringForce )
{
    vec3 acceleration = steeringForce / mMass;
    mVelocity = mVelocity + acceleration;
    mLastPosition = mPosition;
    mPosition = mPosition + mVelocity;
}

void Airplane::draw() const
{
    gl::ScopedColor color( mColor );
    gl::ScopedModelMatrix matrix;

    gl::translate( getPosition() );
    gl::rotate( getAngle(), vec3( 0, 0, 1 ) );
    // Drawing a vector can help understand the heading.
//    gl::drawVector( vec3( 0, 0, 0 ), vec3( 10, 0, 0 ), 20, 10);

    // For what ever reason we have to do some rotating to get the model
    // facing the right direction.
    gl::rotate( 1.571, vec3( 1, 0, 0 ) );
    gl::rotate( 1.571, vec3( 0, 1, 0 ) );
    gl::scale( vec3( 10 ) );

    mBatch->draw();
}
