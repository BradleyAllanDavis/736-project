#include "fs.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "xassert.h"

struct FsFile { // The typedef is in fs.h.
	FILE *file;
	FsOffset size;
	FsStats stats;
	bool last_one;
};

FsFile *fs_open(const char *name, bool truncate) {
	FsFile *file = malloc(sizeof(*file));
	xassert(1, file != NULL);

	file->stats.n_reads = 0;
	file->stats.read_time = 0;
	file->stats.n_writes = 0;
	file->stats.write_time = 0;

	printf("=== to open %s\n", name);
        file->file = fopen(name, truncate ? "w+b" : "a+b");
	xassert(1, file->file != NULL);

	struct stat file_stat;
	int fstat_result = fstat(fileno(file->file), &file_stat);
	xassert(1, fstat_result != -1);
	file->size = file_stat.st_size;

	file->last_one = false;

	return file;
}

void fs_set_last_one(FsFile *file)
{
	file->last_one = true;
}

void fs_set_read(FsFile *file, clock_t val)
{
	file->stats.read_time += val;
}

void fs_clear(FsFile *file)
{
        /* printf("fs_clear()\n"); */
	file->stats.n_reads = 0;
	file->stats.read_time = 0;
	file->stats.n_writes = 0;
	file->stats.write_time = 0;
}

void fs_print(FsFile *file)
{
        printf("node_reads: %ld\n", file->stats.n_reads);
        double msec = ((double)file->stats.read_time) * 1000 / CLOCKS_PER_SEC;
        printf("node_read_time: %f (msec)\n", msec);
        /* printf("num_writes: %ld\n", file->stats.n_writes); */
        /* msec = ((double)file->stats.write_time) * 1000 / CLOCKS_PER_SEC; */
        /* printf("write_time: %f (msec)\n", msec); */
        printf("################################################################\n");
}

void fs_close(FsFile *file) {
	xassert(1, file != NULL);
	fclose(file->file);
	free(file);
}

void fs_set_size(FsFile *file, FsOffset size) {
	xassert(1, file != NULL);

	int ftruncate_result = ftruncate(fileno(file->file), size);
	xassert(1, ftruncate_result != -1);
	file->size = size;
}

void fs_read(FsFile *file, void *dest, FsOffset offset, size_t n_bytes) {
	xassert(1, file != NULL);
	xassert(1, offset < file->size);

	clock_t start = clock();
	ssize_t pread_result = pread(fileno(file->file), dest, n_bytes, offset);
        clock_t diff = clock() - start;

	if (file->last_one == false) {
		file->stats.n_reads++;
		file->stats.read_time += diff;
	}

	xassert(1, (size_t) pread_result == n_bytes);
	/* printf("fs_read() start:\t%ld\tend:%ld\n", start, clock()); */
}

void fs_write(FsFile *file, const void *src, FsOffset offset, size_t n_bytes) {
	xassert(1, file != NULL);
	xassert(1, offset < file->size);

	file->stats.n_writes++;

	/* clock_t start = clock(); */
	ssize_t pwrite_result = pwrite(fileno(file->file), src, n_bytes, offset);
        /* clock_t diff = clock() - start; */
	/* file->stats.write_time += diff; */
	xassert(1, (size_t) pwrite_result == n_bytes);
}

FsStats fs_stats(FsFile *file) {
	return file->stats;
}
