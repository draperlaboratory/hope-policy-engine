#include <assert.h>
#include "compute_hash.h"

#define UNASSIGNED -1
// switch between 0-3 (original hashes) and 4-7 (random permutation hashes)
#define HASH_BASE 4

int perm4[2048];
//int perm47324[2048];
int perm5172[2048];
int perm6237886[2048];
int perm7128386[2048];

void init_hashes() {
  #include "iperm.c"
  //#include "perm4.c"
  #include "perm5.c"
  #include "perm6.c"
  #include "perm7.c"
}

compute_hash_t::compute_hash_t(int nfields, int *field_widths, 
                               int k, int capacity) {
  //printf("Beginning compute_hash_t\n");
  int i,h,m,b;

#ifdef INIT_HASH_POSITIONS
  hash_positions_initialized=false;
  num_fields=nfields;
  total_ops_bits=0;
  for (i=0;i<num_fields;i++)
    total_ops_bits+=field_widths[i];

  sized_perm=(int **)malloc(sizeof(int *)*k);
  ops_index=(int *)malloc(sizeof(int)*total_ops_bits);
  bit_index=(int *)malloc(sizeof(int)*total_ops_bits);
  
  for (h=0;h<k;h++)
    sized_perm[h]=(int *)malloc(sizeof(int)*total_ops_bits);

  for (i=0;i<total_ops_bits;i++) {
    ops_index[i]=UNASSIGNED;
    bit_index[i]=UNASSIGNED;
    for (h=0;h<k;h++)
      sized_perm[h][i]=UNASSIGNED;
  }
  k_hash_position=k;
  hash_input_position=(int ***)malloc(sizeof(int **)*k);
  for (h=0;h<k;h++) {
    hash_input_position[h]=(int **)malloc(sizeof(int *)*num_fields);
    for (m=0;m<num_fields;m++) {
      hash_input_position[h][m]=(int *)malloc(sizeof(int)*field_widths[m]);
      for (b=0;b<field_widths[m];b++)
        hash_input_position[h][m][b]=UNASSIGNED;
    }
  }

  // make calls to perform initialization
  // (a) setup arguments/inputs need to make call
  meta_set_t *dummy_ops=(meta_set_t *)malloc(sizeof(meta_set_t)*num_fields);
  for (int i=0;i<num_fields;i++){
    meta_set_t *meta_set = new meta_set_t();
    meta_set->tags[0]=0;
    dummy_ops[i]=*meta_set;
    delete meta_set;
  }

  int *dummy_hashes=(int *)malloc(sizeof(int)*k);

  bool *dummy_consider=(bool *)malloc(sizeof(bool)*num_fields);

  // (b) call compute_hash for each of the hashes to setup sized_perm and hash_input_position
  // need to call this to get ops_index, bits_index set up
  //printf("Before compute_hash_set\n");
  compute_hash_set(k, dummy_ops, dummy_hashes, 
                   num_fields, field_widths,
                   capacity, dummy_consider);
  //printf("After compute_hash_set\n");
//#ifdef WRITE_VERILOG_HASH
 // verilog_hash(capacity,field_widths);
//#endif
  free(dummy_consider);
  free(dummy_ops);
  free(dummy_hashes);
  hash_positions_initialized=true;
#else
  total_ops_bits=0;
  k_hash_position=0;
  ops_index=(int *)NULL;
  bit_index=(int *)NULL;
  hash_input_position=(int ***)NULL;
  sized_perm=(int **)NULL;
#endif

  hash_table.clear();
  //static_ops=(operands_t *)malloc(sizeof(operands_t));

  //printf("Hasher initialized\n");
}

