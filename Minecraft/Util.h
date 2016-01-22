#pragma once

#include <vector>
#include <set>
#include <math.h>
#include <minmax.h>

#define PI 3.14159265358979323846

class Util{
public:

	template <typename T>
	static bool vectorContains(const std::vector<T>& v, const T e){
		for (T t : v){
			if (t == e){
				return true;
			}
		}
		return false;
	}

	static double dist(double x0, double y0, double z0, double x1, double y1, double z1){
		return sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2) + pow(z0 - z1, 2));
	}

	static double dist(Chunk* chunk, Camera* camera){
		return dist(
			Chunk::CHUNK_SIZE * chunk->x + Chunk::CHUNK_SIZE / 2,
			Chunk::CHUNK_SIZE * chunk->y + Chunk::CHUNK_SIZE / 2,
			Chunk::CHUNK_SIZE * chunk->z + Chunk::CHUNK_SIZE / 2,
			camera->x,
			camera->y + camera->height,
			camera->z);
	}

	static long mDist(Chunk* c1, Chunk* c2){
		long xd = sqrt(pow(c1->x - c2->x, 2));
		long zd = sqrt(pow(c1->z - c2->z, 2));

		return xd > zd ? xd : zd;
	}
};