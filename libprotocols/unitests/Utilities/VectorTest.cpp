#include <gtest/gtest.h>

#include <protocols/Utilities.h>

struct prosVector_s {
	size_t typeSize;
	uint32_t capacity;
	uint32_t size;
};

struct alignas(8) PaddedObject {
	uint8_t a;
	uint32_t b;
	uint16_t c;
};

class ProtocolsUtilitiesVector: public testing::Test {
protected:
	prosVector v = nullptr;

	void TearDown() override {
		if (v)
			prosVector_del(&v);
	}
};

/* Constructor prosVector_new() */

TEST_F(ProtocolsUtilitiesVector, NewReturnsValidPointerAndStoresTypeSizeInTheAllocatedStruct) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	EXPECT_EQ(v->typeSize, sizeof(int));
	EXPECT_EQ(v->capacity, 0);
	EXPECT_EQ(v->size, 0);
}

TEST_F(ProtocolsUtilitiesVector, NewCallWithZeroTypeSizeParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			prosVector v = prosVector_new(0);
		},
		""
	);
}

/* Destructor prosVector_del() */

TEST_F(ProtocolsUtilitiesVector, DelFreesTheMemoryInHeapAndTurnsHandleNullptr) {
	v = prosVector_new(sizeof(int));
	prosVector_del(&v);
	EXPECT_FALSE(v);
}

TEST_F(ProtocolsUtilitiesVector, DelCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			prosVector_del(nullptr);
		},
		""
	);
}

/* Method prosVector_reserve() */

TEST_F(ProtocolsUtilitiesVector, MetReserveIncreaseCapacityAndPreserveData) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 4);

	ASSERT_NO_FATAL_FAILURE(prosVector_reserve(&v, 256)) << "Humm... Something went wrong.";
	ASSERT_TRUE(v) << "Humm... Something went wrong.";
	EXPECT_EQ(prosVector_getCapacity(&v), 256);
	for (int i = 0; i < 4; i++)
		EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, i)), vl[i]);
}

TEST_F(ProtocolsUtilitiesVector, MetReserveReservationSizeLessThanCapacityDoesNothing) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	ASSERT_EQ(v->capacity, 4);
	ASSERT_EQ(v->size, 4);

	ASSERT_NO_FATAL_FAILURE(prosVector_reserve(&v, 0));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->capacity, 4);
	EXPECT_EQ(v->size, 4);
}

TEST_F(ProtocolsUtilitiesVector, MetReserveCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			prosVector_reserve(nullptr, 0);
		},
		""
	);
}

/* Method prosVector_pushBack() */

TEST_F(ProtocolsUtilitiesVector, MetPushBackAppendsObjectAndUpdatesSizeWhileCapacityHasRoom) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	EXPECT_EQ(v->size, 0u);
	EXPECT_EQ(v->capacity, 0u);

	int vl[] = {10, 20, 30, 40};

	prosVector_pushBack(&v, vl);
	EXPECT_EQ(v->size, 1u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[0]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 0)), vl[0]);

	prosVector_pushBack(&v, &vl[1]);
	EXPECT_EQ(v->size, 2u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[1]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 1)), vl[1]);

	prosVector_pushBack(&v, &vl[2]);
	EXPECT_EQ(v->size, 3u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[2]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 2)), vl[2]);

	prosVector_pushBack(&v, &vl[3]);
	EXPECT_EQ(v->size, 4u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[3]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 3)), vl[3]);

	EXPECT_FALSE(prosVector_getAt(&v, 4));
}

TEST_F(ProtocolsUtilitiesVector, MetPushBackDoublesCapacityWhileUpdatesSizeAndPreservesItsObjects) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

	for (uint32_t i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	const int initialCapacity = v->capacity;

	ASSERT_EQ(v->size, 4u) << "Humm... Something went wrong.";
	ASSERT_EQ(v->capacity, 4u) << "Humm... Something went wrong.";

	prosVector_pushBack(&v, &vl[4]);
	EXPECT_EQ(v->size, 5u);
	EXPECT_EQ(v->capacity, initialCapacity * 2);

	for (uint32_t i = 0; i < 5; i++)
		EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, i)), vl[i]);

	for (uint32_t i = 5; i < 8; i++)
		prosVector_pushBack(&v, &vl[i]);

	EXPECT_EQ(v->size, 8u);
	EXPECT_EQ(v->capacity, initialCapacity * 2);

	prosVector_pushBack(&v, &vl[8]);
	EXPECT_EQ(v->size, 9u);
	EXPECT_EQ(v->capacity, initialCapacity * 4);

	for (uint32_t i = 0; i < 9; i++)
		EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, i)), vl[i]);

	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), 8);
}

