//set associative cache
#ifndef SACACHE_H
#define SACACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BLK_SIZE 64 //bytes

struct Way{
	unsigned int *valid;
	unsigned int *dirty;
	unsigned long *tag;
	unsigned int *age;
};

struct SACache{
	struct Way *ways;

	unsigned int num_ways;
	unsigned int num_sets;	//sets = size/(num_ways * block_size)

	unsigned long addr_mask;
	unsigned long index_bits;
	unsigned long offset_bits;

	unsigned int hits;
	unsigned int read_misses;
	unsigned int write_misses;  

	unsigned int evictions;

	unsigned int clock;
};

extern struct SACache * l2cache;

struct SACache * create_sacache (unsigned int capacity, unsigned int nways);
unsigned int lookup_sacache (struct SACache *sacache, unsigned long address, unsigned int rw, unsigned long *dram_wb_addr);	//rw = 0 for read, 1 for write

unsigned int flush_sacache(struct SACache *sacache);

unsigned int get_hits(struct SACache *sacache);

unsigned int clear_hits(struct SACache *sacache);

// read misses + write_misses
unsigned int get_misses(struct SACache *sacache);

unsigned int get_read_misses(struct SACache *sacache);
unsigned int get_write_misses(struct SACache *sacache);
unsigned int clear_misses(struct SACache *sacache);

unsigned int destroy_sacache(struct SACache *sacache);

#endif
