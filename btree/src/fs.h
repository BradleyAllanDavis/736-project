// Random access file abstraction layer.
#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef uint64_t FsOffset;

typedef struct FsFile FsFile;

typedef struct {
	uint64_t n_reads;
	clock_t read_time;

	uint64_t n_writes;
	clock_t write_time;
} FsStats;

FsFile *fs_open(const char *name, bool truncate);
void fs_set_last_one(FsFile *file);
void fs_set_read(FsFile *file, clock_t val);
void fs_clear(FsFile *file);
void fs_print(FsFile *file);
void fs_close(FsFile *file);

void fs_set_size(FsFile *file, FsOffset size);

void fs_read(FsFile *file, void *dest, FsOffset offset, size_t n_bytes);
void fs_write(FsFile *file, const void *src, FsOffset offset, size_t n_bytes);

FsStats fs_stats(FsFile *file);
