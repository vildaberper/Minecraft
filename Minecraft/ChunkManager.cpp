#include "ChunkManager.h"

ChunkManager::ChunkManager(){
	groundm = new noise::module::Perlin();
	groundp = new noise::model::Plane();
	groundm->SetFrequency(MAX / 1000.0);
	groundp->SetModule(*groundm);
	groundm->SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);

	mtmm = new noise::module::Perlin();
	mtmp = new noise::model::Plane();
	mtmm->SetFrequency(MAX / 1000.0);
	mtmp->SetModule(*mtmm);
	mtmm->SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);

	hillsm = new noise::module::Perlin();
	hillsp = new noise::model::Plane();
	hillsm->SetFrequency(MAX / 600.0);
	hillsm->SetLacunarity(2);
	hillsp->SetModule(*hillsm);
	hillsm->SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);

	cavem = new noise::module::Perlin();
	cavem->SetFrequency(MAX / 100.0);
	cavem->SetLacunarity(2);
	cavem->SetOctaveCount(9);
	cavem->SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);
}

ChunkManager::~ChunkManager(){
	delete groundp;
	delete mtmp;
	delete hillsp;
}

bool ChunkManager::checkLoad_helper(){
	bool toload = false;
	if (toload = !isLoaded(lastChunk->x + lx, ly, lastChunk->z + lz))
		chunksToLoad.insert(new Chunk(lastChunk->x + lx, ly, lastChunk->z + lz));
	ly++;
	return toload;
}

void ChunkManager::checkLoad(Camera* camera){
	if (chunksToGenerate.size() > 0)
		return;

	if (chunksToUpdateMesh.size() > 0)
		return;

	for (int i = 0; i < CHUNKS_TO_CHECKLOAD_PER_FRAME; i++){
		bool found = false;

		for (int j = 0; j < 100 && !found && checked < (1 + viewDistance * 2) * (1 + viewDistance * 2) && (inc == 1 || li <= (1 + viewDistance * viewDistance)); j++){
			if (ly < Chunk::CHUNK_HEIGHT){
				found = checkLoad_helper();
			}
			else{
				ly = 0;
				if (lefti == 0){
					if (lx != 0 && lz != 0 && (inc = inc % 2) == 0){
						li++;
					}
					ld = ++ld % 4;
					inc++;
					lefti = min(li, (1 + viewDistance * viewDistance));
				}
				switch (ld){
				case 0:
					lx += 1;
					break;
				case 1:
					lz += 1;
					break;
				case 2:
					lx -= 1;
					break;
				case 3:
					lz -= 1;
					break;
				}
				checked++;
				lefti--;
			}
		}
	}
}

void ChunkManager::frame(Camera* camera){
	Chunk* chunk = forcedChunkAt(camera->x, camera->y, camera->z);

	if (chunk != lastChunk){
		if (lastChunk == NULL || lastChunk->x != chunk->x || lastChunk->z != chunk->z){
			lx = 0;
			ly = 0;
			lz = 0;
			ld = 0;
			li = 1;
			lefti = 0;
			inc = 0;
			checked = 0;
		}
		lastChunk = chunk;
		checkUnload(camera);
	}
	checkLoad(camera);
	load(camera);
	unload(camera);
	generate();
	updateMesh();
	render();
	framecount++;
	if (framecount % 60 * 10 == 0){
		printf("-----------------------\n");
		printf(("framecount:" + std::to_string(framecount) + "\n").c_str());
		printf(("chunksToUpdateMesh:" + std::to_string(chunksToUpdateMesh.size()) + "\n").c_str());
		printf(("chunksToLoad:" + std::to_string(chunksToLoad.size()) + "\n").c_str());
		printf(("chunksToUnload:" + std::to_string(chunksToUnload.size()) + "\n").c_str());
		printf(("chunksToGenerate:" + std::to_string(chunksToGenerate.size()) + "\n").c_str());
		printf(("chunksToRender:" + std::to_string(chunksToRender.size()) + "\n").c_str());
		printf(("pos:{" + std::to_string(camera->x) + ", " + std::to_string(camera->y) + ", " + std::to_string(camera->z) + "}\n").c_str());
		printf(("chunk:{" + std::to_string(lastChunk->x) + ", " + std::to_string(lastChunk->y) + ", " + std::to_string(lastChunk->z) + "}\n").c_str());
	}
}

