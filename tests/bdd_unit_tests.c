#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/hooks.h>
#include <stdio.h>
#include <math.h>

#include "const.h"
#include "image.h"
#include "test_help/test_help.h"

// https://github.com/codewars/codewars-runner-cli/blob/master/frameworks/c/criterion.c
ReportHook(TEST_CRASH)(struct criterion_test_stats *stats) {
    printf("Test [%s] Crashed\n", stats->test->name);
}

static BDD_NODE test_bdd_nodes[BDD_NODES_MAX];

/*
 * Function which makes a copy of the bdd_nodes
 * this is so that we can check against it after
 * completing an operation
 */
static void copy_from_user() {
	for (int i = BDD_NUM_LEAVES; i < BDD_NODES_MAX; i++) {
		test_bdd_nodes[i] = bdd_nodes[i];
	}
}

/* 
 * Function that tests that two bdds have the same levels at every node
 * and that they have matching leaves at level 0
 * 
 * NOTE: you must copy into test_bdd_nodes before calling this function
 */
static bool bdd_equality(BDD_NODE *user, BDD_NODE *test) {
	if (user->level != test->level) {
		// printf("returning false because levels unequal\n");
		return false;
	}
	BDD_NODE * user_l = &bdd_nodes[user->left];
	BDD_NODE * user_r = &bdd_nodes[user->right];
	BDD_NODE * test_l = &test_bdd_nodes[test->left];
	BDD_NODE * test_r = &test_bdd_nodes[test->right];

	if (user_l->level > 0 && test_l->level > 0) {
		if (bdd_equality(user_l, test_l) == false)
			return false;
	} else if (user->left != test->left) { // one or both is level 0
		// printf("returning false because %d /= %d on left\n", user->left, test->left);
		return false;
	}
	if (user_r->level > 0 && test_r->level > 0) {
		if (bdd_equality(user_r, test_r) == false)
			return false;
	} else if (user->right != test->right) { // one or both is level 0
		// printf("returning false because %d /= %d on right\n", user->right, test->right);
		return false;
	}
	return true;
}

/* 
 * Helper function for creating a 8 x 8 raster 
 * with numbers 0-63
 */
static void init_test_raster(unsigned char test_raster[64]) {
	for (int i = 0; i < 64; i++) {
		test_raster[i] = i;
	}
}

/* 
 * Helper function for debugging only
 */
static void print_bdd(BDD_NODE *root, int rows, int cols) {
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
			printf("%d ", bdd_apply(root, row, col));
		}
		printf("\n");
	}
}

static void populate_global_raster() {
	for (int i = 0; i < RASTER_SIZE_MAX; i+=2)
		raster_data[i] = 0xAB;
}

static int check_global_raster() {
	for (int i = 0; i < RASTER_SIZE_MAX; i+=2) {
		if (raster_data[i] != 0xAB)
			return -1;
	}
	return 0;
}

static int min_bdd_level(int h, int w) {
    int l;
    for(l = 0; (1 << (l/2)) < w || (1 << (l/2)) < h; l++)
	;
    return l;
}

/*
 * Tests the bdd_apply function on a 4x4 bdd
 * Tests: bdd_apply
 */
