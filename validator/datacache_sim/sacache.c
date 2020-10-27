// A simple set associative cache implementation.
// Used to attach to the validator for simple cache statistics.
#ifndef SACACHE_C
#define SACACHE_C

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sacache.h"

struct SACache * create_sacache (unsigned int capacity, unsigned int nways)	//capacity in bytes
{
	unsigned int i;

	struct SACache *sacache = (struct SACache *) calloc(1, sizeof(struct SACache));

	sacache->ways = (struct Way *) calloc(nways, sizeof(struct Way));

	sacache->num_sets = ((capacity / BLK_SIZE) / nways);
	sacache->num_ways = nways;


	for(i=0; i<nways; i++)
	{
		sacache->ways[i].valid = (unsigned int *) calloc(sacache->num_sets, sizeof(unsigned int));
		sacache->ways[i].dirty = (unsigned int *) calloc(sacache->num_sets, sizeof(unsigned int));
		sacache->ways[i].tag   = (unsigned long *) calloc(sacache->num_sets, sizeof(unsigned long));
		sacache->ways[i].age   = (unsigned int *) calloc(sacache->num_sets, sizeof(unsigned int));
	}

	sacache->addr_mask = sacache->num_sets - 1;
	sacache->index_bits = (unsigned long) (log(sacache->num_sets)/log(2));
	sacache->offset_bits = (unsigned long) (log(BLK_SIZE)/log(2));

	sacache->hits = 0;
	sacache->read_misses = 0;
	sacache->write_misses = 0;	

	sacache->evictions = 0;

	sacache->clock = 0;

	return sacache;
}


unsigned int lookup_sacache (struct SACache *sacache, unsigned long address, unsigned int rw, unsigned long *dram_wb_addr)	//rw = 0 for read, 1 for write
{
  //unsigned long address_backup = address;
	unsigned int index, i, victim;
	unsigned long tag;
	unsigned long addr = address >> sacache->offset_bits;

	unsigned int age = 0x7ffffff;

	sacache->clock++;

	index = addr & sacache->addr_mask;
	tag   = addr >> sacache->index_bits;

	//unsigned int found = 0;
	for(i=0; i<sacache->num_ways; i++)
	{
		if(sacache->ways[i].valid[index] == 1)
		{
			if(sacache->ways[i].tag[index] == tag)
			{
				sacache->hits++;
				if(rw == 1)
				{
					sacache->ways[i].dirty[index] = 1;
					//sacache->ways[i].age[index] = sacache -> clock;
				}
				return 1;
			}
		}
	}

	//else
	{

	  if (rw == 1){
	    sacache -> write_misses++;
	  } else {
	    sacache -> read_misses++;
	  }
	  
	  //if (sacache == l2cache){
	  //printf("Inst %s missed on %lx", last_inst, address_backup);
	  //}

		//insert the entry in to the cache
		victim = 0;
		for(i=0; i<sacache->num_ways; i++)
		{	
			if(sacache->ways[i].valid[index] == 0)
			{
				sacache->ways[i].tag[index] = tag;
				sacache->ways[i].valid[index] = 1;
				sacache->ways[i].age[index] = sacache->clock;
				if(rw == 1)
				{
					sacache->ways[i].dirty[index] = 1;
				}
				else
				{
					sacache->ways[i].dirty[index] = 0;
				}

	  			//if (sacache == l2cache){
				//  printf("\n");
				//}				
				
				return 0;
			}
			else
			{
				if(sacache->ways[i].age[index] < age)
				{
					victim = i;
					age = sacache->ways[i].age[index];
				}
			}
		}
	}

	sacache->evictions++;

	unsigned long evictee_tag = sacache->ways[victim].tag[index];

	
	//if (sacache == l2cache){
	// unsigned long evicted_block = (evictee_tag << (sacache -> offset_bits + sacache -> index_bits)) + (index << sacache -> offset_bits);
	// printf(" evicting index %d block %lx00\n", victim, evicted_block);
	//}		

	if(sacache->ways[victim].dirty[index] == 1)
	{
		*dram_wb_addr = (evictee_tag << (sacache->index_bits + sacache->offset_bits)) + (index << sacache->offset_bits);
	}

	sacache->ways[victim].tag[index] = tag;
	sacache->ways[victim].valid[index] = 1;
	sacache->ways[victim].age[index] = sacache->clock;
	if(rw == 0)
	{
		sacache->ways[victim].dirty[index] = 0;
	}
	else
	{
		sacache->ways[victim].dirty[index] = 1;
	}


	return 0;
}

unsigned int flush_sacache(struct SACache *sacache)
{
	unsigned int i, j;

	for(i=0; i<sacache->num_sets; i++)
	{
		for(j=0; j<sacache->num_ways; j++)
		{
			sacache->ways[j].valid[i] = 0;
		}
	}

	return 0;
}

unsigned int get_hits(struct SACache *sacache)
{
	return sacache->hits;
}

unsigned int clear_hits(struct SACache *sacache)
{
	sacache->hits = 0;
	return 0;
}

// read misses + write_misses
unsigned int get_misses(struct SACache *sacache)
{
	return sacache->read_misses + sacache->write_misses;
}

unsigned int get_read_misses(struct SACache *sacache)
{
	return sacache->read_misses;
}

unsigned int get_write_misses(struct SACache *sacache)
{
	return sacache->write_misses;
}

unsigned int clear_misses(struct SACache *sacache)
{
	sacache->read_misses = 0;
	sacache->write_misses = 0;	
	return 0;
}

unsigned int destroy_sacache(struct SACache *sacache)
{

	//printf ("%d evictions\n", sacache->evictions);

	unsigned int i;
	for(i=0; i<sacache->num_ways; i++)
	{
		free(sacache->ways[i].valid);
		free(sacache->ways[i].tag);
		free(sacache->ways[i].age);
	}

	free(sacache->ways);

	free(sacache);

	return 0;
}

#endif