void ChunkManager::updateNeighbors(Chunk* chunk, bool unload){
	if (unload){
		if (isLoaded(chunk->x + 1, chunk->y, chunk->z)){
			chunks[chunk->x + 1][chunk->y][chunk->z]->xn = NULL;
		}
		if (isLoaded(chunk->x - 1, chunk->y, chunk->z)){
			chunks[chunk->x - 1][chunk->y][chunk->z]->xp = NULL;
		}
		if (isLoaded(chunk->x, chunk->y + 1, chunk->z)){
			chunks[chunk->x][chunk->y + 1][chunk->z]->yn = NULL;
		}
		if (isLoaded(chunk->x, chunk->y - 1, chunk->z)){
			chunks[chunk->x][chunk->y - 1][chunk->z]->yp = NULL;
		}
		if (isLoaded(chunk->x, chunk->y, chunk->z + 1)){
			chunks[chunk->x][chunk->y][chunk->z + 1]->zn = NULL;
		}
		if (isLoaded(chunk->x, chunk->y, chunk->z - 1)){
			chunks[chunk->x][chunk->y][chunk->z - 1]->zp = NULL;
		}
	}
	else{
		if (isLoaded(chunk->x + 1, chunk->y, chunk->z)){
			Chunk* c = chunks[chunk->x + 1][chunk->y][chunk->z];
			c->xn = chunk;
			chunk->xp = c;
			if (c->readyToMesh())
				chunksToUpdateMesh.insert(c);
		}
		if (isLoaded(chunk->x - 1, chunk->y, chunk->z)){
			Chunk* c = chunks[chunk->x - 1][chunk->y][chunk->z];
			c->xp = chunk;
			chunk->xn = c;
			if (c->readyToMesh())
				chunksToUpdateMesh.insert(c);
		}
		if (isLoaded(chunk->x, chunk->y + 1, chunk->z)){
			Chunk* c = chunks[chunk->x][chunk->y + 1][chunk->z];
			c->yn = chunk;
			chunk->yp = c;
			if (c->readyToMesh())
				chunksToUpdateMesh.insert(c);
		}
		if (isLoaded(chunk->x, chunk->y - 1, chunk->z)){
			Chunk* c = chunks[chunk->x][chunk->y - 1][chunk->z];
			c->yp = chunk;
			chunk->yn = c;
			if (c->readyToMesh())
				chunksToUpdateMesh.insert(c);
		}
		if (isLoaded(chunk->x, chunk->y, chunk->z + 1)){
			Chunk* c = chunks[chunk->x][chunk->y][chunk->z + 1];
			c->zn = chunk;
			chunk->zp = c;
			if (c->readyToMesh())
				chunksToUpdateMesh.insert(c);
		}
		if (isLoaded(chunk->x, chunk->y, chunk->z - 1)){
			Chunk* c = chunks[chunk->x][chunk->y][chunk->z - 1];
			c->zp = chunk;
			chunk->zn = c;
			if (c->readyToMesh())
				chunksToUpdateMesh.insert(c);
		}
		if (chunk->readyToMesh())
			chunksToUpdateMesh.insert(chunk);
	}
}

void ChunkManager::forceLoadChunk(const long x, const long y, const long z){
	if (y > 0 && y < Chunk::CHUNK_HEIGHT && !isLoaded(x, y, z)){
		loadChunk(new Chunk(x, y, z));
	}
}

Chunk* ChunkManager::forcedChunkAt(const long x, const long y, const long z){
	long chunkX = x / Chunk::CHUNK_SIZE;
	long chunkY = y / Chunk::CHUNK_SIZE;
	long chunkZ = z / Chunk::CHUNK_SIZE;
	if (x < 0 && fmod(x, Chunk::CHUNK_SIZE) != 0){
		chunkX--;
	}
	if (y < 0 && fmod(y, Chunk::CHUNK_SIZE) != 0){
		chunkY--;
	}
	if (z < 0 && fmod(z, Chunk::CHUNK_SIZE) != 0){
		chunkZ--;
	}
	if (chunkY >= Chunk::CHUNK_HEIGHT){
		chunkY = Chunk::CHUNK_HEIGHT - 1;
	}
	else if (chunkY < 0){
		chunkY = 0;
	}
	if (isLoaded(chunkX, chunkY, chunkZ)){
		return chunks[chunkX][chunkY][chunkZ];
	}
	else{
		Chunk* chunk = new Chunk(chunkX, chunkY, chunkZ);
		loadChunk(chunk);
		return chunk;
	}
}