compute_hash_t::~compute_hash_t() {
  int h, m;
  free(ops_index);
  free(bit_index);
  for (h=0;h<k_hash_position;h++)
    free(sized_perm[h]);
  free(sized_perm);
  for (h=0;h<k_hash_position;h++) {
    for (m=0;m<num_fields;m++) {
      free(hash_input_position[h][m]);
    }
    free(hash_input_position[h]);
  }
  free(hash_input_position);
  for (auto entry : hash_table) {
    delete entry.first.pc;
    delete entry.first.ci;
    if (entry.first.op1)
      delete entry.first.op1;
    if (entry.first.op2)
      delete entry.first.op2;
    if (entry.first.op3)
      delete entry.first.op3;
    if (entry.first.mem)
      delete entry.first.mem;
  }
  hash_table.clear();
  /**if (static_ops->pc)
    delete static_ops->pc;
  if (static_ops->ci)
    delete static_ops->pc;
  if (static_ops->op1)
    delete static_ops->op1;
  if (static_ops->op2)
    delete static_ops->op2;
  if (static_ops->op3)
    delete static_ops->op3;
  if (static_ops->mem)
    delete static_ops->mem;
  free(static_ops);*/
}

int min(int a, int b) {
  if (a<b) return(a); else return(b);
}

int log2(int a) {
  int res;
  for (res=0;(1<<res)<a;res++);
  return(res);
}

int count_ones(int num_fields, int *field_widths, meta_set_t *fields) {
  int cnt=0;
  for (int i=0;i<num_fields;i++)
    for (int j=0;j<field_widths[i];j++)
      if (((fields->tags[i]>>j) & 0x01)==1)
    cnt++;
  return(cnt);
}
// Potentially use consider to change result
int compute_hash_t::compute_hash_from_precomputed_positions(int which, meta_set_t *ops, bool *consider) {
  int result=0;
  int i;
  int val;
  for (i=0;i<total_ops_bits;i++) {
    val=((ops[ops_index[i]].tags[0])>>bit_index[i]) & 0x01;
    int pos=hash_input_position[which][ops_index[i]][bit_index[i]];
    result=result^(val<<pos);
  }
  return(result);
}

void compute_hash_t::compute_hash_set_from_precomputed_positions(int k, meta_set_t *ops, int *hashes, bool *consider) {
  int h;

#ifdef HASH_HASH
  operands_t *static_ops=new operands_t();
  static_ops->pc=new meta_set_t{OP_PC};
  static_ops->ci=new meta_set_t{OP_CI};
  if (consider[OP_OP1]) 
    static_ops->op1=new meta_set_t{ops[OP_OP1]};
  if (consider[OP_OP2])
    static_ops->op2=new meta_set_t{ops[OP_OP2]};
  if (consider[OP_OP3])
    static_ops->op3=new meta_set_t{ops[OP_OP3]};
  if (consider[OP_MEM])
    static_ops->mem=new meta_set_t{ops[OP_MEM]};

  auto entries = hash_table.find(*static_ops);
  if (entries==hash_table.end()) {
    std::vector<int> new_h(k_hash_position);
    for (h=0;h<k_hash_position;h++) {
      hashes[h]=compute_hash_from_precomputed_positions(h,ops,consider);
      new_h[h]=hashes[h];
    }
    operands_t *new_ops=new operands_t();
    new_ops->pc=static_ops->pc;
    new_ops->ci=static_ops->ci;
    if (static_ops->op1)
      new_ops->op1=static_ops->op1;
    if (static_ops->op2)
      new_ops->op2=static_ops->op2;
    if (static_ops->op3)
      new_ops->op3=static_ops->op3;
    if (static_ops->mem)
      new_ops->mem=static_ops->mem;
    hash_table.insert({*new_ops,new_h});
    delete new_ops;
  } else {
    std::vector<int> result=entries->second;
    for (h=0;h<k_hash_position;h++)
      hashes[h]=result.at(h);
  }
  delete static_ops;
#else
  for (h=0;h<k_hash_position;h++)
    hashes[h]=compute_hash_from_precomputed_positions(h,ops,consider);
#endif
}

