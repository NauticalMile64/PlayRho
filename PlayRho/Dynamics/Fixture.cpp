/*
 * Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <PlayRho/Dynamics/Fixture.hpp>
#include <PlayRho/Dynamics/FixtureProxy.hpp>
#include <PlayRho/Dynamics/Contacts/Contact.hpp>
#include <PlayRho/Dynamics/World.hpp>
#include <PlayRho/Dynamics/Body.hpp>

#include <algorithm>

namespace playrho {

using std::cbegin;
using std::cend;

Fixture::Fixture(const Fixture& other):
    m_body{other.m_body},
    m_userData{other.m_userData},
    m_shape{other.m_shape},
    m_proxyCount{other.m_proxyCount},
    m_filter{other.m_filter},
    m_isSensor{other.m_isSensor}
{
    switch (other.m_proxyCount)
    {
        case 2:
            m_proxies.asArray[1] = other.m_proxies.asArray[1];
            // [[fallthrough]]
        case 1:
            m_proxies.asArray[0] = other.m_proxies.asArray[0];
            // [[fallthrough]]
        case 0:
            break;
        default:
            m_proxies.asBuffer = std::make_unique<FixtureProxy[]>(other.m_proxyCount);
            std::copy(other.m_proxies.asBuffer.get(),
                      other.m_proxies.asBuffer.get() + other.m_proxyCount,
                      m_proxies.asBuffer.get());
            break;
    }
}

FixtureProxy Fixture::GetProxy(ChildCounter index) const noexcept
{
    assert(index < GetProxyCount());
    return (GetProxyCount() <= 2)? m_proxies.asArray[index]: m_proxies.asBuffer[index];
}

void Fixture::Refilter()
{
    const auto body = GetBody();
    const auto world = body->GetWorld();

    // Flag associated contacts for filtering.
    const auto contacts = body->GetContacts();
    std::for_each(cbegin(contacts), cend(contacts), [&](KeyedContactPtr ci) {
        const auto contact = GetContactPtr(ci);
        const auto fixtureA = contact->GetFixtureA();
        const auto fixtureB = contact->GetFixtureB();
        if ((fixtureA == this) || (fixtureB == this))
        {
            contact->FlagForFiltering();
        }
    });
    
    world->TouchProxies(*this);
}

void Fixture::SetSensor(bool sensor) noexcept
{
    if (sensor != m_isSensor)
    {
        // sensor state is changing...
        m_isSensor = sensor;
        const auto body = GetBody();
        if (body != nullptr)
        {
            body->SetAwake();

            const auto contacts = body->GetContacts();
            std::for_each(cbegin(contacts), cend(contacts), [&](KeyedContactPtr ci) {
                const auto contact = GetContactPtr(ci);
                contact->FlagForUpdating();
            });
        }
    }
}

bool TestPoint(const Fixture& f, Length2 p) noexcept
{
    return TestPoint(f.GetShape(), InverseTransform(p, GetTransformation(f)));
}

void SetAwake(const Fixture& f) noexcept
{
    f.GetBody()->SetAwake();
}

Transformation2D GetTransformation(const Fixture& f) noexcept
{
    assert(static_cast<Body*>(f.GetBody()) != nullptr);

    /*
     * If fixtures have transformations (in addition to the body transformation),
     * this could be implemented like:
     *   return Mul(f.GetBody()->GetTransformation(), f.GetTransformation());
     * Note that adding transformations to fixtures requires work to also be done
     * to the manifold calculating code to handle that.
     */
    return f.GetBody()->GetTransformation();
}

} // namespace playrho
