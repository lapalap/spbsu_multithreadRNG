// stepic_one.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "stdafx.h"
#include <thread>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <vector>
#include <chrono>
#include <ctime>
#include <string>
#include "pcg_random.hpp"									//PCG32 Random number generator
#include "pcg_extras.hpp"

const int SEED = 123;
const double EPS = 0.0000001;


void generate_chunk(
	pcg32 &RNG,
	const unsigned int start,
	const unsigned int chunk_size,
	std::exponential_distribution<> &dist,
	const unsigned int shift,
	std::vector<double> &RESULT
)
{
	pcg32 RNG_copy = RNG;

	unsigned int end = start + chunk_size;

	for (unsigned int i = start; i < end; ++i) {			//Filling the vector
		RESULT[i] = dist(RNG_copy);
	}

	RNG_copy.advance(shift);								//Jump ahead
	//RNG = RNG_copy;

}

std::vector<double> generate_vector_parallel(
	pcg32 &RNG,
	std::exponential_distribution<> &dist,
	const unsigned int length,
	const unsigned int thread_count
) {
	std::vector<double> _vec(length);

	unsigned int chunk_size = length / thread_count;
	unsigned int last_chunk_size = length - chunk_size * (thread_count - 1);

	std::vector<std::thread> threads;

	for (unsigned int i = 0; i < thread_count - 1; ++i) {
		threads.emplace_back(
			generate_chunk,
			std::ref(RNG),
			chunk_size * i,
			chunk_size,
			std::ref(dist),
			chunk_size,
			std::ref(_vec)
		);
	}
	threads.emplace_back(
		generate_chunk,
		std::ref(RNG),
		chunk_size * (thread_count - 1),
		last_chunk_size,
		std::ref(dist),
		chunk_size,
		std::ref(_vec)
	);

	for (auto &thread : threads) {
		thread.join();
	}
	return _vec;


}

std::vector<double> generate_vector(
	pcg32 &RNG,
	std::exponential_distribution<> &dist,
	const unsigned int length,
	const unsigned int parts
	)
{
	std::vector<double> _vec(length);

	unsigned int chunk_size = length / parts;
	unsigned int last_chunk_size = length - chunk_size * (parts - 1);

	for (unsigned int i = 0; i < parts - 1; ++i) {
			generate_chunk(
			std::ref(RNG),
			chunk_size * i,
			chunk_size,
			std::ref(dist),
			chunk_size,
			std::ref(_vec)
		);
	}

		generate_chunk(
		std::ref(RNG),
		chunk_size * (parts - 1),
		last_chunk_size,
		std::ref(dist),
		chunk_size,
		std::ref(_vec)
	);

		return _vec;
}

void eval_sin_mean_chunk(
	std::vector<double> &vec,
	std::vector<double> &means,
	const unsigned int thread_id,
	const unsigned int start,
	const unsigned int size
)

{
	double mean = 0.;
	for (unsigned int i = 0; i < vec.size(); ++i)
	{
		mean = (mean*i + std::sin(vec[i])) / (i + 1);
	}

	means[thread_id] = mean;
}

double eval_sin_mean_parralel(
	std::vector<double> &vec,
	const unsigned int thread_count
)
{
	unsigned int chunk_size = vec.size() / thread_count;
	unsigned int last_chunk_size = vec.size() - chunk_size * (thread_count - 1);

	std::vector<double> means(thread_count);
	std::vector<std::thread> threads;

	double mean_final = 0.;

	for (unsigned int i = 0; i < thread_count - 1; ++i) {
		threads.emplace_back(
			eval_sin_mean_chunk,
			std::ref(vec),
			std::ref(means),
			i,
			chunk_size * i,
			chunk_size
		);
	}
	threads.emplace_back(
		eval_sin_mean_chunk,
		std::ref(vec),
		std::ref(means),
		thread_count - 1,
		chunk_size * (thread_count - 1),
		last_chunk_size
	);

	for (auto &thread : threads) {
		thread.join();
	}

	for (unsigned int i = 0; i < thread_count-1; ++i) {
		mean_final += means[i] * chunk_size / vec.size();
	}
	mean_final += means[thread_count - 1] * last_chunk_size / vec.size();
	return mean_final;

}

