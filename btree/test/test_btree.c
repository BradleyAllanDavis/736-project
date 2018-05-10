// For cmocka.
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "btree.h"
#include "btree.c"
#include "fs.h"

Btree *btree = NULL;
Btree *normal = NULL;
Btree *popular = NULL;

static int init() {
	srand(time(NULL));
	/* btree = btree_new("/users/kanwu/test-btree.dat"); */
	normal = btree_new("/mnt/ssd/normal.dat");
	popular = btree_new("/mnt/ssd/popular.dat");
	return 0;
}

static int shutdown() {
	btree_destroy(normal);
	btree_destroy(popular);
	return 0;
}

typedef struct {
	BtreeKey key;
	BtreeValue value;
} Item;

static int item_cmp(const void *a, const void *b) {
	return btree_key_cmp(((Item *) a)->key, ((Item *) b)->key);
}

static bool item_array_has_key(Item *items, size_t n_items, BtreeKey key) {
	for (size_t i = 0; i < n_items; i++) {
		if (items[i].key == key)
			return true;
	}
	return false;
}

typedef struct {
	Item *items;
	int n_items;
	int max_n_items;
} RetrievedItems;

static void retrieve_callback(BtreeKey key, BtreeValue value, void *context) {
	RetrievedItems *retrieved = context;
	assert_true(retrieved->n_items < retrieved->max_n_items);
	retrieved->items[retrieved->n_items].key = key;
	retrieved->items[retrieved->n_items].value = value;
	retrieved->n_items++;
}

static void test_set_walk() { // The B-tree has to be empty when this is run.
	enum { MAX_N_ITEMS = 10000 };

	// Create a sorted array of items with unique keys.
	int n_items = rand() % MAX_N_ITEMS + 1;
	Item *items = malloc(n_items * sizeof(*items));
	for (int i_item = 0; i_item < n_items; i_item++) {
		BtreeKey key;
		do {
			key = rand();
		} while (item_array_has_key(items, i_item, key));
		items[i_item].key = key;
		items[i_item].value = rand();
	}
	qsort(items, n_items, sizeof(*items), item_cmp);

	// Insert the items into the tree.
	for (int i_item = 0; i_item < n_items; i_item++)
		btree_set(btree, items[i_item].key, items[i_item].value, NULL, NULL);

	// Retrieve the items from the tree.
	RetrievedItems retrieved;
	retrieved.max_n_items = n_items;
	retrieved.n_items = 0;
	retrieved.items = malloc(retrieved.max_n_items * sizeof(*retrieved.items));
	btree_walk(btree, retrieve_callback, &retrieved);

	// Verify that we got the correct items in the correct order.
	assert_int_equal(retrieved.n_items, n_items);
	for (int i_item = 0; i_item < n_items; i_item++) {
		assert_true(retrieved.items[i_item].key == items[i_item].key);
		assert_true(retrieved.items[i_item].value == items[i_item].value);
	}

	free(items);
	free(retrieved.items);
}

static void test_set_get() {
	enum { N_ITEMS = 1000000 };

	for (int i_item = 0; i_item < N_ITEMS; i_item++) {
		BtreeKey key = rand();
		BtreeValue value = rand();
		btree_set(btree, key, value, NULL, NULL);

		BtreeValue retrieved_value;
		bool value_exists = btree_get(btree, key, &retrieved_value);
		assert_true(value_exists);
		assert_true(retrieved_value == value);
	}
}

