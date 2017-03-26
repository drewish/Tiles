//
//  Vehicle.cpp
//  Cityscape
//
//  Created by andrew morton on 7/3/16.
//
//

#include "Vehicle.h"

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


vec2 truncate( vec2 v, float s )
{
    if( glm::length2( v ) >  s * s )
        return glm::normalize( v ) * s;
    return v;
}

void FlightPlan::setup( const PolyLine2f &path, float mass )
{
    mPath = path;
    mMaxSpeed = 4;
    mMaxForce = 0.5;
    mMass = mass;

    // If it's closed and the front and back aren't the same point add it
    // to close it up.
    if( path.isClosed() && glm::distance2( mPath.getPoints().front(), mPath.getPoints().back() ) > 0.001 ) {
        mPath.getPoints().push_back( mPath.getPoints().front() );
    }

    mPrevPoint = mPath.size() > 1 ? mPath.size() - 1 : 0;
    mCurrPoint = 0;
    mNextPoint = mPath.size() > 1 ? 1 : 0;
    moveToNextSegment();
}

vec2 FlightPlan::computeSteeringForce( const ci::vec2 &position, const ci::vec2 &velocity )
{
    if( mPath.size() < 2 ) return vec2( 0 );

    vec2 target = mPath.getPoints()[mCurrPoint];

    // steer to arrive at next point
    vec2 target_offset = target - position;
    float distance = glm::length( target_offset );
    float ramped_speed = mNextTurnSpeed * ( distance / mSlowingDistance );
    float clipped_speed = std::min( ramped_speed, mMaxSpeed );
    vec2 desired_velocity = ( clipped_speed / distance ) * target_offset;
    vec2 steering_direction = desired_velocity - velocity;
    vec2 steering_force = truncate( steering_direction, mMaxForce );

    // When we get close to the target, shift to the next point
    if( distance < mNextTurnRadius && glm::length2( velocity ) < ( mNextTurnSpeed * mNextTurnSpeed ) ) {
        moveToNextSegment();
    }

    return steering_force;
}

void findRadius( float turnDistance, const vec2 &v1, const vec2 &v2, const vec2 &v3, float &r, vec2 &center )
{
    // Put the point at the origin create two normal vectors heading
    // to the adjacent points.
    vec2 prev = glm::normalize( v1 - v2 );
    vec2 next = glm::normalize( v3 - v2 );

    // Find the radius of circle tangent to the vectors leaving this
    // point d distance away.
    /*
                     d
           prev <--+----+
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
    vec2 middle = ( prev + next ) / vec2( 2 );
    float middleAngle = std::atan2( middle.y, middle.x );
    float h = turnDistance / std::cos( diff );
    center = v2 + vec2( std::cos( middleAngle ), std::sin( middleAngle ) ) * h;
}

void FlightPlan::moveToNextSegment()
{
    mPrevPoint = mCurrPoint;
    mCurrPoint = mNextPoint;
    ++mNextPoint;

    if( mNextPoint > mPath.size() - ( mPath.isClosed() ? 2 : 1 ) ) {
        mNextPoint = 0;
    }

    std::vector<vec2> &points = mPath.getPoints();

    vec2 p = glm::normalize( points[mCurrPoint] - points[mPrevPoint] );
    vec2 n = glm::normalize( points[mNextPoint] - points[mCurrPoint] );
    float diff = glm::angle( n, p );
    // TODO: Come up with a more gradual way of handling this
    if( diff <= M_PI_4 + 0.1 ) {
        mNextTurnSpeed = mMaxSpeed;
    } else if( diff <= M_PI_2 + 0.1 ) {
        mNextTurnSpeed = mMaxSpeed / 2;
    } else {
        mNextTurnSpeed = mMaxSpeed / 3;
    }

    vec2 _center;
    findRadius( mTurnDistance, points[mPrevPoint], points[mCurrPoint], points[mNextPoint], mNextTurnRadius, _center );

    mSlowingDistance = calcSlowingDistance();
}

float FlightPlan::calcSlowingDistance()
{
    float max_accel = mMaxForce / mMass;
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
        const std::vector<vec2> &points = mPath.getPoints();
        gl::drawStrokedCircle( points[mCurrPoint], 2 * mSlowingDistance );
    }

    // Text debugging info
    if( /* DISABLES CODE */ (false) ) {
        gl::ScopedMatrices mat;
        gl::setMatricesWindow( cinder::app::getWindowSize() );
        boost::format formatter( "%07.5f" );
        Color color = Color::black();
        gl::drawString( "Turn:  " + (formatter % mNextTurnSpeed).str(), vec2( 10, 25 ), color );
        gl::drawString( "Max:   " + (formatter % mMaxSpeed).str(), vec2( 10, 40 ), color );
        gl::drawString( "Dist:  " + (formatter % mSlowingDistance).str(), vec2( 10, 55 ), color );
    }
}


void Vehicle::setup( const PolyLine2f &path )
{
    mAngle = 0;
    mVelocity = vec2( 0 );
    mMass = 5;

    mPlan.setup( path, mMass );

    mPosition = mPlan.getStartingPosition();
    mLastPosition = mPosition;


    // Load the model in so we have something to draw
    ObjLoader loader( app::loadResource( RES_CAR_01_OBJ ) );
    std::cout << "has groups: " << loader.getNumGroups() << "\n";
    for ( const ObjLoader::Group &group : loader.getGroups() ) {
        std::cout << "name: " << group.mName << "\n";
    }
    TriMeshRef mesh = TriMesh::create( loader );
    if( ! loader.getAvailableAttribs().count( geom::NORMAL ) ) mesh->recalculateNormals();
    mBatch = gl::Batch::create( *mesh, gl::getStockShader( gl::ShaderDef().color() ) );
}

// All this is based of Reynold's steering behaviors
// http://www.red3d.com/cwr/steer/gdc99/
void Vehicle::update( double dt )
{
    vec2 steering_force = mPlan.computeSteeringForce( mPosition, mVelocity );

    vec2 acceleration = steering_force / mMass;
    mVelocity = truncate( mVelocity + acceleration, mPlan.mMaxSpeed );
    mLastPosition = mPosition;
    mPosition = mPosition + mVelocity;

//    if( ramped_speed < plan.mMaxSpeed )
//        mColor = Color( 1, 0, 0 );
////    else if( accelerating? )
////        mColor = Color( 0, 1, 0 );
//    else
        mColor = Color::white();

}

void Vehicle::draw() const
{
    gl::ScopedColor color( mColor );

    {
        gl::ScopedModelMatrix matrix;

        gl::translate( getPosition() );
        gl::rotate( getAngle(), vec3( 0, 0, 1 ) );
        // Drawing a vector can help understand the heading.
//        gl::drawVector( vec3( 0, 0, 0 ), vec3( 10, 0, 0 ), 20, 10);

        // For what ever reason we have to do some rotating to get the model
        // facing the right direction.
        gl::rotate( 1.571, vec3( 1, 0, 0 ) );
        gl::rotate( 1.571, vec3( 0, 1, 0 ) );
        gl::scale( vec3( 10 ) );

        mBatch->draw();
    }

    mPlan.draw();
}