TEST_F(ProtocolsUtilitiesVector, MetPushBackCopiesBytesAndDoesNotAliasSourceMemory) {
	v = prosVector_new(sizeof(PaddedObject));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(PaddedObject));

	PaddedObject obj;
	memset(&obj, 0xAf, sizeof(obj));	// To `obj` (including padding) contain only known bytes.
	obj = {
		.a = 0x11u,
		.b = 0x22334455u,
		.c = 0x6677u
	};

	std::array<uint8_t, sizeof(PaddedObject)> expected{};
	memcpy(expected.data(), &obj, sizeof(obj));

	prosVector_pushBack(&v, &obj);
	memset(&obj, 0x00, sizeof(obj));

	EXPECT_EQ(memcmp(prosVector_getData(&v), expected.data(), sizeof(PaddedObject)), 0);
}

TEST_F(ProtocolsUtilitiesVector, MetPushBackCallWithNullSelfParameterPanics) {
	int value = 1;
	EXPECT_DEATH(
		{
			prosVector_pushBack(nullptr, &value);
		},
		""
	);
}

TEST_F(ProtocolsUtilitiesVector, MetPushBackCallWithNullObjParameterPanics) {
	EXPECT_DEATH(
		{
			prosVector vext = prosVector_new(sizeof(int));
			ASSERT_TRUE(vext);
			prosVector_pushBack(&vext, nullptr);
		},
		""
	);
}

/* Method prosVector_popBack() */

TEST_F(ProtocolsUtilitiesVector, MetPopBackErasesTheLastElementAndUpdatesSize) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 4);

	EXPECT_TRUE(prosVector_popBack(&v));
	EXPECT_EQ(v->size, 3);
	for (int i = 0; i < 3; i++)
		EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, i)), vl[i]);
}

TEST_F(ProtocolsUtilitiesVector, MetPopBackOnEmptyVectorReturnsFalse) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));
	ASSERT_EQ(v->capacity, 0) << "To proceed, `capacity` field must be 0.";
	ASSERT_EQ(v->size, 0);

	/* On a vector with no memory reserve. */
	EXPECT_FALSE(prosVector_popBack(&v));
	EXPECT_EQ(v->capacity, 0);
	EXPECT_EQ(v->size, 0);

	// Turns vector empty.
	int vl = 1;
	prosVector_pushBack(&v, &vl);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 1);

	EXPECT_TRUE(prosVector_popBack(&v));
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 0);

	/* On empty vector. */
	EXPECT_FALSE(prosVector_popBack(&v));
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 0);
}

TEST_F(ProtocolsUtilitiesVector, MetPopBackCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			prosVector_popBack(nullptr);
		},
		""
	);
}

/* Method prosVector_insert() */

TEST_F(ProtocolsUtilitiesVector, MetInsertInsertsObjectAndUpdatesSizeWhileCapacityHasRoom) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);

	int vl[] = {0, 1, 2, 3};

	prosVector_insert(&v, &vl[0], 0);
	EXPECT_EQ(v->size, 1u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[0]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 0)), vl[0]);

	prosVector_insert(&v, &vl[1], 1);
	EXPECT_EQ(v->size, 2u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[1]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 1)), vl[1]);

	prosVector_insert(&v, &vl[2], 2);
	EXPECT_EQ(v->size, 3u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[2]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 2)), vl[2]);

	prosVector_insert(&v, &vl[3], 3);
	EXPECT_EQ(v->size, 4u);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[3]);
	EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, 3)), vl[3]);

	EXPECT_FALSE(prosVector_getAt(&v, 4));
}

