#include "systemc.h"
#include <ctime>
#include <math.h>
#include <vector>
#include <iostream>


const int nAnts = 5;
const int nCities = 100;
double sum_prob;
int Graph[nCities][2];
int counter = 0;
double distances[nCities][nCities];
double weights[nCities][nCities];
int path_out[nAnts][nCities];
int best_path[nCities];
double full_dist[nAnts];
double alpha = 0.5;
double beta = 2.0;
double best_path_len = 10000;
sc_event path_update, weight_update;

SC_MODULE(Soft_ware) {

	SC_CTOR(Soft_ware) {
		SC_THREAD(moving_ants);
	}

	double transition_prob(int i, int j) {
		return pow(weights[i][j], alpha) * pow((1 / distances[i][j]), beta);
	}

	void moving_ants() {
		while (counter < 100) {
			srand(time(0));
			int all_pathes[nAnts][nCities];
			int path[nCities];
			for (int i = 0; i < nAnts; i++) {
				for (int i = 0; i < nCities; i++)
				{
					path[i] = 0;
				}
				int curr_node = 0;
				sum_prob = 0;
				double distance = 0;
				int count = 0;
				while (count < nCities-1) {
					for (int j = 0; j < nCities; j++) {
						if (j == curr_node || std::find(std::begin(path), std::end(path), j) != std::end(path)) {
							continue;
						}
						sum_prob += transition_prob(curr_node, j);
					}
					double sum_choice = 0;
					double f = (double)rand() / RAND_MAX;
					double choice = f * sum_prob;
					sum_prob = 0;
					for (int j = 0; j < nCities; j++) {
						if (j == curr_node || std::find(std::begin(path), std::end(path), j) != std::end(path)) {
							continue;
						}
						sum_choice += transition_prob(curr_node, j);
						if (choice <= sum_choice) {
							sum_prob = 0;
							distance += distances[curr_node][j];
							path[count] = j;
							count++;
							curr_node = j;
							break;
						}
					}
				}
				for (int j = 0; j < nCities; j++) {
					path_out[i][j] = path[j];
				}
				full_dist[i] = distance;
			}
			wait(10, SC_NS);
			path_update.notify(SC_ZERO_TIME);
			wait(weight_update);
		}
		sc_stop();
	}
};

SC_MODULE(Hard_ware) {

	void update_weights() {
		//evaporate
		while (counter < 100) {
			wait(path_update);
			for (int i = 0; i < nCities; i++) {
				for (int j = 0; j < nCities; j++) {
					weights[i][j] *= 0.99;
				}
			}

			for (int i = 0; i < nAnts; i++) {
				if (full_dist[i] < best_path_len) {
					best_path_len = full_dist[i];
					for (int j = 0; j < nCities; j++) {
						best_path[j] = path_out[i][j];
					}
				}
				double diff = full_dist[i] - best_path_len + 0.01;
				double w = 0.01 / diff;
				for (int j = 0; j < (nCities + 1); j++) {
					weights[j % nCities][(j + 1) % nCities] += w;
				}

			}
			counter++;
			cout << "iteration number: " << counter << endl;
			cout << "best path length: " << best_path_len << endl;
			cout << "best path: " << endl;
			for (int i = 0; i < nCities; i++) {
				cout << best_path[i] << ",";
			}
			wait(10, SC_NS);
			weight_update.notify(SC_ZERO_TIME);
		}
	}


	SC_CTOR(Hard_ware) {
		SC_THREAD(update_weights);
	}

};

int sc_main(int argc, char* argv[]) {
	int posX;
	int posY;
	int sig_Graph[nCities][2];

	srand(time(0));
	for (int i = 0; i < nCities; i++) {
		posX = 1 + (rand() % 100);
		posY = 1 + (rand() % 100);
		Graph[i][0] = posX;
		Graph[i][1] = posY;
	}

	for (int i = 0; i < nCities; i++) {
		for (int j = 0; j < nCities; j++) {
			weights[i][j] = 1;
			distances[i][j] = sqrt(pow(Graph[i][0] - Graph[j][0], 2) + pow(Graph[i][1] - Graph[j][1], 2));
		}
	}

	Soft_ware S("Soft_ware");
	Hard_ware H("Hard_ware");

	sc_start(0.5, SC_SEC);
	sc_start();
	return(0);

}