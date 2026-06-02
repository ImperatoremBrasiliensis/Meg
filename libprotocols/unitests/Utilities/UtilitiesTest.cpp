#include <gtest/gtest.h>

#include <internal/Utilities.h>
#include <protocols/Utilities.h>

#include <stdio.h>

#ifdef _WIN32
#	define fileno _fileno
#endif

TEST(ProtocolsUtilities, FunMemcpyCopiesCorrectly) {
	char src[15] = "Hello Meg!";
	char dst[15]{};

	EXPECT_EQ(pros_memcpy(src, dst, 15), dst);
	EXPECT_STREQ(dst, src);
}

TEST(ProtocolsUtilities, FunDoesNotCopyMoreThanSpecified) {
	char src[20] = "1234567890123456789";
	char dst[20];
	memset(dst, 0, 20);

	EXPECT_EQ(pros_memcpy(src, dst, 10), dst);
	for (int i = 10; i < 20; i++)
		EXPECT_EQ(dst[i], 0) << "Iteration: " << i;
}

TEST(ProtocolsUtilities, FunMemcpyCallWithZeroNParameterDoesNotCopyAndReturnsNullptr) {
	char src[15] = "Hello Meg!";
	char dst[15]{};

	EXPECT_EQ(pros_memcpy(src, dst, 0), nullptr);
	EXPECT_NE(memcmp(dst, src, 15), 0);
}

TEST(ProtocolsUtilities, FunMemcpyCallWithNullDestOrSrcParameterReturnsNullptr) {
	char src1[15] = "Hello Meg!";
	char dst1[15]{};

	EXPECT_EQ(pros_memcpy(nullptr, dst1, 15), nullptr);
	EXPECT_NE(memcmp(dst1, src1, 15), 0);

	char src2[15] = "Hello Meg!";
	char dst2[15]{};

	EXPECT_EQ(pros_memcpy(src2, nullptr, 15), nullptr);
	EXPECT_NE(memcmp(dst2, src2, 15), 0);
}

TEST(ProtocolsUtilities, FunPrintPrintsCorrectlyInTheSpecifiedFd) {
	FILE *out = tmpfile();
	ASSERT_TRUE(out) << "`out` must not be `nullptr`.";
	int fd = fileno(out);

	pros_print(fd, "Message", "this is my message.");
	fflush(out);
	fseek(out, 0, SEEK_SET);

	char buf[60];
	memset(buf, 0, sizeof(buf));
	size_t sizeRead = fread(buf, 1, sizeof(buf) - 1, out);

	ASSERT_GT(sizeRead, 0) << "Humm... Something went wrong.";
	buf[sizeRead] = '\0';

	EXPECT_STREQ(buf, "\033[1;4mProtocols:\033[0m\nMessage: this is my message.\n");
	fclose(out);
}

TEST(ProtocolsUtilities, FunPrintInsertsANewLineIfThereIsNoOne) {
	FILE *out = tmpfile();
	ASSERT_TRUE(out) << "`out` must not be `nullptr`.";
	int fd = fileno(out);

	/* Round 1 */ {
		// It must insert a newline at the end.
		pros_print(fd, "Message", "this is my message.");
		fflush(out);
		fseek(out, 0, SEEK_SET);

		char buf[60];
		memset(buf, 0, sizeof(buf));
		size_t sizeRead = fread(buf, 1, sizeof(buf) - 1, out);

		ASSERT_GT(sizeRead, 0) << "Humm... Something went wrong.";
		buf[sizeRead] = '\0';

		EXPECT_STREQ(buf, "\033[1;4mProtocols:\033[0m\nMessage: this is my message.\n");
	}

	fseek(out, 0, SEEK_SET);

	/* Round 2 */ {
		// It must not insert a newline at the end, there's already one.
		pros_print(fd, "Message", "this is my message.");
		fflush(out);
		fseek(out, 0, SEEK_SET);

		char buf[60];
		memset(buf, 0, sizeof(buf));
		size_t sizeRead = fread(buf, 1, sizeof(buf) - 1, out);

		ASSERT_GT(sizeRead, 0) << "Humm... Something went wrong.";
		buf[sizeRead] = '\0';

		EXPECT_STREQ(buf, "\033[1;4mProtocols:\033[0m\nMessage: this is my message.\n");
	}	 // Same output.

	fclose(out);
}

TEST(ProtocolsUtilities, FunPrintCallWithEmptyStringAsPrefixParamPrintsOnlyWithTheDefaultPrefix) {
	FILE *out = tmpfile();
	ASSERT_TRUE(out);
	int fd = fileno(out);

	pros_print(fd, "", "this is my message.");
	fflush(out);
	fseek(out, 0, SEEK_SET);

	char buf[60];
	memset(buf, 0, sizeof(buf));
	size_t sizeRead = fread(buf, 1, sizeof(buf) - 1, out);

	ASSERT_GT(sizeRead, 0) << "Humm... Something went wrong.";
	buf[sizeRead] = '\0';

	EXPECT_STREQ(buf, "\033[1;4mProtocols:\033[0m\nthis is my message.\n");
	fclose(out);
}

TEST(ProtocolsUtilities, FunPrintCallWithNullPrefixParamPrintsOnlyTheStrParam) {
	FILE *out = tmpfile();
	ASSERT_TRUE(out);
	int fd = fileno(out);

	pros_print(fd, nullptr, "this is my message.");
	fflush(out);
	fseek(out, 0, SEEK_SET);

	char buf[60];
	memset(buf, 0, sizeof(buf));
	size_t sizeRead = fread(buf, 1, sizeof(buf) - 1, out);

	ASSERT_GT(sizeRead, 0) << "Humm... Something went wrong.";
	buf[sizeRead] = '\0';

	EXPECT_STREQ(buf, "this is my message.\n");
	fclose(out);
}

TEST(ProtocolsUtilities, FunPrintCallWithNullStrParamDoesNothing) {
	FILE *out = tmpfile();
	ASSERT_TRUE(out);
	int fd = fileno(out);

	// With null `pfx`.
	pros_print(fd, nullptr, nullptr);
	fflush(out);
	fseek(out, 0, SEEK_END);
	EXPECT_EQ(ftell(out), 0);

	// With empty string in `str`.
	pros_print(fd, "", nullptr);
	fflush(out);
	fseek(out, 0, SEEK_END);
	EXPECT_EQ(ftell(out), 0);

	// With a valid string in `pfx`.
	pros_print(fd, nullptr, nullptr);
	fflush(out);
	fseek(out, 0, SEEK_END);
	EXPECT_EQ(ftell(out), 0);

	fclose(out);
}

int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