TEST_F(ProtocolsUtilitiesVector, MetInsertWithSpecificInsertionOrderDoublesCapacityWhileUpdatesSizeAndPreserveTheObjects) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 4);

	int vl2[] = {4, 5, 6, 7, 8};

	prosVector_insert(&v, &vl2[0], 2);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE * 2);
	EXPECT_EQ(v->size, 5);
	[[maybe_unused]] int (&p)[8] = *static_cast<int (*)[8]>(prosVector_getData(&v));

	prosVector_insert(&v, &vl2[1], 0);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE * 2);
	EXPECT_EQ(v->size, 6);

	prosVector_insert(&v, &vl2[2], 6);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE * 2);
	EXPECT_EQ(v->size, 7);

	prosVector_insert(&v, &vl2[3], 2);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE * 2);
	EXPECT_EQ(v->size, 8);

	int expectedValues[] = {5, 0, 7, 1, 4, 2, 3, 6};
	for (int i = 0; i < 8; i++)
		EXPECT_EQ(expectedValues[i], *static_cast<int *>(prosVector_getAt(&v, i))) << "Iteration: " << i;

	prosVector_insert(&v, &vl2[4], 1);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE * 4);
	EXPECT_EQ(v->size, 9);
}

TEST_F(ProtocolsUtilitiesVector, MetInsertCopiesBytesAndDoesNotAliasSourceMemory) {
	v = prosVector_new(sizeof(PaddedObject));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(PaddedObject));

	PaddedObject obj;
	memset(&obj, 0XAF, sizeof(PaddedObject));	 // To `obj` (including padding) contain only known bytes.
	obj = {
		.a = 0x11u,
		.b = 0x22334455u,
		.c = 0x6677u
	};

	std::array<uint8_t, sizeof(PaddedObject)> expected{};
	memcpy(expected.data(), &obj, sizeof(PaddedObject));

	prosVector_insert(&v, &obj, 0);
	memset(&obj, 0x00, sizeof(PaddedObject));

	EXPECT_EQ(memcmp(expected.data(), prosVector_getData(&v), sizeof(PaddedObject)), 0);
}

TEST_F(ProtocolsUtilitiesVector, MetInsertIntoAnAlreadyOcuppiedIndexMovesObjects) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 3; i++)
		prosVector_pushBack(&v, &vl[i]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 3);

	prosVector_insert(&v, &vl[3], 0);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 4);

	int expectedValues[] = {3, 0, 1, 2};
	for (int i = 0; i < 3; i++)
		EXPECT_EQ(*static_cast<int *>(prosVector_getAt(&v, i)), expectedValues[i]) << "Iteration: " << i;
}

TEST_F(ProtocolsUtilitiesVector, MetInsertCallWithNullSelfParameterPanic) {
	int vl = 1;
	EXPECT_DEATH(
		{
			prosVector_insert(nullptr, &vl, 0);
		},
		""
	);
}

TEST_F(ProtocolsUtilitiesVector, MetInsertCallWithNullObjParameterPanic) {
	EXPECT_DEATH(
		{
			prosVector vext = prosVector_new(sizeof(int));
			ASSERT_TRUE(vext);
			prosVector_insert(&vext, nullptr, 0);
		},
		""
	);
}

/* Method prosVector_remove() */

TEST_F(ProtocolsUtilitiesVector, MetRemoveErasesElementAtTheSpecifiedIndex) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 4);

	EXPECT_TRUE(prosVector_remove(&v, 0));
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 3);

	EXPECT_TRUE(prosVector_remove(&v, 2));
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 2);

	EXPECT_TRUE(prosVector_remove(&v, 0));
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 1);

	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), 2);
}

TEST_F(ProtocolsUtilitiesVector, MetRemoveCallWithInvalidIndexParameterReturnsFalse) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl = 1;
	prosVector_pushBack(&v, &vl);

	EXPECT_FALSE(prosVector_remove(&v, 1));
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 1);

	/* On an empty vector. */
	ASSERT_TRUE(prosVector_popBack(&v)) << "Humm... Something went wrong.";
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 0);

	EXPECT_FALSE(prosVector_remove(&v, 0));
}

TEST_F(ProtocolsUtilitiesVector, MetRemoveCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			prosVector_remove(nullptr, 0);
		},
		""
	);
}

/* Method prosVector_getAt() */

TEST_F(ProtocolsUtilitiesVector, MetGetAtGetsReturnsAPointerToTheElementAtTheSpecifiedIndex) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1, 2, 3};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 4);

	int accessOrder[] = {3, 0, 2, 1};
	for (int i = 0; i < 4; i++) {
		EXPECT_EQ(
			*static_cast<int *>(prosVector_getAt(&v, accessOrder[i])),
			accessOrder[i]
		);
	};
}

