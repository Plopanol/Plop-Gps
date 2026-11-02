#include "testing/testing.hpp"

#include "map/speed_camera_zones.hpp"

#include "geometry/mercator.hpp"

#include <string>

namespace speed_camera_zones_tests
{
using Type = SpeedCameraZones::Event::Type;

double constexpr kCamLat = 45.361;
double constexpr kCamLon = 4.2526;

// Same layout as the data.gouv.fr file: keys with spaces, coordinates as strings with '+', nullable VMA.
std::string const kJson = R"([
  {"__id":1,"Numéro de radar":"12001","Type de radar":"ETD","Date de mise en service":"2011-10-31T00:00:00","VMA ":70,"Latitude":"+45.361"," Longitude":"+4.2526"},
  {"__id":2,"Numéro de radar":"12002","Type de radar":"ETFR","Date de mise en service":"2012-01-01T00:00:00","VMA ":null,"Latitude":"+46.0"," Longitude":"+5.0"},
  {"__id":3,"Numéro de radar":"12003","Type de radar":"ETF","Date de mise en service":"2012-01-01T00:00:00","VMA ":90,"Latitude":"bad"," Longitude":"+5.0"}
])";

// Location |meters| north (or south if negative) of |lat|, |lon|.
location::GpsInfo MakeInfo(double lat, double lon, double meters, double accuracy = 10.0)
{
  double const kDeltaLat = 0.01;
  double const metersPerDelta =
      mercator::DistanceOnEarth(mercator::FromLatLon(lat, lon), mercator::FromLatLon(lat + kDeltaLat, lon));

  location::GpsInfo info;
  info.m_source = location::EAndroidNative;
  info.m_latitude = lat + meters * kDeltaLat / metersPerDelta;
  info.m_longitude = lon;
  info.m_horizontalAccuracy = accuracy;
  return info;
}

SpeedCameraZones MakeZones(std::string const & json = kJson)
{
  SpeedCameraZones zones;
  TEST(zones.LoadFromJson(json), ());
  return zones;
}

UNIT_TEST(SpeedCameraZones_Parse)
{
  auto const zones = MakeZones();
  TEST_EQUAL(zones.GetCount(), 2, ("The entry with invalid coordinates is skipped"));

  SpeedCameraZones invalid;
  TEST(!invalid.LoadFromJson("{not json"), ());
  TEST_EQUAL(invalid.GetCount(), 0, ());
}

UNIT_TEST(SpeedCameraZones_EnterAndLeave)
{
  auto zones = MakeZones();

  TEST(!zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, -2050)), ());

  auto const entered = zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, -1990));
  TEST(entered, ());
  TEST_EQUAL(entered->m_type, Type::Entered, ());
  TEST_EQUAL(entered->m_maxSpeedKmph, 70, ());

  // No repeated alert inside the zone, nor in the hysteresis band after the camera.
  for (double m : {-1000.0, 0.0, 1000.0, 1990.0, 2050.0})
    TEST(!zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, m)), (m));

  auto const left = zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, 2150));
  TEST(left, ());
  TEST_EQUAL(left->m_type, Type::Left, ());
  TEST(!zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, 3000)), ());

  // Re-armed after leaving.
  auto const again = zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, 1500));
  TEST(again && again->m_type == Type::Entered, ());
}

UNIT_TEST(SpeedCameraZones_UnknownSpeed)
{
  auto zones = MakeZones();
  auto const entered = zones.OnLocationUpdate(MakeInfo(46.0, 5.0, 0));
  TEST(entered && entered->m_type == Type::Entered, ());
  TEST_EQUAL(entered->m_maxSpeedKmph, SpeedCameraZones::kUnknownSpeed, ());
}

UNIT_TEST(SpeedCameraZones_OverlappingZones)
{
  // Two cameras 1.5 km apart give a single zone.
  auto zones = MakeZones(R"([
    {"VMA ":50,"Latitude":"+45.0"," Longitude":"+4.0"},
    {"VMA ":90,"Latitude":"+45.0135"," Longitude":"+4.0"}
  ])");

  int entered = 0, left = 0;
  for (double m = -3000; m <= 6000; m += 100)
  {
    if (auto const e = zones.OnLocationUpdate(MakeInfo(45.0, 4.0, m)))
      ++(e->m_type == Type::Entered ? entered : left);
  }
  TEST_EQUAL(entered, 1, ());
  TEST_EQUAL(left, 1, ());
}

UNIT_TEST(SpeedCameraZones_RedLight)
{
  auto zones = MakeZones(R"([
    {"Type de radar":"ETFR","VMA ":null,"Latitude":"+45.0"," Longitude":"+4.0"}
  ])");

  TEST(!zones.OnLocationUpdate(MakeInfo(45.0, 4.0, -300)), ());
  auto const entered = zones.OnLocationUpdate(MakeInfo(45.0, 4.0, -240));
  TEST(entered && entered->m_type == Type::Entered, ());
  TEST(entered->m_isRedLight, ());

  TEST(!zones.OnLocationUpdate(MakeInfo(45.0, 4.0, 300)), ("Hysteresis"));
  auto const left = zones.OnLocationUpdate(MakeInfo(45.0, 4.0, 360));
  TEST(left && left->m_type == Type::Left, ());
}

UNIT_TEST(SpeedCameraZones_RedLightInsideSpeedZone)
{
  // A red light camera close to a speed camera: the speed zone is entered first.
  auto zones = MakeZones(R"([
    {"Type de radar":"ETFR","VMA ":null,"Latitude":"+45.0"," Longitude":"+4.0"},
    {"Type de radar":"ETF","VMA ":50,"Latitude":"+45.0045"," Longitude":"+4.0"}
  ])");

  auto const entered = zones.OnLocationUpdate(MakeInfo(45.0, 4.0, -1000));
  TEST(entered && !entered->m_isRedLight, ());
  TEST_EQUAL(entered->m_maxSpeedKmph, 50, ());
  TEST(!zones.OnLocationUpdate(MakeInfo(45.0, 4.0, 0)), ());
}

UNIT_TEST(SpeedCameraZones_PoorAccuracyIgnored)
{
  auto zones = MakeZones();
  TEST(!zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, 0, 500.0)), ());
  TEST(zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, 0, 50.0)), ());
}

UNIT_TEST(SpeedCameraZones_RealData)
{
  SpeedCameraZones zones;
  zones.LoadOnce("radars_fixes_fr.json");
  TEST_GREATER(zones.GetCount(), 3000, ());

  size_t redLights = 0;
  zones.ForEachCamera([&redLights](SpeedCameraZones::Camera const & c) { redLights += c.m_isRedLight; });
  TEST_GREATER(redLights, 400, ());

  auto const entered = zones.OnLocationUpdate(MakeInfo(kCamLat, kCamLon, 0));
  TEST(entered && entered->m_type == Type::Entered, ());
}
}  // namespace speed_camera_zones_tests
