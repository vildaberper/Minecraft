#pragma once

#include "Block.h"
#include "Mesh.h"

class Chunk{
public:
	static const long CHUNK_SIZE = 32;
	static const long CHUNK_HEIGHT = 4;

	Chunk();
	Chunk(long x, long y, long z);
	~Chunk();

	void allocblocks();
	void deleteblocks();

	void updateEmpty();

	bool isFaceVisible(int x, int y, int z, Face f);

	void update(double dt);

	void render();

	bool readyToMesh();

	void updateMesh();

	bool transparentAt(int x, int y, int z);

	bool isBlockAt(int x, int y, int z);

	Chunk* xp = NULL;
	Chunk* xn = NULL;
	Chunk* yp = NULL;
	Chunk* yn = NULL;
	Chunk* zp = NULL;
	Chunk* zn = NULL;

	Block*** blocks;
	Mesh* mesh;
	Mesh* ncmesh;
	bool modified;
	long x, y, z;
	bool empty = false;
};