Test(unit_test_suite, bdd_apply_test, .timeout=5) {
	unsigned char test_raster[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	BDD_NODE *root = TEST_bdd_from_raster(4, 4, test_raster);

	cr_assert_not_null(root, "Root is NULL");

	for(int row = 0; row < 4; row++) {
		for(int col = 0; col < 4; col++) {
			unsigned char pixel_val = bdd_apply(root, row, col);
			unsigned char exp_val = test_raster[row * 4 + col];
			cr_assert_eq(pixel_val, exp_val, "Wrong pixel value at [%d][%d]. Got: %d | Expected: %d",
		 		row, col, pixel_val, exp_val);
		}
	}
}

/*
 * Construct a bdd from a simple 4x4 raster and verifies that
 * it is correct
 * Tests: bdd_from_raster
 */
Test(unit_test_suite, bdd_raster_test, .timeout=5) {
	unsigned char test_raster[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	
	BDD_NODE *test_root = TEST_bdd_from_raster(4, 4, test_raster);

	// copy the data from the bdd_nodes and then completely reset the system
	copy_from_user();
	test_root = (test_root - bdd_nodes) + test_bdd_nodes;
	TEST_bdd_reset();

	BDD_NODE *user_root = bdd_from_raster(4, 4, test_raster);

	cr_assert_eq(bdd_equality(user_root, test_root), true, "bdd created from raster is not correct");
}

/*
 * Rotate the bdd 4 times. This should result in the
 * exact same bdd at the end.
 * Tests: bdd_rotate
 */
Test(unit_test_suite, bdd_rotate_360_test, .timeout=5) {
	unsigned char test_raster[64];
	init_test_raster(test_raster);

	// here we use the student's bdd_from_raster because
	// they may have globals which they need for correctly
	// performing the manipulations
	BDD_NODE *root = TEST_bdd_from_raster(8, 8, test_raster);
	int root_ind = root - bdd_nodes;

	// make a copy of the bdd
	copy_from_user();
	BDD_NODE *old_root = test_bdd_nodes + root_ind;

	// run the rotate function 4 times
	populate_global_raster();
	root = bdd_rotate(root, root->level);
	cr_assert_neq(bdd_equality(root, old_root), true, "Rotated should not be equal");
	
	root = bdd_rotate(root, root->level);
	root = bdd_rotate(root, root->level);
	root = bdd_rotate(root, root->level);
	cr_assert_eq(bdd_equality(root, old_root), true, "Rotating 4 times must restore original bdd");
	cr_assert_eq(check_global_raster(), 0, "raster_data was modified");
}

/*
 * Apply a function via map which rounds each value to nearest 10
 * Check that result is correct
 */
Test(unit_test_suite, bdd_map_contrast_test, .timeout=5) {
	unsigned char test_raster[16] = {0,3,11,14,52,48,13,98,57,15,9,87,37,27,11,76};
	unsigned char exp_raster[16] = {0,0,10,10,50,50,10,100,60,20,10,90,40,30,10,80};
	
	// here we use the student's bdd_from_raster because
	// they may have globals which they need for correctly
	// performing the manipulations
	BDD_NODE *root = TEST_bdd_from_raster(4, 4, test_raster);
	
	
	// contrast function
	unsigned char round_to_ten(unsigned char input) {
		if (input % 10 >= 5) {
			return (input / 10 + 1) * 10;
		}
		return (input / 10) * 10;
	}

	populate_global_raster();

	root = bdd_map(root, round_to_ten);
	BDD_NODE *exp_root = TEST_bdd_from_raster(4, 4, exp_raster);
	copy_from_user();
	exp_root = (exp_root - bdd_nodes) + test_bdd_nodes;
	cr_assert_eq(bdd_equality(root, exp_root), true, "bdd after map does not match expected");
	cr_assert_eq(check_global_raster(), 0, "raster_data was modified");
}

/*
 * Construct a BDD from M.pgm,
 * zoom in then out to see if
 * the original BDD and zoomed-in-out BDD are equal
 */
 
Test(unit_test_suite, zoom_in_out_test, .timeout=5) {
    int w, h;
    unsigned char test_raster[1000]; // RASTER_SIZE_MAX caused crashing
    FILE *in = fopen("tests/rsrc/M.pgm", "r");

    img_read_pgm(in, &w, &h, test_raster, sizeof(test_raster));
    
    fclose(in);
    BDD_NODE *root = TEST_bdd_from_raster(w, h, test_raster);

    int root_ind = root - bdd_nodes;
	copy_from_user();
	BDD_NODE *old_root = test_bdd_nodes + root_ind;

	populate_global_raster();
    root = bdd_zoom(root, min_bdd_level(w, h), 1);
    cr_assert_neq(bdd_equality(root, old_root), true, "Zooming in should not be equal");
    root = bdd_zoom(root, min_bdd_level(w * 2, h * 2), -1);
    cr_assert_eq(bdd_equality(root, old_root), true, "Zooming in then out must restore original bdd");
	cr_assert_eq(check_global_raster(), 0, "raster_data was modified");
}

Test(unit_test_suite, bdd_serialize_twice, .timeout=5) {
	unsigned char test_raster[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	BDD_NODE *root = TEST_bdd_from_raster(4, 4, test_raster);

	cr_assert_not_null(root, "Root is NULL");

    FILE *out = fopen("test_output/serialize.1.birp", "w+");
    int ret = bdd_serialize(root, out);
    fflush(out);
    cr_assert_eq(ret, 0);

    out = fopen("test_output/serialize.2.birp", "w+");
    ret = bdd_serialize(root, out);
    fflush(out);
    cr_assert_eq(ret, 0);


	char *cmp = "cmp test_output/serialize.1.birp test_output/serialize.2.birp";

	ret = WEXITSTATUS(system(cmp));
	cr_assert_eq(ret, EXIT_SUCCESS, "bdd_serialize did not produce the same file between two calls");
}

Test(unit_test_suite, bdd_lookup_retrieval, .timeout=5) {
    int bdd_node_indices[6];

    // creation lookups
    bdd_node_indices[0] = bdd_lookup(1, 0, 10);
    cr_assert_geq(bdd_node_indices[0], BDD_NUM_LEAVES, "Node not created");

    bdd_node_indices[1] = bdd_lookup(1, 255, 127);
    cr_assert_gt(bdd_node_indices[1], BDD_NUM_LEAVES, "Node not created");
    cr_assert_neq(bdd_node_indices[0], bdd_node_indices[1], "Node not created; got previous node");

    bdd_node_indices[2] = bdd_lookup(2, bdd_node_indices[0], bdd_node_indices[1]);
    cr_assert_gt(bdd_node_indices[2], BDD_NUM_LEAVES, "Node not created");
    cr_assert_neq(bdd_node_indices[0], bdd_node_indices[2], "Node not created; got child node");
    cr_assert_neq(bdd_node_indices[1], bdd_node_indices[2], "Node not created; got child node");


    // retrieval lookups
    bdd_node_indices[3] = bdd_lookup(1, 0, 10);
    cr_assert_eq(bdd_node_indices[0], bdd_node_indices[3], "Node retrieval did not return previously created node");

    bdd_node_indices[4] = bdd_lookup(1, 255, 127);
    cr_assert_eq(bdd_node_indices[1], bdd_node_indices[4], "Node retrieval did not return previously created node");

    bdd_node_indices[5] = bdd_lookup(2, bdd_node_indices[0], bdd_node_indices[1]);
    cr_assert_eq(bdd_node_indices[2], bdd_node_indices[5], "Node retrieval did not return previously created node");
}

Test(unit_test_suite, bdd_lookup_skip, .timeout=5) {
    int bdd_node_indices[3];

    // skip lookup
    bdd_node_indices[0] = bdd_lookup(1, 0, 0);
    bdd_node_indices[1] = bdd_lookup(1, 10, 10);
    bdd_node_indices[2] = bdd_lookup(1, 203, 203);

    cr_assert_eq(bdd_node_indices[0], 0, "Node with same children should return the child");
    cr_assert_eq(bdd_node_indices[1], 10, "Node with same children should return the child");
    cr_assert_eq(bdd_node_indices[2], 203, "Node with same children should return the child");
}

Test(unit_test_suite, bdd_lookup_hash_stress, .timeout=10) {
	int i, j, k;
	// fill to < 50% capacity of bdd_nodes
	// aka < 25% of hash map
	// n == 128
	int n = (int)sqrt((double)(BDD_NODES_MAX >> 1) / BDD_LEVELS_MAX);

	int test_nodes[BDD_NODES_MAX];
	int test_node_index = 0;

	// fill hash map
	for (i = 0; i < BDD_LEVELS_MAX; i++)
		for (j = 0; j < n; j++)
			for (k = 0; k < n; k++)
				test_nodes[test_node_index++] = bdd_lookup(i, j, k);

	// sanity check on some random nodes
	// note that every n+1 nodes will return a leaf node
	cr_assert_geq(test_nodes[1], BDD_NUM_LEAVES, "bdd_lookup did not create a valid node");
	cr_assert_geq(test_nodes[n], BDD_NUM_LEAVES, "bdd_lookup did not create a valid node");
	cr_assert_geq(test_nodes[n*11], BDD_NUM_LEAVES, "bdd_lookup did not create a valid node");
	cr_assert_geq(test_nodes[n*15 + 22], BDD_NUM_LEAVES, "bdd_lookup did not create a valid node");

	// retrieve from hash map
	test_node_index = 0;
	for (i = 0; i < BDD_LEVELS_MAX; i++)
		for (j = 0; j < n; j++)
			for (k = 0; k < n; k++) {
				int ret = bdd_lookup(i, j, k);
				cr_assert_eq(ret, test_nodes[test_node_index++], "bdd_lookup did not retrieve the same node");
			}
}

/**
 * deserialize the textual representation and serialize bdd node tree back to textual representation, the results should be exact same
 */
Test(unit_test_suite, bdd_serialize, .timeout=5) {  // suite_name, test_name
    int w, h, max;
    FILE *in = fopen("tests/rsrc/M.birp", "r") ;
    BDD_NODE *node;
    node = img_read_birp(in, &w, &h) ;

    FILE *out = fopen("/tmp/tmp-serialize", "w+") ;
    int ret = bdd_serialize(node, out) ;
    fflush(out) ;
    cr_assert_eq(ret, 0);

    // compare out and in, in == out
    rewind(in) ;
    fscanf(in, "B5 ");
    fscanf(in, "%d %d %d", &w, &h, &max);
    fgetc(in) ;
    rewind(out) ;

    char a, b ;
    while ( !feof(in) && !feof(out) ) {
        a = fgetc(in) ;
        b = fgetc(out) ;
        if (a != b) {
            cr_assert_eq(1, 0) ;
            break ;
        }
    }

    if ( !(feof(in) && feof(out)) ) {
        cr_assert_eq(1, 0) ;
    }
}
