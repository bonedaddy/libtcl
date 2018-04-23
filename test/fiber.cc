/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

#include <gtest/gtest.h>

#include "tempow/bt/hci.h"

int add(int a, int b) {
	return a + b;
}

TEST(test1, c1) {
	EXPECT_EQ(3, add(1, 2));
}

GTEST_API_ int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