TEST_F(ProtocolsUtilitiesVector, MetGetAtCallWithInvalidIndexParameterReturnsNullptr) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	EXPECT_FALSE(prosVector_getAt(&v, 0));

	// On an empty and non-zero capacity vector.
	int vl = 9;
	prosVector_pushBack(&v, &vl);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 1);

	EXPECT_FALSE(prosVector_getAt(&v, 1));
}

TEST_F(ProtocolsUtilitiesVector, MetGetAtCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			void *r = prosVector_getAt(nullptr, 0);
		},
		""
	);
}

/* Method prosVector_getLastObj() */

TEST_F(ProtocolsUtilitiesVector, MetGetLastObjReturnsAPointerToTheLastElementInTheVector) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	EXPECT_EQ(v->capacity, 0);
	EXPECT_EQ(v->size, 0);
	EXPECT_FALSE(prosVector_getLastObj(&v));

	int vl[] = {0, 1, 2, 3};

	prosVector_pushBack(&v, &vl[0]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 1);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[0]);

	prosVector_pushBack(&v, &vl[1]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 2);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[1]);

	prosVector_pushBack(&v, &vl[2]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 3);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[2]);

	prosVector_pushBack(&v, &vl[3]);
	EXPECT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(v->size, 4);
	EXPECT_EQ(*static_cast<int *>(prosVector_getLastObj(&v)), vl[3]);
}

TEST_F(ProtocolsUtilitiesVector, MetGetLastObjOnEmptyVectorReturnsFalse) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));
	ASSERT_EQ(v->capacity, 0);
	ASSERT_EQ(v->size, 0);

	EXPECT_FALSE(prosVector_getLastObj(&v));

	int vl = 5;
	prosVector_pushBack(&v, &vl);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE) << "To proceed, capacity must be a non-zero value";
	ASSERT_EQ(v->size, 1);
	prosVector_popBack(&v);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE) << "To proceed, capacity must be a non-zero value.";
	ASSERT_EQ(v->size, 0);

	EXPECT_FALSE(prosVector_getLastObj(&v));
}

TEST_F(ProtocolsUtilitiesVector, MetGetLastObjCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			void *r = prosVector_getLastObj(nullptr);
		},
		""
	);
}

/* Method prosVector_getData() */

TEST_F(ProtocolsUtilitiesVector, MetGetDataReturnsAliasToVectorObjectDataThatIsContiguos) {
	v = prosVector_new(sizeof(char));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(char));

	char vl[] = {'1', '2', '3', '\0'};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 4);
	for (int i = 0; i < 4; i++)
		EXPECT_EQ(*static_cast<char *>(prosVector_getAt(&v, i)), vl[i]);

	char (&p)[4] = *static_cast<char (*)[4]>(prosVector_getData(&v));
	p[0] = 'M';
	p[1] = 'e';
	p[2] = 'g';
	p[3] = '\0';

	char meg[] = "Meg";
	for (int i = 0; i < 4; i++)
		EXPECT_EQ(*static_cast<char *>(prosVector_getAt(&v, i)), meg[i]);
}

TEST_F(ProtocolsUtilitiesVector, MetGetDataOnEmptyVectorReturnsNullptr) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	/* On a with zero capacity vector. */
	EXPECT_FALSE(prosVector_getData(&v));

	// Turns the vector empty but with capacity.
	int vl = 5;
	prosVector_pushBack(&v, &vl);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE) << "To proceed, capacity must be a non-zero value";
	ASSERT_EQ(v->size, 1);
	prosVector_popBack(&v);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE) << "To proceed, capacity must be a non-zero value.";
	ASSERT_EQ(v->size, 0);

	/* On empty and non-zero capacity vector. */
	EXPECT_FALSE(prosVector_getData(&v));
}

TEST_F(ProtocolsUtilitiesVector, MetGetDataCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			void *r = prosVector_getData(nullptr);
		},
		""
	);
}

/* Method prosVector_getCapacity() */

