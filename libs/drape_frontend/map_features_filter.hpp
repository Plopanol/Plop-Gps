#pragma once

#include "indexer/poi_category.hpp"

#include <cstdint>

namespace df
{
// User choice of map features which are not drawn at all.
struct MapFeaturesFilter
{
  bool IsPoiCategoryHidden(ftypes::PoiCategory c) const
  {
    return (m_hiddenPoiCategories & ftypes::PoiCategoryBit(c)) != 0;
  }

  bool operator==(MapFeaturesFilter const &) const = default;

  bool m_buildingsEnabled = true;
  // Bit mask of ftypes::PoiCategoryBit().
  uint32_t m_hiddenPoiCategories = 0;
};
}  // namespace df
