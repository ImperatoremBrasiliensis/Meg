#include <gtest/gtest.h>

#include <internal/Utilities.h>
#include <protocols/Utilities.h>

TEST(ProtocolsUtilitiesString, FunLengthReturnsCorrectlyStringLength) {
	const char myString[] = "Long live to Meg!";
	EXPECT_EQ(prosString_length(myString), sizeof(myString) - 1);
}

int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
