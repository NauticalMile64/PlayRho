/*
 * Author: Chris Campbell - www.iforce2d.net
 *
 * Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef IFORCE2D_TOPDOWN_CAR_HPP
#define IFORCE2D_TOPDOWN_CAR_HPP

#include "../Framework/Test.hpp"
#include <vector>
#include <set>

namespace testbed {

using ControlStateType = unsigned int;

enum ControlState: ControlStateType
{
    TDC_LEFT     = 0x1,
    TDC_RIGHT    = 0x2,
    TDC_UP       = 0x4,
    TDC_DOWN     = 0x8
};


//types of fixture user data
enum fixtureUserDataType
{
    FUD_CAR_TIRE,
    FUD_GROUND_AREA
};

//a class to allow subclassing of different fixture user data
class FixtureUserData
{
private:
    fixtureUserDataType m_type;

protected:
    FixtureUserData(fixtureUserDataType type) : m_type{type} {}

public:
    virtual fixtureUserDataType getType() { return m_type; }
    virtual ~FixtureUserData() {}
};

//class to allow marking a fixture as a car tire
class CarTireFUD : public FixtureUserData
{
public:
    CarTireFUD() : FixtureUserData{FUD_CAR_TIRE} {}
};

//class to allow marking a fixture as a ground area
class GroundAreaFUD : public FixtureUserData
{
public:
    float frictionModifier;
    bool outOfCourse;
    
    GroundAreaFUD(float fm, bool ooc):
        FixtureUserData{FUD_GROUND_AREA},
        frictionModifier{fm},
        outOfCourse{ooc}
    {
    }
};


class TDTire
{
private:
    Body* m_body;
    std::set<GroundAreaFUD*> m_groundAreas;
    Force m_maxDriveForce = 0_N;
    LinearVelocity m_maxForwardSpeed = 0_mps;
    LinearVelocity m_maxBackwardSpeed = 0_mps;
    Momentum m_maxLateralImpulse = 0_Ns;
    Real m_currentTraction = 1;
    
public:
    
    TDTire(World* world, Shape tireShape)
    {
        BodyConf bodyConf;
        bodyConf.type = BodyType::Dynamic;
        m_body = world->CreateBody(bodyConf);
        
        const auto fixture = m_body->CreateFixture(tireShape);
        fixture->SetUserData( new CarTireFUD() );
        
        m_body->SetUserData( this );
    }
    
    ~TDTire()
    {
        m_body->GetWorld()->Destroy(m_body);
    }
    
    void setCharacteristics(LinearVelocity maxForwardSpeed, LinearVelocity maxBackwardSpeed, Force maxDriveForce, Momentum maxLateralImpulse)
    {
        m_maxForwardSpeed = maxForwardSpeed;
        m_maxBackwardSpeed = maxBackwardSpeed;
        m_maxDriveForce = maxDriveForce;
        m_maxLateralImpulse = maxLateralImpulse;
    }
    
    void addGroundArea(GroundAreaFUD* ga) { m_groundAreas.insert(ga); updateTraction(); }
    void removeGroundArea(GroundAreaFUD* ga) { m_groundAreas.erase(ga); updateTraction(); }
    
    void updateTraction()
    {
        if ( m_groundAreas.empty() )
            m_currentTraction = 1;
        else
        {
            //find area with highest traction
            m_currentTraction = 0;
            auto it = m_groundAreas.begin();
            while (it != m_groundAreas.end())
            {
                const auto ga = *it;
                if ( ga->frictionModifier > m_currentTraction )
                    m_currentTraction = ga->frictionModifier;
                ++it;
            }
        }
    }
    
    Body* GetBody() const
    {
        return m_body;
    }
    
    LinearVelocity2 getLateralVelocity() const
    {
        const auto currentRightNormal = GetWorldVector(*m_body, UnitVec::GetRight());
        const auto vel = GetLinearVelocity(*m_body);
        return Dot(currentRightNormal, vel) * currentRightNormal;
    }
    
    LinearVelocity2 getForwardVelocity() const
    {
        const auto currentForwardNormal = GetWorldVector(*m_body, UnitVec::GetTop());
        const auto vel = GetLinearVelocity(*m_body);
        return Dot(currentForwardNormal, vel) * currentForwardNormal;
    }
    
    void updateFriction()
    {
        //lateral linear velocity
        auto impulse = Momentum2{GetMass(*m_body) * -getLateralVelocity()};
        const auto length = GetMagnitude(GetVec2(impulse)) * 1_kg * 1_mps;
        if ( length > m_maxLateralImpulse )
            impulse *= m_maxLateralImpulse / length;
        ApplyLinearImpulse(*m_body, m_currentTraction * impulse, m_body->GetWorldCenter());
        
        //angular velocity
        const auto rotInertia = GetRotInertia(*m_body);
        PLAYRHO_CONSTEXPR const auto Tenth = Real{1} / Real{10};
        ApplyAngularImpulse(*m_body, m_currentTraction * Tenth * rotInertia * -GetAngularVelocity(*m_body));
        
        //forward linear velocity
        const auto forwardVelocity = getForwardVelocity();
        const auto uvresult = UnitVec::Get(StripUnit(GetX(forwardVelocity)), StripUnit(GetY(forwardVelocity)));
        const auto forwardDir = std::get<UnitVec>(uvresult);
        const auto currentForwardSpeed = std::get<Real>(uvresult) * 1_mps;
        const auto dragForceMagnitude = -2 * currentForwardSpeed;
        const auto newForce = Force2{m_currentTraction * dragForceMagnitude * forwardDir * 1_kg / 1_s};
        SetForce(*m_body, newForce, m_body->GetWorldCenter());
    }
    
    void updateDrive(ControlStateType controlState)
    {
        //find desired speed
        auto desiredSpeed = 0_mps;
        switch ( controlState & (TDC_UP|TDC_DOWN) ) {
            case TDC_UP:   desiredSpeed = m_maxForwardSpeed;  break;
            case TDC_DOWN: desiredSpeed = m_maxBackwardSpeed; break;
            default: return;//do nothing
        }
        
        //find current speed in forward direction
        const auto currentForwardNormal = GetWorldVector(*m_body, UnitVec::GetTop());
        const auto currentSpeed = Dot(getForwardVelocity(), currentForwardNormal);
        
        //apply necessary force
        auto forceMagnitude = 0_N;
        if (desiredSpeed > currentSpeed)
            forceMagnitude = m_maxDriveForce;
        else if (desiredSpeed < currentSpeed)
            forceMagnitude = -m_maxDriveForce;
        else
            return;
        
        const auto newForce = Force2{m_currentTraction * forceMagnitude * currentForwardNormal};
        SetForce(*m_body, newForce, m_body->GetWorldCenter());
    }
    
    void updateTurn(ControlStateType controlState)
    {
        auto desiredTorque = 0_Nm;
        switch (controlState & (TDC_LEFT|TDC_RIGHT))
        {
            case TDC_LEFT:  desiredTorque = +15_Nm; break;
            case TDC_RIGHT: desiredTorque = -15_Nm; break;
            default: ;//nothing
        }
        SetTorque(*m_body, desiredTorque);
    }
};


class TDCar
{
private:
    Body* m_body;
    std::vector<TDTire*> m_tires;
    RevoluteJoint *flJoint, *frJoint;

public:
    TDCar(World* world)
    {
        //create car body
        BodyConf bodyConf;
        bodyConf.type = BodyType::Dynamic;
        m_body = world->CreateBody(bodyConf);
        m_body->SetAngularDamping(3_Hz);
        
        Length2 vertices[8];
        vertices[0] = Vec2(+1.5f,  +0.0f) * 1_m;
        vertices[1] = Vec2(+3.0f,  +2.5f) * 1_m;
        vertices[2] = Vec2(+2.8f,  +5.5f) * 1_m;
        vertices[3] = Vec2(+1.0f, +10.0f) * 1_m;
        vertices[4] = Vec2(-1.0f, +10.0f) * 1_m;
        vertices[5] = Vec2(-2.8f,  +5.5f) * 1_m;
        vertices[6] = Vec2(-3.0f,  +2.5f) * 1_m;
        vertices[7] = Vec2(-1.5f,  +0.0f) * 1_m;
        auto polygonShape = PolygonShapeConf{};
        polygonShape.Set(Span<const Length2>(vertices, 8));
        polygonShape.UseDensity(0.1_kgpm2);
        m_body->CreateFixture(Shape(polygonShape));
        
        //prepare common joint parameters
        RevoluteJointConf jointConf;
        jointConf.bodyA = m_body;
        jointConf.enableLimit = true;
        jointConf.lowerAngle = 0_deg;
        jointConf.upperAngle = 0_deg;
        jointConf.localAnchorB = Length2{}; //center of tire
        
        const auto maxForwardSpeed = 250_mps;
        const auto maxBackwardSpeed = -40_mps;
        const auto backTireMaxDriveForce = 950_N; // 300.0f;
        const auto frontTireMaxDriveForce = 400_N; // 500.0f;
        const auto backTireMaxLateralImpulse = 9_Ns; // 8.5f;
        const auto frontTireMaxLateralImpulse = 9_Ns; // 7.5f;

        auto tireShape = PolygonShapeConf{};
        tireShape.SetAsBox(0.5_m, 1.25_m);
        tireShape.UseDensity(1_kgpm2);
        const auto sharedTireShape = Shape(tireShape);

        TDTire* tire;

        //back left tire (starts at absolute 0, 0 but pulled into place by joint)
        tire = new TDTire{world, sharedTireShape};
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        jointConf.bodyB = tire->GetBody();
        jointConf.localAnchorA = Vec2(-3, 0.75f) * 1_m; // sets car relative location of tire
        world->CreateJoint(jointConf);
        m_tires.push_back(tire);
        
        //back right tire (starts at absolute 0, 0 but pulled into place by joint)
        tire = new TDTire{world, sharedTireShape};
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        jointConf.bodyB = tire->GetBody();
        jointConf.localAnchorA = Vec2(+3, 0.75f) * 1_m; // sets car relative location of tire
        world->CreateJoint(jointConf);
        m_tires.push_back(tire);
        
        //front left tire (starts at absolute 0, 0 but pulled into place by joint)
        tire = new TDTire{world, sharedTireShape};
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
        jointConf.bodyB = tire->GetBody();
        jointConf.localAnchorA = Vec2(-3, 8.5f) * 1_m; // sets car relative location of tire
        flJoint = static_cast<RevoluteJoint*>(world->CreateJoint(jointConf));
        m_tires.push_back(tire);
        
        //front right tire (starts at absolute 0, 0 but pulled into place by joint)
        tire = new TDTire{world, sharedTireShape};
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
        jointConf.bodyB = tire->GetBody();
        jointConf.localAnchorA = Vec2(+3, 8.5f) * 1_m; // sets car relative location of tire
        frJoint = static_cast<RevoluteJoint*>(world->CreateJoint(jointConf));
        m_tires.push_back(tire);
    }
    
    ~TDCar()
    {
        for (auto i = decltype(m_tires.size()){0}; i < m_tires.size(); i++)
            delete m_tires[i];
    }
    
    void update(ControlStateType controlState)
    {
        for (auto i = decltype(m_tires.size()){0}; i < m_tires.size(); i++)
        {
            m_tires[i]->updateFriction();
        }
        for (auto i = decltype(m_tires.size()){0}; i < m_tires.size(); i++)
        {
            m_tires[i]->updateDrive(controlState);
        }
        
        //control steering
        const auto lockAngle = 35_deg;
        const auto turnSpeedPerSec = 160_deg;//from lock to lock in 0.5 sec
        const auto turnPerTimeStep = turnSpeedPerSec / Real{60.0f};
        auto desiredAngle = 0_deg;
        switch ( controlState & (TDC_LEFT|TDC_RIGHT) ) {
            case TDC_LEFT:  desiredAngle = lockAngle;  break;
            case TDC_RIGHT: desiredAngle = -lockAngle; break;
            default: ;//nothing
        }
        const auto angleNow = GetJointAngle(*flJoint);
        const auto desiredAngleToTurn = desiredAngle - angleNow;
        const auto angleToTurn = Clamp(desiredAngleToTurn, -turnPerTimeStep, turnPerTimeStep);
        if (angleToTurn != 0_deg)
        {
            const auto newAngle = angleNow + angleToTurn;
            flJoint->SetLimits( newAngle, newAngle );
            frJoint->SetLimits( newAngle, newAngle );
        }
    }
};


class MyDestructionListener :  public DestructionListener
{
    void SayGoodbye(Fixture& fixture) override
    {
        const auto fud = static_cast<FixtureUserData*>(fixture.GetUserData());
        if ( fud )
            delete fud;
    }
    
    //(unused but must implement all pure virtual functions)
    void SayGoodbye(Joint&) override {}
};


class iforce2d_TopdownCar : public Test
{
public:
    static Test::Conf GetTestConf()
    {
        auto conf = Test::Conf{};
        conf.seeAlso = "https://www.iforce2d.net/b2dtut/projected-trajectory";
        conf.credits = "Originally written by Chris Campbell for Box. Ported to PlayRho by Louis Langholtz.";
        return conf;
    }

    iforce2d_TopdownCar(): Test(GetTestConf())
    {
        m_gravity = LinearAcceleration2{};
        m_world.SetDestructionListener(&m_destructionListener);
        
        //set up ground areas
        {
            Fixture* groundAreaFixture;

            BodyConf bodyConf;
            m_groundBody = m_world.CreateBody(bodyConf);
            
            auto polygonShape = PolygonShapeConf{};
            FixtureConf fixtureConf;
            fixtureConf.isSensor = true;
            
            polygonShape.SetAsBox(9_m, 7_m, Vec2(-10,15) * 1_m, 20_deg );
            groundAreaFixture = m_groundBody->CreateFixture(Shape(polygonShape), fixtureConf);
            groundAreaFixture->SetUserData( new GroundAreaFUD( 0.5f, false ) );
            
            polygonShape.SetAsBox(9_m, 5_m, Vec2(5,20) * 1_m, -40_deg );
            groundAreaFixture = m_groundBody->CreateFixture(Shape(polygonShape), fixtureConf);
            groundAreaFixture->SetUserData( new GroundAreaFUD( 0.2f, false ) );
        }
        
        //m_tire = new TDTire(m_world);
        //m_tire->setCharacteristics(100, -20, 150);
        
        m_car = new TDCar{&m_world};
        m_controlState = 0;
        
        RegisterForKey(GLFW_KEY_A, GLFW_PRESS, 0, "Turn left.", [&](KeyActionMods) {
            m_controlState |= TDC_LEFT;
        });
        RegisterForKey(GLFW_KEY_D, GLFW_PRESS, 0, "Turn right.", [&](KeyActionMods) {
            m_controlState |= TDC_RIGHT;
        });
        RegisterForKey(GLFW_KEY_W, GLFW_PRESS, 0, "Accelerate forward.", [&](KeyActionMods) {
            m_controlState |= TDC_UP;
        });
        RegisterForKey(GLFW_KEY_S, GLFW_PRESS, 0, "Accelerate backward.", [&](KeyActionMods) {
            m_controlState |= TDC_DOWN;
        });
        
        RegisterForKey(GLFW_KEY_A, GLFW_RELEASE, 0, "Stop turning left.", [&](KeyActionMods) {
            m_controlState &= ~TDC_LEFT;
        });
        RegisterForKey(GLFW_KEY_D, GLFW_RELEASE, 0, "Stop turning right.", [&](KeyActionMods) {
            m_controlState &= ~TDC_RIGHT;
        });
        RegisterForKey(GLFW_KEY_W, GLFW_RELEASE, 0, "Stop accelerating forward.", [&](KeyActionMods) {
            m_controlState &= ~TDC_UP;
        });
        RegisterForKey(GLFW_KEY_S, GLFW_RELEASE, 0, "Stop accelerating backward.", [&](KeyActionMods) {
            m_controlState &= ~TDC_DOWN;
        });
    }
    
    ~iforce2d_TopdownCar()
    {
        //delete m_tire;
        delete m_car;
    }
    
    void handleContact(Contact* contact, bool began)
    {
        const auto fA = contact->GetFixtureA();
        const auto fB = contact->GetFixtureB();
        const auto fudA = (FixtureUserData*)fA->GetUserData();
        const auto fudB = (FixtureUserData*)fB->GetUserData();
        
        if ( !fudA || !fudB )
            return;
        
        if ( fudA->getType() == FUD_CAR_TIRE || fudB->getType() == FUD_GROUND_AREA )
            tire_vs_groundArea(fA, fB, began);
        else if ( fudA->getType() == FUD_GROUND_AREA || fudB->getType() == FUD_CAR_TIRE )
            tire_vs_groundArea(fB, fA, began);
    }
    
    void BeginContact(Contact& contact) override { handleContact(&contact, true); }
    void EndContact(Contact& contact) override { handleContact(&contact, false); }
    
    void tire_vs_groundArea(Fixture* tireFixture, Fixture* groundAreaFixture, bool began)
    {
        const auto tire = (TDTire*)tireFixture->GetBody()->GetUserData();
        const auto gaFud = (GroundAreaFUD*)groundAreaFixture->GetUserData();
        if ( began )
            tire->addGroundArea( gaFud );
        else
            tire->removeGroundArea( gaFud );
    }
    
    void PreStep(const Settings&, Drawer&) override
    {
        /*m_tire->updateFriction();
         m_tire->updateDrive(m_controlState);
         m_tire->updateTurn(m_controlState);*/
        
        m_car->update(m_controlState);
    }
    
    ControlStateType m_controlState;
    MyDestructionListener m_destructionListener;
    Body* m_groundBody;
    //TDTire* m_tire;
    TDCar* m_car;
};

} // namespace testbed

#endif
