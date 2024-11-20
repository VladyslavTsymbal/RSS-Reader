#include "MockNetworkUtils.hpp"

namespace {

using namespace testing;

struct NetworkUtilsTest : public Test
{
    std::unique_ptr<StrictMock<MockNetworkUtils>> network_utils =
            std::make_unique<StrictMock<MockNetworkUtils>>();
};

TEST_F(NetworkUtilsTest, when_true)
{
    ASSERT_TRUE(true);
}

} // namespace