#include "indexer/poi_category.hpp"

#include "base/assert.hpp"
#include "base/stl_helpers.hpp"

namespace ftypes
{
std::string DebugPrint(PoiCategory c)
{
  switch (c)
  {
  case PoiCategory::Food: return "Food";
  case PoiCategory::Shops: return "Shops";
  case PoiCategory::Accommodation: return "Accommodation";
  case PoiCategory::Tourism: return "Tourism";
  case PoiCategory::Transport: return "Transport";
  case PoiCategory::Car: return "Car";
  case PoiCategory::Health: return "Health";
  case PoiCategory::Services: return "Services";
  case PoiCategory::Leisure: return "Leisure";
  case PoiCategory::Worship: return "Worship";
  case PoiCategory::Count: return "Count";
  }
  UNREACHABLE();
}

// static
PoiCategoryClassifier const & PoiCategoryClassifier::Instance()
{
  static PoiCategoryClassifier const inst;
  return inst;
}

PoiCategoryClassifier::PoiCategoryClassifier()
{
  // The longest matching prefix wins, e.g. shop-car_repair is Car while other shops are Shops.
  base::StringIL const food[] = {
      {"amenity", "restaurant"}, {"amenity", "cafe"},       {"amenity", "fast_food"},  {"amenity", "bar"},
      {"amenity", "pub"},        {"amenity", "biergarten"}, {"amenity", "food_court"}, {"amenity", "ice_cream"},
  };
  base::StringIL const shops[] = {{"shop"}, {"amenity", "marketplace"}};
  base::StringIL const accommodation[] = {
      {"tourism", "hotel"},     {"tourism", "hostel"},       {"tourism", "motel"},       {"tourism", "guest_house"},
      {"tourism", "apartment"}, {"tourism", "chalet"},       {"tourism", "camp_site"},   {"tourism", "caravan_site"},
      {"tourism", "alpine_hut"}, {"tourism", "wilderness_hut"},
  };
  base::StringIL const tourism[] = {
      {"tourism"}, {"historic"}, {"amenity", "theatre"}, {"amenity", "cinema"}, {"amenity", "arts_centre"},
  };
  base::StringIL const transport[] = {
      {"public_transport"},        {"railway", "station"},      {"railway", "halt"},
      {"railway", "tram_stop"},    {"railway", "subway_entrance"}, {"highway", "bus_stop"},
      {"aeroway", "aerodrome"},    {"amenity", "bus_station"},  {"amenity", "ferry_terminal"},
      {"amenity", "taxi"},         {"amenity", "bicycle_rental"},
  };
  base::StringIL const car[] = {
      {"amenity", "parking"},  {"amenity", "fuel"},       {"amenity", "charging_station"},
      {"amenity", "car_wash"}, {"amenity", "car_rental"}, {"shop", "car_repair"},
  };
  base::StringIL const health[] = {
      {"healthcare"},         {"amenity", "hospital"}, {"amenity", "clinic"},     {"amenity", "doctors"},
      {"amenity", "dentist"}, {"amenity", "pharmacy"}, {"amenity", "veterinary"},
  };
  base::StringIL const services[] = {
      {"amenity", "bank"},           {"amenity", "atm"},    {"amenity", "post_office"}, {"amenity", "police"},
      {"amenity", "townhall"},       {"amenity", "library"}, {"amenity", "toilets"},    {"amenity", "drinking_water"},
      {"amenity", "school"},         {"amenity", "university"}, {"amenity", "kindergarten"}, {"office"},
      {"craft"},
  };
  base::StringIL const leisure[] = {{"leisure"}, {"sport"}};
  base::StringIL const worship[] = {{"amenity", "place_of_worship"}};

  m_matcher.Append(food, PoiCategory::Food);
  m_matcher.Append(shops, PoiCategory::Shops);
  m_matcher.Append(accommodation, PoiCategory::Accommodation);
  m_matcher.Append(tourism, PoiCategory::Tourism);
  m_matcher.Append(transport, PoiCategory::Transport);
  m_matcher.Append(car, PoiCategory::Car);
  m_matcher.Append(health, PoiCategory::Health);
  m_matcher.Append(services, PoiCategory::Services);
  m_matcher.Append(leisure, PoiCategory::Leisure);
  m_matcher.Append(worship, PoiCategory::Worship);
}

std::optional<PoiCategory> PoiCategoryClassifier::Get(feature::TypesHolder const & types) const
{
  auto const it = m_matcher.Find(types);
  if (!m_matcher.IsValid(it))
    return {};
  return it->second;
}
}  // namespace ftypes