static void normal_workload()
{
	int num_insertions = 1000000;
	int num_searches = 1000;

        printf("\n################################################################\n");
        printf("# Run Info NORMAL\n");
        printf("BTREE_BLOCK_SIZE: %d\n", BTREE_BLOCK_SIZE);
        printf("size(BtreeKey): %ld bits\n", sizeof(BtreeKey) * 8);
        printf("size(BtreeValue): %ld bits\n", sizeof(BtreeValue) * 8);
        printf("######################################\n");
        printf("BTREE_MAX_POSSIBLE_KEYS: %d\n", BTREE_MAX_POSSIBLE_KEYS);
        printf("BTREE_MIN_KEYS: %d\n", BTREE_MIN_KEYS);
        printf("BTREE_MAX_KEYS: %d\n", BTREE_MAX_KEYS);
        printf("BTREE_MIN_CHILDREN: %d\n", BTREE_MIN_CHILDREN);
        printf("BTREE_MAX_CHILDREN: %d\n", BTREE_MAX_CHILDREN);
        printf("################################################################\n");

	///////////////////////// generate random keys /////////////////////////
        Item *items = malloc(num_insertions * sizeof(*items));
        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen("/users/kanwu/project/data/unique_random_keys", "r");
        for (int i_item = 0; i_item < num_insertions; i_item++) {
		BtreeKey key;
                read = getline(&line, &len, fp);
                key = atoi(line);
                items[i_item].key = key;
		items[i_item].value = rand();
		/* printf("%d", key); */
	}
        fclose(fp);

        ////////////////////////// fill the tree //////////////////////////////
        printf("\n################################################################\n");
        printf("# Insert\n");
        printf("num_insertions: %d\n", num_insertions);
        clock_t start = clock();
	for (int i_item = 0; i_item < num_insertions; i_item++)
		btree_set(normal, items[i_item].key, items[i_item].value, NULL, NULL);
        clock_t diff = clock() - start;
        double milsecs = ((double)diff) * 1000 / CLOCKS_PER_SEC;
        printf("Insertion taken %f (msec)\n", milsecs);
        printf("TPS: %f\n", num_insertions/(float)milsecs*1000);
        printf("######################################\n");
        /* printf("######################################\n"); */
	fs_print(normal->file);
	fs_clear(normal->file); // Clear read and write counts

	///////////////////////// get pareto keys /////////////////////////
        Item *pareto_items = malloc(num_insertions * sizeof(*items));
        fp = fopen("/users/kanwu/project/data/search_querylog", "r");
        for (int i_item = 0; i_item < num_insertions; i_item++) {
		BtreeKey key;
                read = getline(&line, &len, fp);
                key = atoi(line);
                pareto_items[i_item].key = key;
		pareto_items[i_item].value = rand();
	}
        fclose(fp);

        ////////////////////////// search the tree //////////////////////////
        printf("\n################################################################\n");
        printf("# Search\n");
        printf("num_searches: %d\n", num_searches);
        system("/users/kanwu/project/btree/scripts/clear_page_cache.sh");
        BtreeValue retrieved_value;
        system("blktrace -d /dev/nvme0n1p4 -o - | blkparse -i - >> ./blktrace.output &");
        /* system("iostat -x nvme0n1p4 1 100 > ./iostat.output &"); */
        diff = 0;
        for (int i_item = 0; i_item < num_searches; i_item++) {
                system("/users/kanwu/project/scripts/clear_page_cache.sh");
                start = clock();
                btree_get(normal, pareto_items[i_item].key, &retrieved_value);
                diff += clock() - start;
		if (i_item == (num_searches - 1)) {
			system("pkill blktrace");
			system("pkill blkparse");
			system("sync");
			fs_set_last_one(normal->file);
		}
	}
        milsecs = ((double)diff) * 1000 / CLOCKS_PER_SEC;
        printf("Searching taken %f (msec)\n", milsecs);
        printf("TPS: %f\n", (num_searches/(float)milsecs)*1000);
        printf("######################################\n");
	btree_print(normal, stdout); // Print highest level (height)
	fs_print(normal->file);
        printf("\n");
        printf("Total searching taken %f (msec)\n", milsecs);
}

