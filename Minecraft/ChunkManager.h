#pragma once

#include "Chunk.h"
#include "Camera.h"
#include "Util.h"
#include "IO.h"
#include "noise.h"
#include <thread>

#include <set>
#include <map>

class ChunkManager{
public:
	static const long MAX = 2147480;

	noise::module::Perlin* groundm;
	noise::model::Plane* groundp;

	noise::module::Perlin* mtmm;
	noise::model::Plane* mtmp;

	noise::module::Perlin* hillsm;
	noise::model::Plane* hillsp;

	noise::module::Perlin* cavem;

	ChunkManager();
	~ChunkManager();

	bool checkLoad_helper();

	void checkLoad(Camera* camera);

	void frame(Camera* camera);

	void updateNeighbors(Chunk* chunk, bool unload);

	void forceLoadChunk(const long x, const long y, const long z);

	Chunk* forcedChunkAt(const long x, const long y, const long z);

	Chunk* chunkAt(const long x, const long y, const long z);

	Block* blockAt(const long x, const long y, const long z);

	void setTypeAt(const long x, const long y, const long z, const BlockType type);

	void updateMesh();

	bool isLoaded(long x, long y, long z);

	void load(Camera* camera);

	void checkUnload(Camera* camera);

	void unload(Camera* camera);

	void generate();

	void render();

	map<long, map<long, map<long, Chunk*>>> chunks;

	std::set<Chunk*> chunksToUpdateMesh;

	std::set<Chunk*> chunksToLoad;
	std::set<Chunk*> chunksToUnload;

	std::set<Chunk*> chunksToGenerate;

	std::set<Chunk*> chunksToRender;

	long viewDistance = 16;

	int CHUNKS_TO_CHECKLOAD_PER_FRAME = 3;
	int CHUNKS_TO_LOAD_PER_FRAME = 4;
	int CHUNKS_TO_UNLOAD_PER_FRAME = 4;
	int CHUNKS_TO_MESH_PER_FRAME = 4;
	int CHUNKS_TO_GENERATE_PER_FRAME = 4;

	long framecount = 0;

	long lx = 0;
	long ly = 0;
	long lz = 0;
	int ld = 0;
	int li = 0;
	int lefti = 0;
	int inc = 0;
	int checked = 0;

	Chunk* lastChunk;
private:
	bool loadChunk(Chunk* chunk);

	void unloadChunk(Chunk* chunk);

	void generateChunk(Chunk* chunk);
};