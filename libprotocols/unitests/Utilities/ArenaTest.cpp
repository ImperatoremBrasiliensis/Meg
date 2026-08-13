#include <gtest/gtest.h>

#include <internal/Utilities.h>

class ProtocolsUtilitiesArena: public testing::Test {
protected:
	prosArena arena = {};

	void TearDown() override {
		prosArena_del(&arena);
	}
};

/* Constructor, prosArena_new() */

TEST_F(ProtocolsUtilitiesArena, NewAllocatesJustOneBlock) {
	arena = prosArena_new();

	EXPECT_TRUE(arena.blocks.size);
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);  // Must have one block pointer.
	char *blockp = *static_cast<char **>(prosVector_getLastObj(&arena.blocks));
	EXPECT_EQ(blockp, 0);  // Must be 0, as we don't allocated anything.
}

/* Destructor, prosArena_del() */

TEST_F(ProtocolsUtilitiesArena, DelClearsTheObjet) {
	arena = prosArena_new();
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);
	char *blockp = *static_cast<char **>(prosVector_getLastObj(&arena.blocks));
	ASSERT_EQ(blockp - arena.level, 0);	 // No allocation.

	prosArena_del(&arena);
	ASSERT_EQ(arena.level, nullptr);
}

TEST_F(ProtocolsUtilitiesArena, DelCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			prosArena_del(nullptr);
		},
		""
	);
}

/* Method prosArena_alloc() */

TEST_F(ProtocolsUtilitiesArena, MetAllocReservesMemoryInTheArenaAndReturnsItsPointer) {
	arena = prosArena_new();
	ASSERT_TRUE(arena.blocks);
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);

	ASSERT_NE(prosVector_getSize(&arena.blocks), 1)
		<< "Block pointer array is `nullptr`.";
	char *blockp = *(char **) prosVector_getLastObj(&arena.blocks);
	ASSERT_TRUE(blockp) << "`blockp` pointer is `nullptr`.";
	ASSERT_EQ(blockp - arena.level, 0);

	char *ptr1 = static_cast<char *>(prosArena_alloc(&arena, 16));
	EXPECT_EQ(ptr1, blockp);
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);
	EXPECT_EQ(arena.level, 16);	 // How many was reserved.

	char *ptr2 = static_cast<char *>(prosArena_alloc(&arena, 8));
	EXPECT_NE(ptr2, blockp);
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);
	EXPECT_EQ(arena.level, 24);	 // How many was reserved.
}

TEST_F(ProtocolsUtilitiesArena, MetAllocAllocsNewBlockWhenFull) {
	arena = prosArena_new();
	ASSERT_TRUE(arena.blocks.size);
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);

	char *blockp1, *blockp2;

	char *ptr1 =  // One block allcation.
		static_cast<char *>(prosArena_alloc(&arena, prosARENA_BLOCK_SIZE));

	// Gets the first block pointer.
	ASSERT_NE(prosVector_getSize(&arena.blocks), 1)
		<< "Block pointer array is `nullptr`.";
	blockp1 = *(char **) prosVector_getLastObj(&arena.blocks);
	ASSERT_TRUE(blockp1) << "`blockp` pointer is `nullptr`.";
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);

	EXPECT_EQ(ptr1, blockp1);  // First byte of the block 1.
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);
	EXPECT_EQ(arena.level, prosARENA_BLOCK_SIZE);  // How many was reserved.

	/* Allocates the block 2. */
	char *ptr2 =  // One block allcation again.
		static_cast<char *>(prosArena_alloc(&arena, prosARENA_BLOCK_SIZE));

	// Gets the second block pointer.
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);
	blockp2 = *(char **) prosVector_getLastObj(&arena.blocks);
	ASSERT_TRUE(blockp2) << "`blockp2` pointer is `nullptr`.";
	// Has two blocks now?
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 2);

	EXPECT_EQ(ptr2, blockp2);  // First byte of the block 2.
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);
	EXPECT_EQ(arena.level, prosARENA_BLOCK_SIZE);  // How many was reserved.
}

TEST_F(
	ProtocolsUtilitiesArena,
	MetAllocAllocationGreaterThanDefaultBlockSizeAllocatesAnExclusiveBlock
) {
	/*
    * When you attempt to allocate a block greater than
    * the deault block, the arena `malloc`s a new block
    * directly with the size you requested. However, this
    * block will not be used for new allocation afterwards.
    */

	arena = prosArena_new();
	ASSERT_TRUE(arena.blocks.size);
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 1);

	char *block1p = *(char **) prosVector_getLastObj(&arena.blocks);
	ASSERT_EQ(arena.level, 0);

	char *ptr = static_cast<char *>(prosArena_alloc(&arena, prosARENA_BLOCK_SIZE + 1));
	EXPECT_EQ(prosVector_getSize(&arena.blocks), 2);  // Should have two blocks.
	EXPECT_EQ(arena.level, 0);						  // That block should be unmuted.

	// The exclusive block should be at index 0.
	char *exclusiveBlock = *(char **) prosVector_getAt(&arena.blocks, 0);
	EXPECT_EQ(exclusiveBlock, ptr);
	EXPECT_NE(exclusiveBlock, block1p);
}

TEST_F(ProtocolsUtilitiesArena, MetAllocCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			void *index = prosArena_alloc(nullptr, 4);
		},
		""
	);
}

TEST_F(ProtocolsUtilitiesArena, MetAllocCallWithZeroSizeParameterPanics) {
	EXPECT_DEATH(
		{
			prosArena arena = prosArena_new();
			[[maybe_unused]]
			void *index = prosArena_alloc(&arena, 0);
		},
		""
	);
}

int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
