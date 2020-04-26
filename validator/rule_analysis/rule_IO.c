// This file contains the logic for reading/writing rules to a file in binary form

// Create a new rule file, write some struct size information to first 8 bytes
FILE * open_rule_file(const char * filename){
  FILE * f = fopen(filename, "wb");
  int ops_size = sizeof(operands_t);
  int res_size = sizeof(results_t);
  int rule_size = ops_size + res_size;
  int written1 = fwrite(&ops_size, sizeof(int), 1, f);
  int written2 = fwrite(&res_size, sizeof(int), 1, f);
  return f;
}

// Write out the contents of a meta_set_t, or if it's NULL then write sentinel bytes
void write_meta_set_t(const meta_set_t * t, FILE * f){
  if (t != NULL){
    fwrite(t, sizeof(meta_set_t), 1, f);
  } else {
    char null_bytes[sizeof(meta_set_t)];
    memset(null_bytes, 'A', sizeof(null_bytes));
    fwrite(null_bytes, sizeof(meta_set_t), 1, f);    
  }
}

void write_ops(operands_t * ops, FILE * f){
  int written = 0;
  write_meta_set_t(ops -> pc, f);
  write_meta_set_t(ops -> ci, f);
  write_meta_set_t(ops -> op1, f);
  write_meta_set_t(ops -> op2, f);
  write_meta_set_t(ops -> op3, f);
  write_meta_set_t(ops -> mem, f);
}

void write_res(results_t * res, FILE * f){
  write_meta_set_t(res -> pc, f);
  write_meta_set_t(res -> rd, f);
  write_meta_set_t(res -> csr, f);
  fwrite(&res -> pcResult, sizeof(bool), 1, f);
  fwrite(&res -> rdResult, sizeof(bool), 1, f);
  fwrite(&res -> csrResult, sizeof(bool), 1, f);
}

void write_rule(Rule * key_rule, Rule * val_rule, FILE * f){
  int written = 0;
  write_ops(&(key_rule -> first), f);
  write_ops(&(val_rule -> first), f);
  write_res(&(val_rule -> second), f);
}
