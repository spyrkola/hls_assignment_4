#include <iostream>
#include <cstdlib>
#include <fstream>
#include "ac_int.h"
#include "ac_math.h"
#include "my_types.h"

#include "mc_scverify.h"

const static int N = 50;
const static int M = 3;

// N: dataset size
// M: number of clusters
#pragma hls_design top
template<int N, int M>
bool CCS_BLOCK(kmeans)(Point points[N], int ID[N], Point center[M]) {
	// create register to store size of clusters and initialize to zero
	// and registers to store sums of x and y for new centroid
	// worst-case lengths: all points in one cluster
	ac_int<ac::log2_ceil<N>::val, false> cluster_size[M]; 
	ac::init_array<AC_VAL_0>(cluster_size, M);
	
	ac_int<ac::log2_floor<N * 65535>::val + 1, false> sums_x[M];
	ac::init_array<AC_VAL_0>(sums_x, M);

	ac_int<ac::log2_floor<N * 65535>::val + 1, false> sums_y[M];
	ac::init_array<AC_VAL_0>(sums_y, M);

	// for each point in the dataset, find closest center and assign the point to it.
	POINTS: for (int i = 0; i < N; i++) {
		ac_int<16, false> x = points[i].x;
		ac_int<16, false> y = points[i].y;
		
		int min_id = 0;
		ac_int<17, false> min_dist;
		min_dist.set_val<AC_VAL_MAX>();

		DIST: for (int j = 0; j < M; j++) {
			Point cur_center = center[j];

			ac_int<16, false> x_dist, y_dist;
			ac_math::ac_abs(x - cur_center.x, x_dist);
			ac_math::ac_abs(y - cur_center.y, y_dist);
			
			ac_int<17, false> dist = x_dist + y_dist;
			if (dist < min_dist) {
				min_dist = dist;
				min_id = j;
			}
		}
		
		cluster_size[min_id] += 1;
		sums_x[min_id] += x;
		sums_y[min_id] += y;

		ID[i] = min_id;
	}
	
	bool go_on = false;
	CENTROIDS: for (int i = 0; i < M; i++) {
		Point old_center = center[i];
		Point new_center;
		
		ac_math::ac_div(sums_x[i], cluster_size[i], new_center.x);
		ac_math::ac_div(sums_y[i], cluster_size[i], new_center.y);

		center[i].x = new_center.x;
		center[i].y = new_center.y;
		
		if ((old_center.x != new_center.x || old_center.y != new_center.y) && (!go_on)) {
			go_on = true;
		}
	}

	return go_on;
}

CCS_MAIN(int argc, char *argv[]) {
	srand(42);  // for reproducibility
	Point points[N];
	int ID[N];
	Point center[M];
	
	// generate random points with x, y between 0 and 100
	// also generate random initial centers in the same range
	for (int i = 0; i < N; i++) {
		points[i].x = rand() % 101;
		points[i].y = rand() % 101;
	}
	
	for (int i = 0; i < M; i++) {
		center[i].x = rand() % 101;
		center[i].y = rand() % 101;
	}
	
	// save generated dataset and initial random centers
	/*
	std::ofstream file;
	file.open("random_data.txt");
	for (int i = 0; i < N; i++) {
		file << points[i].x << ' ' << points[i].y << "\n";
	}
	file.close();
	
	file.open("random_centers.txt");
	for (int i = 0; i < M; i++) {
	   file << center[i].x << ' ' << center[i].y << "\n";
	}
	file.close();
	*/
	
	// run k-means algorithm
	int iter_counter = 0;
	bool go_on;
	do {
		iter_counter += 1;
		go_on = kmeans<N, M>(points, ID, center);
	} while (go_on);
	
	// save the final centers and the IDs that were assigned to each sample in the dataset
	/*
	file.open("final_centers.txt");
	for (int i = 0; i  < M; i++) {
	   file << center[i].x << ' ' << center[i].y << "\n";
	}
	file.close();
	
	file.open("final_ids.txt");
	for (int i = 0; i < N; i++) {
	   file << ID[i] << "\n";
	}
	file.close();
	*/
	
	std::cout << "K-Means algorithm finished after " << iter_counter << " iterations." << std::endl;
	std::cout << "The 3 centers are:" << std::endl;
	std::cout << center[0].x << ", " << center[0].y << std::endl;
	std::cout << center[1].x << ", " << center[1].y << std::endl;
	std::cout << center[2].x << ", " << center[2].y << std::endl; 
	
	CCS_RETURN(0);
}
