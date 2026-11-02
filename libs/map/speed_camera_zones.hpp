#pragma once

#include "platform/location.hpp"

#include "geometry/point2d.hpp"
#include "geometry/rect2d.hpp"
#include "geometry/tree4d.hpp"

#include <cstdint>
#include <optional>
#include <string>

// Danger zones around fixed speed cameras from the French open data (data.gouv.fr "Liste des radars fixes").
// Works without a route: an event is emitted when the user enters or leaves a zone.
class SpeedCameraZones
{
public:
  // The data has no direction, so the zone is a disc which covers 2 km before and after the camera.
  static double constexpr kZoneRadiusM = 2000.0;
  // Red light cameras are in towns, a wide zone would alert several streets away.
  static double constexpr kRedLightZoneRadiusM = 250.0;
  // Avoids Entered/Left flickering on the zone border because of GPS noise.
  static double constexpr kExitHysteresisM = 100.0;
  static double constexpr kMaxHorizontalAccuracyM = 100.0;
  static uint8_t constexpr kUnknownSpeed = 0;

  struct Event
  {
    enum class Type
    {
      Entered,
      Left
    };

    Type m_type;
    // Speed limit of the closest camera on entering, |kUnknownSpeed| if not set or on leaving.
    uint8_t m_maxSpeedKmph = kUnknownSpeed;
    bool m_isRedLight = false;
  };

  struct Camera
  {
    m2::RectD GetLimitRect() const { return m2::RectD(m_point, m_point); }
    double GetZoneRadiusM() const { return m_isRedLight ? kRedLightZoneRadiusM : kZoneRadiusM; }

    m2::PointD m_point;
    uint8_t m_maxSpeedKmph = kUnknownSpeed;
    bool m_isRedLight = false;
  };

  // Loads the cameras once from the resources file, subsequent calls do nothing.
  void LoadOnce(std::string const & fileName);
  // Returns false if |json| can't be parsed. Entries with invalid coordinates are skipped.
  bool LoadFromJson(std::string const & json);

  size_t GetCount() const { return m_cameras.GetSize(); }

  template <typename ToDo>
  void ForEachCamera(ToDo && toDo) const
  {
    m_cameras.ForEach(toDo);
  }

  std::optional<Event> OnLocationUpdate(location::GpsInfo const & info);

private:
  m4::Tree<Camera> m_cameras;
  bool m_loadAttempted = false;
  bool m_inZone = false;
};

std::string DebugPrint(SpeedCameraZones::Event::Type type);
