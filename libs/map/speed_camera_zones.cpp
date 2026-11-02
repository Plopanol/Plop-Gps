#include "map/speed_camera_zones.hpp"

#include "platform/platform.hpp"

#include "coding/reader.hpp"

#include "geometry/mercator.hpp"

#include "base/logging.hpp"
#include "base/string_utils.hpp"

#include <glaze/json.hpp>

#include <limits>
#include <vector>

namespace speed_camera_zones_json
{
struct Camera
{
  std::string type;
  std::optional<int> vma;
  std::string lat;
  std::string lon;
};
}  // namespace speed_camera_zones_json

// Keys of the original data.gouv.fr file, including their stray spaces.
template <>
struct glz::meta<speed_camera_zones_json::Camera>
{
  using T = speed_camera_zones_json::Camera;
  static constexpr auto value = object("Type de radar", &T::type, "VMA ", &T::vma, "Latitude", &T::lat, " Longitude", &T::lon);
};

namespace
{
// Type code of red light cameras.
std::string_view constexpr kRedLightType = "ETFR";

bool ParseCoord(std::string_view s, double & d)
{
  strings::Trim(s);
  if (s.starts_with('+'))
    s.remove_prefix(1);
  return strings::to_double(s, d);
}
}  // namespace

void SpeedCameraZones::LoadOnce(std::string const & fileName)
{
  if (m_loadAttempted)
    return;
  m_loadAttempted = true;

  std::string data;
  try
  {
    ReaderPtr<Reader>(GetPlatform().GetReader(fileName)).ReadAsString(data);
  }
  catch (RootException const & ex)
  {
    LOG(LWARNING, ("Loading speed cameras failed:", ex.what()));
    return;
  }

  if (LoadFromJson(data))
    LOG(LINFO, ("Loaded", GetCount(), "speed cameras from", fileName));
}

bool SpeedCameraZones::LoadFromJson(std::string const & json)
{
  std::vector<speed_camera_zones_json::Camera> cameras;
  glz::opts constexpr opts{.error_on_unknown_keys = false};
  if (auto const error = glz::read<opts>(cameras, json); error)
  {
    LOG(LWARNING, ("Parsing speed cameras failed:", glz::format_error(error, json)));
    return false;
  }

  for (auto const & c : cameras)
  {
    double lat, lon;
    if (!ParseCoord(c.lat, lat) || !ParseCoord(c.lon, lon) || !mercator::ValidLat(lat) || !mercator::ValidLon(lon))
    {
      LOG(LWARNING, ("Invalid speed camera coordinates:", c.lat, c.lon));
      continue;
    }

    uint8_t speed = kUnknownSpeed;
    if (c.vma && *c.vma > 0 && *c.vma <= std::numeric_limits<uint8_t>::max())
      speed = static_cast<uint8_t>(*c.vma);

    m_cameras.Add(Camera{mercator::FromLatLon(lat, lon), speed, c.type == kRedLightType});
  }
  return true;
}

std::optional<SpeedCameraZones::Event> SpeedCameraZones::OnLocationUpdate(location::GpsInfo const & info)
{
  if (info.m_horizontalAccuracy > kMaxHorizontalAccuracyM)
    return {};

  auto const pos = mercator::FromLatLon(info.m_latitude, info.m_longitude);
  double const hysteresis = m_inZone ? kExitHysteresisM : 0.0;

  Camera const * closest = nullptr;
  double minDist = std::numeric_limits<double>::max();
  m_cameras.ForEachInRect(mercator::RectByCenterXYAndSizeInMeters(pos, kZoneRadiusM + hysteresis), [&](Camera const & c)
  {
    double const dist = mercator::DistanceOnEarth(pos, c.m_point);
    if (dist <= c.GetZoneRadiusM() + hysteresis && dist < minDist)
    {
      minDist = dist;
      closest = &c;
    }
  });

  if (!m_inZone && closest)
  {
    m_inZone = true;
    return Event{Event::Type::Entered, closest->m_maxSpeedKmph, closest->m_isRedLight};
  }
  if (m_inZone && !closest)
  {
    m_inZone = false;
    return Event{Event::Type::Left};
  }
  return {};
}

std::string DebugPrint(SpeedCameraZones::Event::Type type)
{
  switch (type)
  {
  case SpeedCameraZones::Event::Type::Entered: return "Entered";
  case SpeedCameraZones::Event::Type::Left: return "Left";
  }
  UNREACHABLE();
}
