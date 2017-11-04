#include "hmm.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEQ_LINE
#	define SEQ_LINE 2500
#endif

#ifndef SEQ_LINE_LEN
#	define SEQ_LINE_LEN 50
#endif

int result_model[SEQ_LINE];
double result_p[SEQ_LINE];

void load_seq(int seq[SEQ_LINE][SEQ_LINE_LEN], const char *filename)
{
	FILE *fp = open_or_die( filename, "r");

	for(int i = 0; i < SEQ_LINE; ++i) {
		char line[SEQ_LINE_LEN+1];
		fscanf(fp, "%s", line);
		for(int j = 0; j < SEQ_LINE_LEN; ++j) {
			seq[i][j] = line[j] - 'A';
			// printf("%d", seq[i][j]);
		}
		// puts("");
	}
}

double calc_p(HMM *hmm, int seq[SEQ_LINE_LEN])
{
	double delta[SEQ_LINE_LEN][hmm->state_num];

	// initialization
	for(int n = 0; n < hmm->state_num; ++n) {
		delta[0][n] = hmm->initial[n] * hmm->observation[seq[0]][n];
	}

	// recursion
	for(int t = 0; t < SEQ_LINE_LEN-1; ++t) {
		for(int n = 0; n < hmm->state_num; ++n) {
			double max_prod = -1;
			for(int i = 0; i < hmm->state_num; ++i) {
				double prod = delta[t][i] * hmm->transition[i][n];
				if(prod > max_prod) {
					max_prod = prod;
				}
			}
			delta[t+1][n] = max_prod * hmm->observation[seq[t+1]][n];
		}
	}

	// termination
	double max_delta = -1;
	for(int n = 0; n < hmm->state_num; ++n) {
		if(delta[SEQ_LINE_LEN-1][n] > max_delta) {
			max_delta = delta[SEQ_LINE_LEN-1][n];
		}
	}

	return max_delta;
}

void test(HMM hmms[5], int seq[SEQ_LINE][SEQ_LINE_LEN])
{
	for(int j = 0; j < SEQ_LINE; ++j) {
		double max_p = -1;
		int arg_max_p;
		for(int i = 0; i < 5; ++i) {
			double p = calc_p(&(hmms[i]), seq[j]);
			if(p > max_p) {
				max_p = p;
				arg_max_p = i;
			}
		}

		result_p[j] = max_p;
		result_model[j] = arg_max_p;
	}
}

void dump_result(FILE *fp)
{
	for(int j = 0; j < SEQ_LINE; ++j) {
		fprintf(fp, "model_0%d.txt %g\n", result_model[j]+1, result_p[j]);
	}
}

int main(int argc, char **argv)
{
	// parse args
	char *fn_model_list = argv[1];
	char *fn_testing_data = argv[2];
	char *fn_result = argv[3];

	// init model
	HMM hmms[5];
	load_models(fn_model_list, hmms, 5);
	/* dump_models(hmms, 5); */

	// load seq
	int seq[SEQ_LINE][SEQ_LINE_LEN];
	load_seq(seq, fn_testing_data);

	test(hmms, seq);

	FILE *fp = open_or_die(fn_result, "w");
	dump_result(fp);

	return 0;
}