TEST_F(ProtocolsUtilitiesVector, MetGetCapacityReturnsTheStorageCapacityOfTheVector) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));
	ASSERT_EQ(v->size, 0);

	/* `v` has no capacity yet. */
	EXPECT_EQ(prosVector_getCapacity(&v), 0);
	EXPECT_EQ(prosVector_getCapacity(&v), v->capacity);

	int vl[] = {0, 1, 2, 3, 4};
	for (int i = 0; i < 4; i++)
		prosVector_pushBack(&v, &vl[i]);
	ASSERT_EQ(v->size, 4) << "Humm... Something went wrong.";
	EXPECT_EQ(prosVector_getCapacity(&v), PROTOCOLS_INITIAL_VECTOR_SIZE);
	EXPECT_EQ(prosVector_getCapacity(&v), v->capacity);

	prosVector_pushBack(&v, &vl[4]);
	ASSERT_EQ(v->size, 5) << "Humm... Something went wrong.";
	EXPECT_EQ(prosVector_getCapacity(&v), PROTOCOLS_INITIAL_VECTOR_SIZE * 2);
	EXPECT_EQ(prosVector_getCapacity(&v), v->capacity);
}

TEST_F(ProtocolsUtilitiesVector, MetGetCapacityCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			uint32_t r = prosVector_getCapacity(&v);
		},
		""
	);
}

/* Method prosVector_getSize() */
TEST_F(ProtocolsUtilitiesVector, MetGetSizeRetunsTheCountOfElementsInTheVector) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	// Vector with no memory reserve, size is always zero.
	ASSERT_EQ(v->capacity, 0);
	ASSERT_EQ(v->size, 0);
	EXPECT_EQ(prosVector_getSize(&v), 0);

	// Now the vector has memory reserve and one object, size is 1.
	int vl = 10;
	prosVector_pushBack(&v, &vl);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 1);
	EXPECT_EQ(prosVector_getSize(&v), 1);

	// The vector has memory reserve but no objects, size is 0.
	prosVector_popBack(&v);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 0);
	EXPECT_EQ(prosVector_getSize(&v), 0);
}

TEST_F(ProtocolsUtilitiesVector, MetGetSizeCallWithNullsSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			uint32_t r = prosVector_getSize(nullptr);
		},
		""
	);
}

/* Method prosVector_end() */

TEST_F(ProtocolsUtilitiesVector, MetEndReturnsAPastTheEndPointerForTheVectorEvenThoughVectorChanges) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));

	int vl[] = {0, 1};
	void *data[2];

	prosVector_pushBack(&v, &vl[0]);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 1);
	EXPECT_EQ(
		data[0] = prosVector_end(&v),
		static_cast<char *>(prosVector_getData(&v)) + (v->size * v->typeSize)
	);

	prosVector_pushBack(&v, &vl[1]);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE);
	ASSERT_EQ(v->size, 2);
	EXPECT_EQ(
		data[1] = prosVector_end(&v),
		static_cast<char *>(prosVector_getData(&v)) + (v->size * v->typeSize)
	);

	EXPECT_NE(data[0], data[1]);
}

TEST_F(ProtocolsUtilitiesVector, MetEndOnEmptyVectorReturnsNullptr) {
	v = prosVector_new(sizeof(int));
	ASSERT_TRUE(v);
	ASSERT_EQ(v->typeSize, sizeof(int));
	ASSERT_EQ(v->capacity, 0);
	ASSERT_EQ(v->size, 0);

	/* On a with zero capacity vector. */
	EXPECT_FALSE(prosVector_end(&v));

	// Turns the vector empty but with capacity.
	int vl = 5;
	prosVector_pushBack(&v, &vl);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE) << "To proceed, capacity must be a non-zero value";
	ASSERT_EQ(v->size, 1);
	prosVector_popBack(&v);
	ASSERT_EQ(v->capacity, PROTOCOLS_INITIAL_VECTOR_SIZE) << "To proceed, capacity must be a non-zero value.";
	ASSERT_EQ(v->size, 0);

	/* On empty and non-zero capacity vector. */
	EXPECT_FALSE(prosVector_end(&v));
}

TEST_F(ProtocolsUtilitiesVector, MetEndCallWithNullSelfParameterPanics) {
	EXPECT_DEATH(
		{
			[[maybe_unused]]
			void *r = prosVector_end(nullptr);
		},
		""
	);
}

int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