double eval_sin_mean(
	std::vector<double> &vec,
	const unsigned int thread_count
)
{
	unsigned int chunk_size = vec.size() / thread_count;
	unsigned int last_chunk_size = vec.size() - chunk_size * (thread_count - 1);

	std::vector<double> means(thread_count);
	std::vector<std::thread> threads;

	double mean_final = 0.;

	for (unsigned int i = 0; i < thread_count - 1; ++i) {
			eval_sin_mean_chunk(
			std::ref(vec),
			std::ref(means),
			i,
			chunk_size * i,
			chunk_size
		);
	}
		eval_sin_mean_chunk(
		std::ref(vec),
		std::ref(means),
		thread_count - 1,
		chunk_size * (thread_count - 1),
		last_chunk_size
	);

	for (auto &thread : threads) {
		thread.join();
	}

	for (unsigned int i = 0; i < thread_count - 1; ++i) {
		mean_final += means[i] * chunk_size / vec.size();
	}
	mean_final += means[thread_count - 1] * last_chunk_size / vec.size();
	return mean_final;
}


bool are_vectors_identical(
	const std::vector<double> &vec1,
	const std::vector<double> &vec2
)
{
	if (vec1.size() != vec2.size()) {
		return false;
	}
	unsigned int length = vec1.size();
	for (unsigned int i = 0; i < length; ++i) {			//Filling the vector
		if (vec1[i] != vec2[i]) {
			std::cout << std::to_string(i);
			return false;
		}
	}
	return true;
}

int main()
{
	std::exponential_distribution<> normal_dist(1);
	pcg32 RNG(SEED);
	pcg32 RNG2(SEED);
	std::cout << "               _    _       _    _                             _  ___    _   _  ___ " << std::endl;
	std::cout << "/'\\_/`\\       (_ ) ( )_  _ ( )_ ( )                           ( )|  _`\\ ( ) ( )(  _`\\ " << std::endl;
	std::cout << "|     | _   _  | | | ,_)(_)| ,_)| |__   _ __   __     _ _    _| || (_) )| `\\| || ( (_)" << std::endl;
	std::cout << "| (_) |( ) ( ) | | | |  | || |  |  _ `\\( '__)/'__`\\ /'_` ) /'_` || ,  / | , ` || |___ " << std::endl;
	std::cout << "| | | || (_) | | | | |_ | || |_ | | | || |  (  ___/( (_| |( (_| || |\\ \\ | |`\\ || (_, )" << std::endl;
	std::cout << "(_) (_)`\\___/'(___)`\\__)(_)`\\__)(_) (_)(_)  `\\____)`\\__,_)`\\__,_)(_) (_)(_) (_)(____/'" << std::endl << std::endl;

	unsigned int N = 1000000;
	std::cout << "Generation of vector of length: "<<std::to_string(N) << " from a normal distribution"<< std::endl;
	std::cout << "Parralel vector generation..." << std::endl;
	auto start = std::chrono::steady_clock::now();
	std::vector<double> parallel = generate_vector_parallel(RNG, normal_dist, N, 4);
	auto end = std::chrono::steady_clock::now();
	std::cout << "that took " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;
	std::cout << "COMPLETED" << std::endl;
	std::cout << "NONparralel vector generation..." << std::endl;
	start = std::chrono::steady_clock::now();
	//RNG2;
	std::vector<double> nonparallel = generate_vector(RNG2, normal_dist, N, 4);
	end = std::chrono::steady_clock::now();
	std::cout << "that took " << std::chrono::duration<double>(end - start).count()<<" s" << std::endl;
	std::cout << "COMPLETED" << std::endl;
	if (are_vectors_identical(parallel, nonparallel)) {
		std::cout << "Vectors seem to be identical"<< std::endl;
	}
	else {
		std::cout << "Vectors seem not to be identical" << std::endl;
	}
	std::cout << "PART 2" << std::endl;
	std::cout << "Evaluating sin mean via parallel computing..." << std::endl;
	start = std::chrono::steady_clock::now();
	std::cout <<std::to_string(eval_sin_mean_parralel(nonparallel,4)) << std::endl;
	end = std::chrono::steady_clock::now();
	std::cout << "that took " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;
	std::cout << "COMPLETED" << std::endl;

	std::cout << "Evaluating sin mean via NONparallel computing..." << std::endl;
	start = std::chrono::steady_clock::now();
	std::cout << std::to_string(eval_sin_mean(nonparallel, 4)) << std::endl;
	end = std::chrono::steady_clock::now();
	std::cout << "that took " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;
	std::cout << "COMPLETED" << std::endl;



	std::cin.get();

}