static void pareto_workload()
{
	int num_insertions = 1000000;
	int num_searches = 1000;

        printf("\n################################################################\n");
        printf("# Run Info PARETO\n");
        printf("BTREE_BLOCK_SIZE: %d\n", BTREE_BLOCK_SIZE);
        printf("size(BtreeKey): %ld bits\n", sizeof(BtreeKey) * 8);
        printf("size(BtreeValue): %ld bits\n", sizeof(BtreeValue) * 8);
        printf("######################################\n");
        printf("BTREE_MAX_POSSIBLE_KEYS: %d\n", BTREE_MAX_POSSIBLE_KEYS);
        printf("BTREE_MIN_KEYS: %d\n", BTREE_MIN_KEYS);
        printf("BTREE_MAX_KEYS: %d\n", BTREE_MAX_KEYS);
        printf("BTREE_MIN_CHILDREN: %d\n", BTREE_MIN_CHILDREN);
        printf("BTREE_MAX_CHILDREN: %d\n", BTREE_MAX_CHILDREN);
        printf("################################################################\n");

	///////////////////////// generate random keys /////////////////////////
        Item *items = malloc(num_insertions * sizeof(*items));
        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen("/users/kanwu/project/data/unique_random_keys", "r");
        for (int i_item = 0; i_item < num_insertions; i_item++) {
		BtreeKey key;
                read = getline(&line, &len, fp);
                key = atoi(line);
                items[i_item].key = key;
		items[i_item].value = rand();
	}
        fclose(fp);

        ////////////////////////// fill the tree //////////////////////////////
        printf("\n################################################################\n");
        printf("# Insert\n");
        printf("num_insertions: %d\n", num_insertions);
        clock_t start = clock();
	for (int i_item = 0; i_item < num_insertions; i_item++) {
		if (items[i_item].key < 2000)
			btree_set(popular, items[i_item].key, items[i_item].value, NULL, NULL);
		btree_set(normal, items[i_item].key, items[i_item].value, NULL, NULL);
	}
        clock_t diff = clock() - start;
        double milsecs = ((double)diff) * 1000 / CLOCKS_PER_SEC;
        printf("Insertion taken %f (msec)\n", milsecs);
        printf("TPS: %f\n", num_insertions/(float)milsecs*1000);
        printf("######################################\n");
        /* printf("######################################\n"); */
	fs_print(normal->file);
	fs_clear(normal->file); // Clear read and write counts
	fs_print(popular->file);
	fs_clear(popular->file); // Clear read and write counts

	///////////////////////// get pareto keys /////////////////////////
        Item *pareto_items = malloc(num_insertions * sizeof(*items));
        fp = fopen("/users/kanwu/project/data/search_querylog", "r");
        for (int i_item = 0; i_item < num_insertions; i_item++) {
		BtreeKey key;
                read = getline(&line, &len, fp);
                key = atoi(line);
                pareto_items[i_item].key = key;
		pareto_items[i_item].value = rand();
	}
        fclose(fp);

        ////////////////////////// search the tree //////////////////////////
        printf("\n################################################################\n");
        printf("# Search\n");
        printf("num_searches: %d\n", num_searches);
        system("/users/kanwu/project/btree/scripts/clear_page_cache.sh");
        BtreeValue retrieved_value;
        system("blktrace -d /dev/nvme0n1p4 -o - | blkparse -i - >> ./blktrace.output &");
        /* system("iostat -x nvme0n1p4 1 100 > ./iostat.output &"); */
	diff = 0;
        clock_t normal_diff = 0;
	clock_t popular_diff = 0;
	int num_searches_normal = 0;
	int num_searches_popular = 0;
        for (int i_item = 0; i_item < num_searches; i_item++) {
                system("/users/kanwu/project/scripts/clear_page_cache.sh");
                start = clock();
		if (i_item < 800) {
			btree_get(popular, pareto_items[i_item].key, &retrieved_value);
			num_searches_popular++;
			popular_diff += clock() - start;
		} else {
			btree_get(normal, pareto_items[i_item].key, &retrieved_value);
			num_searches_normal++;
			normal_diff += clock() - start;
		}
		diff += clock() - start;
		if (i_item == (num_searches - 1)) {
			system("pkill blktrace");
			system("pkill blkparse");
			system("sync");
			fs_set_last_one(normal->file);
			fs_set_last_one(popular->file);
		}
	}
        milsecs = ((double)diff) * 1000 / CLOCKS_PER_SEC;
        printf("Searching taken %f (msec)\n", milsecs);
        printf("TPS: %f\n", (num_searches/(float)milsecs)*1000);
        printf("################################################################\n");

        printf("\n################################################################\n");
        printf("# Popular\n");
        printf("num_searches: %d\n", num_searches_popular);
        double popular_milsecs = ((double)popular_diff) * 1000 / CLOCKS_PER_SEC;
        printf("Searching taken %f (msec)\n", popular_milsecs);
        printf("TPS: %f\n", (num_searches_popular/(float)popular_milsecs)*1000);
        printf("######################################\n");
	btree_print(popular, stdout); // Print highest level (height)
	fs_print(popular->file);

        printf("\n################################################################\n");
        printf("num_searches: %d\n", num_searches_normal);
        printf("# Normal\n");
        double normal_milsecs = ((double)normal_diff) * 1000 / CLOCKS_PER_SEC;
        printf("Searching taken %f (msec)\n", normal_milsecs);
        printf("TPS: %f\n", (num_searches_normal/(float)normal_milsecs)*1000);
        printf("######################################\n");
	btree_print(normal, stdout); // Print highest level (height)
	fs_print(normal->file);
        printf("\n");
        printf("Total searching taken %f (msec)\n", popular_milsecs + normal_milsecs);
}

int main(void) {
	const struct CMUnitTest tests[] = {
		//cmocka_unit_test(test_set_walk),
		//cmocka_unit_test(test_set_get),

		/* cmocka_unit_test(normal_workload), */
		cmocka_unit_test(pareto_workload),
	};

	return cmocka_run_group_tests(tests, init, shutdown);
}