Chunk* ChunkManager::chunkAt(const long x, const long y, const long z){
	long chunkX = x / Chunk::CHUNK_SIZE;
	long chunkY = y / Chunk::CHUNK_SIZE;
	long chunkZ = z / Chunk::CHUNK_SIZE;
	if (x < 0 && fmod(x, Chunk::CHUNK_SIZE) != 0){
		chunkX--;
	}
	if (y < 0 && fmod(y, Chunk::CHUNK_SIZE) != 0){
		chunkY--;
	}
	if (z < 0 && fmod(z, Chunk::CHUNK_SIZE) != 0){
		chunkZ--;
	}
	if (isLoaded(chunkX, chunkY, chunkZ)){
		return chunks[chunkX][chunkY][chunkZ];
	}
	return NULL;
}

Block* ChunkManager::blockAt(long x, long y, long z){
	Chunk* chunk = chunkAt(x, y, z);

	if (chunk != NULL){
		int blockX = fmod(x, Chunk::CHUNK_SIZE);
		int blockY = fmod(y, Chunk::CHUNK_SIZE);
		int blockZ = fmod(z, Chunk::CHUNK_SIZE);
		if (blockX < 0)
			blockX += Chunk::CHUNK_SIZE;
		if (blockY < 0)
			blockY += Chunk::CHUNK_SIZE;
		if (blockZ < 0)
			blockZ += Chunk::CHUNK_SIZE;

		return &(chunk->blocks[blockX][blockY][blockZ]);
	}
	return NULL;
}

void ChunkManager::setTypeAt(long x, long y, long z, BlockType type){
	Chunk* chunk = chunkAt(x, y, z);

	if (chunk != NULL){
		int blockX = fmod(x, Chunk::CHUNK_SIZE);
		int blockY = fmod(y, Chunk::CHUNK_SIZE);
		int blockZ = fmod(z, Chunk::CHUNK_SIZE);
		if (blockX < 0)
			blockX += Chunk::CHUNK_SIZE;
		if (blockY < 0)
			blockY += Chunk::CHUNK_SIZE;
		if (blockZ < 0)
			blockZ += Chunk::CHUNK_SIZE;

		chunk->blocks[blockX][blockY][blockZ].type = type;
		chunk->modified = true;
		chunk->updateEmpty();
		updateNeighbors(chunk, false);
	}
}

void ChunkManager::updateMesh(){
	for (int i = 0; i < CHUNKS_TO_MESH_PER_FRAME; i++){
		if (!chunksToUpdateMesh.empty()){
			Chunk* chunk = *chunksToUpdateMesh.begin();
			if (chunk->readyToMesh()){
				chunk->updateMesh();
				if (!chunk->mesh->empty || !chunk->ncmesh->empty){
					chunksToRender.insert(chunk);
				}
				else{
					if (chunksToRender.count(chunk) > 0)
					chunksToRender.erase(chunk);
				}
			}
			chunksToUpdateMesh.erase(chunk);
		}
	}
}

bool ChunkManager::isLoaded(long x, long y, long z){
	return chunks.count(x) > 0 && chunks.at(x).count(y) > 0 && chunks.at(x).at(y).count(z) > 0;
}

void ChunkManager::load(Camera* camera){
	for (int i = 0; i < CHUNKS_TO_LOAD_PER_FRAME; i++){
		if (!chunksToLoad.empty()){
			Chunk* chunk = *chunksToLoad.begin();
			loadChunk(chunk);
			chunksToLoad.erase(chunk);
		}
	}
}

void ChunkManager::checkUnload(Camera* camera){
	for (auto const &ent1 : chunks) {
		for (auto const &ent2 : ent1.second) {
			for (auto const &ent3 : ent2.second) {
				Chunk* chunk = ent3.second;
				if (Util::mDist(chunk, lastChunk) > viewDistance){
					chunksToUnload.insert(chunk);
				}
			}
		}
	}
}

