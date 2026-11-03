#pragma once

#include "indexer/feature_data.hpp"
#include "indexer/ftypes_mapping.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace ftypes
{
// User-selectable POI groups which can be hidden on the map.
// Values are bit indexes in the persisted mask, so never reorder them (Android poi_categories_values relies on it).
enum class PoiCategory : uint8_t
{
  Food,
  Shops,
  Accommodation,
  Tourism,
  Transport,
  Car,
  Health,
  Services,
  Leisure,
  Worship,
  Count
};

constexpr uint32_t PoiCategoryBit(PoiCategory c)
{
  return 1U << static_cast<uint8_t>(c);
}

std::string DebugPrint(PoiCategory c);

class PoiCategoryClassifier
{
public:
  static PoiCategoryClassifier const & Instance();

  // Types without a category (places, roads, addresses, nature, etc.) are never filtered.
  std::optional<PoiCategory> Get(feature::TypesHolder const & types) const;

private:
  PoiCategoryClassifier();

  HashMapMatcher<uint32_t, PoiCategory> m_matcher;
};
}  // namespace ftypes
