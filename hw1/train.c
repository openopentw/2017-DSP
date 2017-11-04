#include "hmm.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEQ_LINE
#	define SEQ_LINE 10000
#endif

#ifndef SEQ_LINE_LEN
#	define SEQ_LINE_LEN 50
#endif

double alpha[SEQ_LINE_LEN][MAX_STATE];
double beta[SEQ_LINE_LEN][MAX_STATE];
double gama[SEQ_LINE_LEN][MAX_STATE];
double epsilon[SEQ_LINE_LEN][MAX_STATE][MAX_STATE];

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

void calc_alpha(HMM *hmm, int seq[SEQ_LINE_LEN])
{
	// initialization
	for(int i = 0; i < hmm->state_num; ++i) {
		alpha[0][i] = hmm->initial[i] * hmm->observation[seq[0]][i];
	}

	// induction
	for(int t = 0; t < SEQ_LINE_LEN - 1; ++t) {
		for(int j = 0; j < hmm->state_num; ++j) {
			alpha[t+1][j] = 0;
			for(int i = 0; i < hmm->state_num; ++i) {
				alpha[t+1][j] += alpha[t][i] * hmm->transition[i][j];
			}
			alpha[t+1][j] *= hmm->observation[seq[t+1]][j];
		}
	}
}

void calc_beta(HMM *hmm, int seq[SEQ_LINE_LEN])
{
	// initialization
	for(int i = 0; i < hmm->state_num; ++i) {
		beta[SEQ_LINE_LEN - 1][i] = 1;
	}

	// induction
	for(int t = SEQ_LINE_LEN - 1; t > 0; --t) {
		for(int i = 0; i < hmm->state_num; ++i) {
			beta[t-1][i] = 0;
			for(int j = 0; j < hmm->state_num; ++j) {
				beta[t-1][i] += beta[t][j] * hmm->transition[i][j] * hmm->observation[seq[t]][j];
			}
		}
	}
}

void calc_gama(HMM *hmm)
{
	for(int t = 0; t < SEQ_LINE_LEN; ++t) {
		double sum = 0;

		for(int i = 0; i < hmm->state_num; ++i) {
			gama[t][i] = alpha[t][i] * beta[t][i];
			sum += gama[t][i];
		}

		for(int i = 0; i < hmm->state_num; ++i) {
			gama[t][i] /= sum;
		}
	}
}

void calc_epsilon(HMM *hmm, int seq[SEQ_LINE_LEN])
{
	for(int t = 0; t < SEQ_LINE_LEN-1; ++t) {
		double sum = 0;

		for(int i = 0; i < hmm->state_num; ++i) {
			for(int j = 0; j < hmm->state_num; ++j) {
				epsilon[t][i][j] = alpha[t][i] * hmm->transition[i][j] * hmm->observation[seq[t+1]][j] * beta[t+1][j];
				sum += epsilon[t][i][j];
			}
		}

		for(int i = 0; i < hmm->state_num; ++i) {
			for(int j = 0; j < hmm->state_num; ++j) {
				epsilon[t][i][j] /= sum;
			}
		}
	}
}

void train(HMM *hmm, int seq[SEQ_LINE][SEQ_LINE_LEN])
{
	double pi_up[6] = {0};
	double a_up[6][6] = {{0}};
	double a_down[6] = {0};
	double b_up[6][6] = {{0}};
	double b_down[6] = {0};

	for(int n = 0; n < SEQ_LINE; ++n) {
		calc_alpha(hmm, seq[n]);
		calc_beta(hmm, seq[n]);
		calc_gama(hmm);
		calc_epsilon(hmm, seq[n]);

		// pi
		for(int i = 0; i < hmm->state_num; ++i) {
			pi_up[i] += gama[0][i];
		}

		// a
		for(int t = 0; t < SEQ_LINE_LEN-1; ++t) {
			for(int i = 0; i < hmm->state_num; ++i) {
				for(int j = 0; j < hmm->state_num; ++j) {
					a_up[i][j] += epsilon[t][i][j];
				}
				a_down[i] += gama[t][i];
			}
		}

		// b
		for(int t = 0; t < SEQ_LINE_LEN; ++t) {
			for(int j = 0; j < hmm->state_num; ++j) {
				int k = seq[n][t];
				b_up[k][j] += gama[t][j];
				b_down[j] += gama[t][j];
			}
		}
	}

	// update pi
	for(int i = 0; i < hmm->state_num; ++i) {
		hmm->initial[i] = pi_up[i] / SEQ_LINE;
	}

	// update a
	for(int i = 0; i < hmm->state_num; ++i) {
		for(int j = 0; j < hmm->state_num; ++j) {
			hmm->transition[i][j] = a_up[i][j] / a_down[i];
		}
	}

	// update b
	for(int k = 0; k < 6; ++k) {
		for(int j = 0; j < hmm->state_num; ++j) {
			hmm->observation[k][j] = b_up[k][j] / b_down[j];
		}
	}
}

int main(int argc, char **argv)
{
	// parse args
	int iterations = atoi(argv[1]);
	char *fn_model_init = argv[2];
	char *fn_seq_model = argv[3];
	char *fn_model = argv[4];

	// init model
	HMM hmm_initial;
	loadHMM(&hmm_initial, fn_model_init);
	/* dumpHMM(stdout, &hmm_initial ); */

	// load seq
	int seq[SEQ_LINE][SEQ_LINE_LEN];
	load_seq(seq, fn_seq_model);

	// train
	for(int i = 0; i < iterations; ++i) {
		/* printf("\n======= iteration %d =======\n", i); */
		train(&hmm_initial, seq);
		/* dumpHMM(stdout, &hmm_initial); */
	}

	// write
	FILE *fp = open_or_die(fn_model, "w");
	dumpHMM(fp, &hmm_initial);

	return 0;
}