void ChunkManager::unload(Camera* camera){
	for (int i = 0; i < CHUNKS_TO_UNLOAD_PER_FRAME; i++){
		bool found = false;
		for (int j = 0; j < 100 && !found; j++){
			if (!chunksToUnload.empty()){
				Chunk* chunk = *chunksToUnload.begin();
				if (Util::mDist(chunk, lastChunk) > viewDistance){
					unloadChunk(chunk);
					found = true;
				}
				chunksToUnload.erase(chunk);
			}
		}
	}
}

void ChunkManager::generate(){
	for (int i = 0; i < CHUNKS_TO_GENERATE_PER_FRAME; i++){
		if (!chunksToGenerate.empty()){
			Chunk* chunk = *chunksToGenerate.begin();
			generateChunk(chunk);
			updateNeighbors(chunk, false);
			chunksToGenerate.erase(chunk);
		}
	}
}

void ChunkManager::render(){
	for (Chunk* chunk : chunksToRender){
		chunk->render();
	}
}

bool ChunkManager::loadChunk(Chunk* chunk){
	if (!IO::loadChunk(chunk)){
		chunksToGenerate.insert(chunk);
	}
	chunks[chunk->x][chunk->y][chunk->z] = chunk;
	updateNeighbors(chunk, false);
	return true;
}

void ChunkManager::unloadChunk(Chunk* chunk){
	if (chunk->modified){
		IO::saveChunk(chunk);
	}
	chunksToUpdateMesh.erase(chunk);
	chunksToLoad.erase(chunk);
	chunksToGenerate.erase(chunk);
	chunksToRender.erase(chunk);
	chunks.at(chunk->x).at(chunk->y).erase(chunk->z);
	updateNeighbors(chunk, true);
	delete chunk;
}

void ChunkManager::generateChunk(Chunk* chunk){
	chunk->empty = true;
	for (int x = 0; x < Chunk::CHUNK_SIZE; x++){
		for (int z = 0; z < Chunk::CHUNK_SIZE; z++){
			long bx = chunk->x * Chunk::CHUNK_SIZE + x + MAX / 2;
			long bz = chunk->z * Chunk::CHUNK_SIZE + z + MAX / 2;
			double ground = groundp->GetValue(double(double(bx) / double(MAX)), double(double(bz) / double(MAX)));
			double mtn = mtmp->GetValue(double(double(bx) / double(MAX)), double(double(bz) / double(MAX)));
			double hills = hillsp->GetValue(double(double(bx) / double(MAX)), double(double(bz) / double(MAX)));
			long my =
				20 * ground
				+ max(0, 50 * mtn)
				+ max(0, 20 * hills)
				+ 30;

			long ry = chunk->y * Chunk::CHUNK_SIZE;
			for (int y = 0; y < Chunk::CHUNK_SIZE && ry < my; y++, ry++){
				if (ry == 0){
					chunk->blocks[x][y][z].type = BEDROCK;
				}
				/*if (cavem->GetValue(double(double(bx) / double(MAX)), double(double(y) / double(Chunk::CHUNK_HEIGHT - 1)), double(double(bz) / double(MAX))) > 0.5)
				chunk->blocks[x][y][z].type = AIR;
				else */else if (mtn > 0.2)
					chunk->blocks[x][y][z].type = STONE;
				else if (ry == my - 1)
					chunk->blocks[x][y][z].type = GRASS;
				else if (ry > my - 6)
					chunk->blocks[x][y][z].type = DIRT;
				else
					chunk->blocks[x][y][z].type = STONE;

				if (chunk->blocks[x][y][z].type != AIR){
					chunk->empty = false;
				}
			}
		}
	}

	chunk->modified = false;
	/*chunk->blocks[0][0][0].type =
	chunk->blocks[Chunk::CHUNK_SIZE-1][0][0].type =
	chunk->blocks[0][Chunk::CHUNK_SIZE-1][0].type =
	chunk->blocks[0][0][Chunk::CHUNK_SIZE-1].type =
	chunk->blocks[Chunk::CHUNK_SIZE-1][Chunk::CHUNK_SIZE - 1][0].type =
	chunk->blocks[0][Chunk::CHUNK_SIZE-1][Chunk::CHUNK_SIZE - 1].type =
	chunk->blocks[Chunk::CHUNK_SIZE-1][0][Chunk::CHUNK_SIZE - 1].type =
	chunk->blocks[Chunk::CHUNK_SIZE - 1][Chunk::CHUNK_SIZE - 1][Chunk::CHUNK_SIZE - 1].type = DIRT;*/
}