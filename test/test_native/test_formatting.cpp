#include <gtest/gtest.h>
#include "render.h"

// --- format_time tests ---

TEST(FormatTimeTest, BasicConversion) {
    char buf[16];
    render::format_time(12345, buf, sizeof(buf));
    EXPECT_STREQ(buf, "12.345");
}

TEST(FormatTimeTest, ZeroMilliseconds) {
    char buf[16];
    render::format_time(0, buf, sizeof(buf));
    EXPECT_STREQ(buf, "0.000");
}

TEST(FormatTimeTest, ExactSeconds) {
    char buf[16];
    render::format_time(5000, buf, sizeof(buf));
    EXPECT_STREQ(buf, "5.000");
}

TEST(FormatTimeTest, SubSecond) {
    char buf[16];
    render::format_time(42, buf, sizeof(buf));
    EXPECT_STREQ(buf, "0.042");
}

TEST(FormatTimeTest, LargeValue) {
    char buf[16];
    render::format_time(99999, buf, sizeof(buf));
    EXPECT_STREQ(buf, "99.999");
}

TEST(FormatTimeTest, TypicalLapTime) {
    char buf[16];
    render::format_time(15234, buf, sizeof(buf));
    EXPECT_STREQ(buf, "15.234");
}

TEST(FormatTimeTest, NullBuffer) {
    // Should not crash
    render::format_time(12345, nullptr, 16);
}

TEST(FormatTimeTest, ZeroBufferSize) {
    char buf[16] = "unchanged";
    render::format_time(12345, buf, 0);
    EXPECT_STREQ(buf, "unchanged");
}

// --- format_gap tests ---

TEST(FormatGapTest, LeaderGap) {
    char buf[16];
    render::format_gap(-1, buf, sizeof(buf));
    EXPECT_STREQ(buf, "---");
}

TEST(FormatGapTest, ZeroGap) {
    char buf[16];
    render::format_gap(0, buf, sizeof(buf));
    EXPECT_STREQ(buf, "+0.000");
}

TEST(FormatGapTest, PositiveGap) {
    char buf[16];
    render::format_gap(1234, buf, sizeof(buf));
    EXPECT_STREQ(buf, "+1.234");
}

TEST(FormatGapTest, LargeGap) {
    char buf[16];
    render::format_gap(65432, buf, sizeof(buf));
    EXPECT_STREQ(buf, "+65.432");
}

TEST(FormatGapTest, SmallGap) {
    char buf[16];
    render::format_gap(7, buf, sizeof(buf));
    EXPECT_STREQ(buf, "+0.007");
}

TEST(FormatGapTest, NullBuffer) {
    // Should not crash
    render::format_gap(-1, nullptr, 16);
}

TEST(FormatGapTest, ZeroBufferSize) {
    char buf[16] = "unchanged";
    render::format_gap(-1, buf, 0);
    EXPECT_STREQ(buf, "unchanged");
}

// --- init_render tests ---

TEST(InitRenderTest, ClearsState) {
    render::RenderState state;
    // Fill with garbage
    std::memset(&state, 0xFF, sizeof(state));
    render::init_render(state);

    EXPECT_EQ(state.last_pilot_count, 0);
    EXPECT_FALSE(state.header_drawn);
    // Verify cells are zeroed
    EXPECT_STREQ(state.cells[0][0].text, "");
    EXPECT_EQ(state.cells[0][0].fg_color, 0);
}