int compute_hash_t::fold(int num_fields, int *field_widths, meta_set_t *fields, int hash_table_size) {
  int width=log2(hash_table_size);
  int result=0;
  int final_pos=0;
  int field=0;
  int chunk=0;

  for (field=0;field<num_fields;field++) {
    int field_pos=0;
    while (field_pos<field_widths[field]) {
      int bits=min((field_widths[field]-field_pos), (width-final_pos));
      int tmp=(((fields[field].tags[0])>>field_pos) & ((1<<bits) -1))<<final_pos;
      chunk^=tmp;

#ifdef INIT_HASH_POSITIONS
      int h;
      if (hash_positions_initialized==false) {
        for (h=0;h<k_hash_position;h++) {
          int p=sized_perm[h][field]; 
          if (p>=0) // gets called for some h's before sized_perm defined
            hash_input_position[h][ops_index[p]][bit_index[p]]=final_pos;
        }
      }
#endif      
      field_pos+=bits;
      final_pos+=bits;
      if (final_pos>=width) {
        result^=chunk;
        final_pos=0;
        chunk=0;
      }
    }
      // field++ occurs here when field_pos gets to field_widths[field]
  }
  if (final_pos>0) {
    result^=chunk; // final chunk
  }
  return result; 
}

meta_set_t bitrev(int len, meta_set_t a) {
  meta_set_t *result= new meta_set_t();
  result->tags[0]=0;

  for (int i=0;i<len;i++) {
      int bit = (a.tags[0]>>i) % 2;
      result->tags[0] |= (bit<<(len-i-1));
  }
  return *result;
}

void compute_hash_t::convert_to_bit_fields(int orig_num_fields, int *orig_field_widths, meta_set_t *orig_fields,
                                           int *field_widths, meta_set_t *fields, bool *consider) {
  int current=0;
  for (int i=0;i<orig_num_fields;i++) {
    for (int j=0;j<orig_field_widths[i];j++) {
      field_widths[current]=1;
      fields[current].tags[0]=(((orig_fields[i].tags[0])>>j) & 0x01);
#ifdef INIT_HASH_POSITIONS
      ops_index[current]=i;
      bit_index[current]=j;
#endif
      current++;
    }
  }
}

void compute_hash_t::compute_hash_set(int k, meta_set_t *ops, int *hashes, int num_fields, 
                                      int *field_widths, int capacity, bool *consider) {
  int ones=0;
  int bits=0;
  for (int i=0;i<num_fields;i++)
    bits+=field_widths[i];

  // convert to long bit vector
  int *bit_field_widths=(int *)malloc(sizeof(int)*(bits));
  meta_set_t *bit_fields=(meta_set_t *)malloc(sizeof(meta_set_t)*(bits));
  for (int i=0;i<bits;i++) {
    meta_set_t *meta_set = new meta_set_t();
    meta_set->tags[0]=0;
    bit_fields[i]=*meta_set;
    delete meta_set;
  }
  int bit_num_fields=bits;
  convert_to_bit_fields(num_fields,field_widths,ops,bit_field_widths,bit_fields,consider);
  meta_set_t *permute_bit_fields=(meta_set_t *)malloc(sizeof(meta_set_t)*(bits));
  for (int i=0;i<bits;i++) {
    meta_set_t *meta_set = new meta_set_t();
    meta_set->tags[0]=0;
    permute_bit_fields[i]=*meta_set;
    delete meta_set;
  }

  for (int i=0;i<k;i++) {
      //int tmp=compute_hash(i,bit_num_fields,bit_field_widths,bit_fields,capacity);
      //fprintf(stderr,"returned from compute_hashes with %x\n",tmp); fflush(stderr); // serious debug
      //fprintf(stderr," (attempt to read old value hashes[%d]=%x)\n",i,hashes[i]);
      hashes[i]=compute_hash((i+HASH_BASE),bit_num_fields,bit_field_widths,bit_fields,permute_bit_fields,capacity,ones);
      //fprintf(stderr,"hash[%d]=%x\n",i,hashes[i]); fflush(stderr); // serious debug
  }
  
  free(bit_field_widths);
  free(bit_fields);
  free(permute_bit_fields);
}

