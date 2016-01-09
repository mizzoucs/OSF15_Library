#include <stdexcept>
#include "../src/clist.c"

#include <gtest/gtest.h>

int deconstructor = 0;

void test_deconstruct(void *data) {
    if (data)
        deconstructor += *((int *) data);
    else
        throw std::bad_argument("Destructor given NULL (but that should be impossible???");
}

int get_deconstruct_val() {
    return deconstructor;
}

void reset_deconstruct_val() {
    deconstructor = 0;
}

TEST(clist_vitals, link_offsets {
    // If this doesn't pass, go cry, because you're not getting clist
    ASSERT_EQ(offsetof(node_t, prev), offsetof(clist_t, back));
    ASSERT_EQ(offsetof(node_t, next), offsetof(clist_t, front));
}


/*
clist_t *clist_create(const size_t data_type_size, void (*const destruct_func)(void *const))
    1. Normal, valid size, NULL constructor
    2. Normal, valid size, actual deconstructor
    3. Fail, 0 size, NULL constructor
    4. Fail, 0 size, actual constructor
*/

TEST(clist_create, normal_use) {
    clist_t *clist_a = clist_create(sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, NULL);
    clist_destroy(clist_a);

    clist_a = clist_create(sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    clist_destroy(clist_a);
}

TEST(clist_create, zero_size) {
    ASSERT_EQ(clist_create(0, NULL), NULL);
    ASSERT_EQ(clist_create(0, &test_deconstruct), NULL);
}



/*
clist_t *clist_import(const void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *))
	1. Normal, valid data ptr, valid count, valid data_size, NULL destruct
	2. Normal, valid data ptr, valid count, valid data_size, actual destruct
	3. Fail, NULL data, null destructor
	4. Fail, NULL data, valid destructor
	5. Fail, 0 count, null destructor
	6. Fail, 0 count, valid destructor
	7. Fail, 0 data_size, null destructor
	8. Fail, 0 data_size, valid destructor

	// tests to break malloc with reguards to size?
	// Eh?
*/

TEST(clist_import, normal_use) {
    int values[5] = {1, 2, 3, 4, 5};
    // we should get... 15 from the deconstructor

    // 1
    clist_t *clist_a = clist_import(values, 5, sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    reset_deconstruct_val();
    clist_destroy(clist_a);
    ASSERT_EQ(get_deconstruct_val(), 0);

    // 2
    clist_t *clist_a = clist_import(values, 5, sizeof(int), &test_deconstruct);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    reset_deconstruct_val();
    clist_destroy(clist_a);
    ASSERT_EQ(get_deconstruct_val(), 15);
}

/*
size_t clist_export(const clist_t *const clist, void *data_dest);
	1. Normal, with contents
	2. Normal, empty list (Not a failure?)
	3. Fail, null list pointer
	4. Fail, null data pointer
*/
TEST(clist_import, normal_use) {
    int values[5] = {1, 2, 3, 4, 5};
    // we should get... 15 from the deconstructor

    // 1
    clist_t *clist_a = clist_import(values, 5, sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    reset_deconstruct_val();
    clist_destroy(clist_a);
    ASSERT_EQ(get_deconstruct_val(), 0);

    // 2
    clist_t *clist_a = clist_import(values, 5, sizeof(int), &test_deconstruct);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    reset_deconstruct_val();
    clist_destroy(clist_a);
    ASSERT_EQ(get_deconstruct_val(), 15);
}




int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}