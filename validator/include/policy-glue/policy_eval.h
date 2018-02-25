/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef POLICY_EVAL_H
#define POLICY_EVAL_H

// pull in riscv definitions for context, operands and results
#include "riscv_isa.h"
#include "policy_meta_set.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define POLICY_EXP_FAILURE 0
#define POLICY_IMP_FAILURE -1
#define POLICY_SUCCESS 1

  
  /**
   * Allocate and free memory for policy eval structures.
   * Only need to do once, prepare eval should initialize each eval cycle
   */
void alloc_eval_params(context_t **ctx, operands_t **ops, results_t **res);
void free_eval_params(context_t **ctx, operands_t **ops, results_t **res);

  /**
   * Initialize context and set up operands before policy eval.
   */
void prepare_eval(context_t *ctx, operands_t *ops, results_t *res);

  /**
   * Evaluate policy with context and operands, populate results.
   *
   * Returns status:
   *    policyExpFailure = 0
   *    policyImpFailure = -1
   *    policySuccess = 1
   */
int eval_policy(context_t *ctx, operands_t *ops, results_t *res);

  /**
   * Install rule with operands and results.
   */
void complete_eval(context_t *ctx, operands_t *ops, results_t *res);

  /**
   * Helper Fn to optomize by returning a cannonical representation of
   * a metadata set. Necessary for performance and to save memory.
   */  
meta_set_t *canonize(meta_set_t *ts);

  
  /**
   * Print eval status
   */
void debug_msg(context_t *ctx, const char *msg);
  
  /**
   * Print eval status
   */
void debug_status(context_t *ctx, int status);
  
  /**
   * Print operands
   */
void debug_operands(context_t *ctx, operands_t *ops);


  /**
   * Print results
   */
void debug_results(context_t *ctx, results_t *res);

  /**
   * Call this if there is a rule violation
   */
void handle_violation(context_t *ctx, operands_t *ops, results_t *out);
  
  /**
   * Call this if there is a fatal error in the eval code
   */
void handle_panic(const char *msg);
  
#ifdef __cplusplus
}
#endif

#endif // POLICY_EVAL_H

