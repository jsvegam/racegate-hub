#include <gtest/gtest.h>
#include "data_provider.h"

TEST(DataProviderTest, PilotEntryFieldSizes) {
    // Verify PilotEntry struct has expected field sizes
    PilotEntry entry{};
    entry.position = 1;
    entry.last_lap_ms = 12345;
    entry.best_lap_ms = 11234;
    entry.lap_count = 10;
    entry.gap_ms = -1;
    entry.total_time_ms = 123456;

    EXPECT_EQ(entry.position, 1);
    EXPECT_EQ(entry.last_lap_ms, 12345u);
    EXPECT_EQ(entry.best_lap_ms, 11234u);
    EXPECT_EQ(entry.lap_count, 10);
    EXPECT_EQ(entry.gap_ms, -1);
    EXPECT_EQ(entry.total_time_ms, 123456u);
}

TEST(DataProviderTest, MaxPilotsConstant) {
    EXPECT_EQ(MAX_PILOTS, 8);
}

TEST(DataProviderTest, NameMaxLenConstant) {
    EXPECT_EQ(NAME_MAX_LEN, 12);
}