int compute_hash_t::compute_hash(int which, int num_fields, int *field_widths, meta_set_t *fields,
         meta_set_t *permute_fields, int hash_table_size, int ones_cnt) {
  if (which==0) {// bit reverse entire thing
    // reverse fields
    for (int i=0;i<(num_fields/2);i++) {
      int tmp_field=fields[i].tags[0];
      permute_fields[i].tags[0]=fields[num_fields-i-1].tags[0];
      permute_fields[num_fields-i-1].tags[0]=tmp_field;
    }
    if (num_fields%2==1) // 
      permute_fields[(num_fields)/2].tags[0]=fields[(num_fields)/2].tags[0];
  } else if (which==1) { // bit pairwise swap
    for (int i=0;(i+1)<num_fields;i+=2) {
      int vi=fields[i].tags[0];
      int vip=fields[i+1].tags[0];
      permute_fields[i].tags[0]=vip;
      permute_fields[i+1].tags[0]=vi;
    }
    if (num_fields%2==1) 
      permute_fields[num_fields-1].tags[0]=fields[num_fields-1].tags[0];
  } else if (which==2) { // swap halves
    int split=num_fields/2;
    if (num_fields%2==0) {
      for (int i=0;i<split;i++) {
        int lo=fields[i].tags[0];
        int hi=fields[i+split].tags[0];
        permute_fields[i].tags[0]=hi;
        permute_fields[i+split].tags[0]=lo;
      }
      permute_fields[split].tags[0]=permute_fields[split<<1].tags[0];
    } else { // odd case (very odd)
      int top=fields[split].tags[0];
      for(int i=0;i<split;i++) {
        permute_fields[i+split].tags[0]=fields[i].tags[0];
        permute_fields[i].tags[0]=fields[i+split+1].tags[0];
      }
      permute_fields[split<<1].tags[0]=fields[split].tags[0];
      permute_fields[num_fields-1].tags[0]=top;
    }
  } else if (which==3) { // looks like one is reversed and other isn't
    int reverse_index=num_fields-1;
    if ((num_fields%2)==1) {
      reverse_index=num_fields-2;
      for (int i=0;i<reverse_index/2;i+=2) {
        int vi=fields[i].tags[0];
        int vip=fields[i+1].tags[0];
        int vri=fields[reverse_index-i].tags[0];
        int vrip=fields[reverse_index-(i+1)].tags[0];

        // 0 and 1 differ only in assignments we make here
        permute_fields[i].tags[0]=vri;
        permute_fields[i+1].tags[0]=vi;
        permute_fields[reverse_index-i].tags[0]=vrip;
        permute_fields[reverse_index-(i+1)].tags[0]=vip;
      }
      if ((reverse_index/4)%2==1) { // blah, hate to special case, but appears to be missed by above
        int vi=fields[reverse_index/2].tags[0];
        int vip=fields[reverse_index/2+1].tags[0];
        permute_fields[reverse_index/2].tags[0]=vip;
        permute_fields[reverse_index/2+1].tags[0]=vi;
      }
      if (num_fields%2==1) 
        permute_fields[num_fields-1].tags[0]=fields[num_fields-1].tags[0];
    }
  }
  //  else if (which==4) // noop
  //    {
  //      // not sure why one of the main hash functions (hash 0) isn't the one that leaves the bits in place?
  //      // --> do nothing and the fold occurs on the original set of bits
  //      for (int i=0;i<num_fields;i++)
  //    permute_fields[i]=fields[i];
  //}
  else if (which<8) {
    int *perm;
    if (which==4)
      //perm=perm47324;
      perm=perm4;
    else if (which==5)
      perm=perm5172;
    else if (which==6)
      perm=perm6237886;
    else if (which==7)
      perm=perm7128386; 
    for (int i=0;i<num_fields;i++) {
      int tmp=perm[i];
      while (tmp>=num_fields) tmp=perm[tmp];
      permute_fields[i].tags[0]=fields[tmp].tags[0];
#ifdef INIT_HASH_POSITIONS
      if (!hash_positions_initialized) {
        sized_perm[which-HASH_BASE][i]=tmp; // original
      }
#endif
    }
  } else {
      //fprintf(stderr,"Hash functions>8 not implemented -- add one or more\n");
      // ...use hash/random_perm.c to generate more random permutations...
      exit(1);
  }  

  int result=fold(num_fields,field_widths,permute_fields,hash_table_size);

  return(result);
}
