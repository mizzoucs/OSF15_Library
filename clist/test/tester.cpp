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

/*
    Test Set A - Basic functionality
    Create, import, export, destroy

clist_t *clist_create(const size_t data_type_size, void (*const destruct_func)(void *const))
    C1. Normal, valid size, NULL constructor
    C2. Normal, valid size, actual deconstructor
    C3. Fail, 0 size, NULL constructor
    C4. Fail, 0 size, actual constructor

void clist_destroy(clist_t *const clist)
    D1. Normal, empty, NULL deconstructor
    D2. Normal, empty, deconstructor
    D3. Normal, contents, NULL deconstructor
    D4. Normal, constent, deconstructor
    D5. Fail, NULL

clist_t *clist_import(const void *const data, const size_t count, const size_t data_type_size,
                        void (*destruct_func)(void *))
    I1. Normal, valid data ptr, valid count, valid data_size, NULL destruct
    I2. Normal, valid data ptr, valid count, valid data_size, actual destruct
    I3. Fail, NULL data, null destructor
    I4. Fail, NULL data, valid destructor
    I5. Fail, 0 count, null destructor
    I6. Fail, 0 count, valid destructor
    I7. Fail, 0 data_size, null destructor
    I8. Fail, 0 data_size, valid destructor

size_t clist_export(const clist_t *const clist, void *data_dest);
    E1. Normal, with contents
    E2. Normal, empty list (Not a failure?)
    E3. Fail, null list pointer
    E4. Fail, null data pointer

*/


TEST(clist_set_a, vital) {
    // If this doesn't pass, go cry, because you're not getting clist to work
    ASSERT_EQ(offsetof(node_t, prev), offsetof(clist_t, back));
    ASSERT_EQ(offsetof(node_t, next), offsetof(clist_t, front));
}


TEST(clist_set_a, create_destroy) {
    // create C1
    clist_t *clist_a = clist_create(sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, NULL);
    // destroy D1
    clist_destroy(clist_a);

    // create C2
    clist_a = clist_create(sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    // destroy D2
    clist_destroy(clist_a);

    // create C3
    ASSERT_EQ(clist_create(0, NULL), NULL);

    // create C4
    ASSERT_EQ(clist_create(0, &test_deconstruct), NULL);
}

TEST(clist_set_a, import_export_destroy) {
    int values[5] = {1, 2, 3, 4, 5};
    // we should get... 15 from the deconstructor

    // import I1
    clist_t *clist_a = clist_import(values, 5, sizeof(int), NULL);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    reset_deconstruct_val();
    // destroy D3
    clist_destroy(clist_a);
    ASSERT_EQ(get_deconstruct_val(), 0);

    // import I2
    clist_t *clist_a = clist_import(values, 5, sizeof(int), &test_deconstruct);
    ASSERT_NE(clist_a, NULL);
    ASSERT_EQ(clist_a->front, clist_a->back);
    ASSERT_EQ(clist_a, clist_a->front);
    ASSERT_EQ(clist_a->size, 0);
    ASSERT_EQ(clist_a->destructor, &test_deconstruct);
    reset_deconstruct_val();
    // destroy D4
    clist_destroy(clist_a);
    ASSERT_EQ(get_deconstruct_val(), 15);

    // destroy D5
    clist_destroy(NULL);
}

/*
clist_t *clist_import(const void *const data, const size_t count, const size_t data_type_size,
                        void (*destruct_func)(void *))
    I1. Normal, valid data ptr, valid count, valid data_size, NULL destruct
    I2. Normal, valid data ptr, valid count, valid data_size, actual destruct
    I3. Fail, NULL data, null destructor
    I4. Fail, NULL data, valid destructor
    I5. Fail, 0 count, null destructor
    I6. Fail, 0 count, valid destructor
    I7. Fail, 0 data_size, null destructor
    I8. Fail, 0 data_size, valid destructor

size_t clist_export(const clist_t *const clist, void *data_dest);
    E1. Normal, with contents
    E2. Normal, empty list (Not a failure?)
    E3. Fail, null list pointer
    E4. Fail, null data pointer
*/




int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}