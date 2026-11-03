#include "testing/testing.hpp"

#include "indexer/classificator.hpp"
#include "indexer/poi_category.hpp"

#include "generator/generator_tests_support/test_with_classificator.hpp"

namespace poi_category_tests
{
using namespace generator::tests_support;
using ftypes::PoiCategory;

std::optional<PoiCategory> GetCategory(base::StringIL const & path)
{
  feature::TypesHolder types;
  types.Add(classif().GetTypeByPath(path));
  return ftypes::PoiCategoryClassifier::Instance().Get(types);
}

UNIT_CLASS_TEST(TestWithClassificator, PoiCategory_Smoke)
{
  TEST_EQUAL(GetCategory({"amenity", "restaurant"}), PoiCategory::Food, ());
  TEST_EQUAL(GetCategory({"shop", "bakery"}), PoiCategory::Shops, ());
  TEST_EQUAL(GetCategory({"tourism", "hotel"}), PoiCategory::Accommodation, ());
  TEST_EQUAL(GetCategory({"tourism", "museum"}), PoiCategory::Tourism, ());
  TEST_EQUAL(GetCategory({"historic", "castle"}), PoiCategory::Tourism, ());
  TEST_EQUAL(GetCategory({"highway", "bus_stop"}), PoiCategory::Transport, ());
  TEST_EQUAL(GetCategory({"amenity", "fuel"}), PoiCategory::Car, ());
  TEST_EQUAL(GetCategory({"amenity", "pharmacy"}), PoiCategory::Health, ());
  TEST_EQUAL(GetCategory({"amenity", "bank"}), PoiCategory::Services, ());
  TEST_EQUAL(GetCategory({"leisure", "park"}), PoiCategory::Leisure, ());
  TEST_EQUAL(GetCategory({"amenity", "place_of_worship"}), PoiCategory::Worship, ());
}

UNIT_CLASS_TEST(TestWithClassificator, PoiCategory_LongestPrefix)
{
  TEST_EQUAL(GetCategory({"shop", "car_repair"}), PoiCategory::Car, ());
  TEST_EQUAL(GetCategory({"tourism", "camp_site"}), PoiCategory::Accommodation, ());
}

UNIT_CLASS_TEST(TestWithClassificator, PoiCategory_NotFiltered)
{
  TEST(!GetCategory({"place", "city"}), ());
  TEST(!GetCategory({"highway", "residential"}), ());
  TEST(!GetCategory({"building"}), ());
  TEST(!GetCategory({"natural", "peak"}), ());
}

UNIT_CLASS_TEST(TestWithClassificator, PoiCategory_BuildingWithAmenity)
{
  auto const & c = classif();
  feature::TypesHolder types;
  types.Add(c.GetTypeByPath({"building"}));
  types.Add(c.GetTypeByPath({"amenity", "school"}));
  TEST_EQUAL(ftypes::PoiCategoryClassifier::Instance().Get(types), PoiCategory::Services, ());
}
}  // namespace poi_category_tests
