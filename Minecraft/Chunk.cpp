#include "Chunk.h"

Chunk::Chunk(){
	allocblocks();
	modified = false;
	mesh = new Mesh();
}

Chunk::Chunk(long x, long y, long z){
	allocblocks();
	modified = false;
	mesh = new Mesh();
	ncmesh = new Mesh();
	Chunk::x = x;
	Chunk::y = y;
	Chunk::z = z;
}

Chunk::~Chunk(){
	deleteblocks();
	delete mesh;
	delete ncmesh;
}

void Chunk::allocblocks(){
	blocks = new Block**[CHUNK_SIZE];
	for (int i = 0; i < CHUNK_SIZE; i++){
		blocks[i] = new Block*[CHUNK_SIZE];
		for (int j = 0; j < CHUNK_SIZE; j++){
			blocks[i][j] = new Block[CHUNK_SIZE];
		}
	}
}

void Chunk::deleteblocks(){
	for (int i = 0; i < CHUNK_SIZE; i++){
		for (int j = 0; j < CHUNK_SIZE; j++){
			delete[] blocks[i][j];
		}
		delete[] blocks[i];
	}
	delete[] blocks;
}

void Chunk::updateEmpty(){
	for (int x = 0; x < CHUNK_SIZE; x++){
		for (int y = 0; y < CHUNK_SIZE; y++){
			for (int z = 0; z < CHUNK_SIZE; z++){
				if (blocks[x][y][z].type != AIR){
					empty = false;
					return;
				}
			}
		}
	}
	empty = true;
}

void Chunk::update(double dt){

}

void Chunk::render(){
	if (mesh->empty && ncmesh->empty)
		return;

	glPushMatrix();
	glTranslatef(
		x * CHUNK_SIZE * Block::RENDER_SIZE,
		y * CHUNK_SIZE * Block::RENDER_SIZE,
		z * CHUNK_SIZE * Block::RENDER_SIZE
		);
	if (!mesh->empty){
		mesh->render();
	}
	if (!ncmesh->empty){
		glDisable(GL_CULL_FACE);
		ncmesh->render();
		glEnable(GL_CULL_FACE);
	}
	glPopMatrix();
}

bool Chunk::isFaceVisible(int x, int y, int z, Face f){
	if (f == TOP)
		return transparentAt(x, y + 1, z);
	if (f == BOTTOM)
		return transparentAt(x, y - 1, z);

	if (f == LEFT)
		return transparentAt(x - 1, y, z);
	if (f == RIGHT)
		return transparentAt(x + 1, y, z);

	if (f == BACK)
		return transparentAt(x, y, z - 1);
	if (f == FRONT)
		return transparentAt(x, y, z + 1);
	return false;
};

bool Chunk::readyToMesh(){
	return xp != NULL
		&& xn != NULL
		&& (yp != NULL || y == Chunk::CHUNK_HEIGHT - 1)
		&& (yn != NULL || y == 0)
		&& zp != NULL
		&& zn != NULL;
}

void Chunk::updateMesh(){
	if (empty){
		mesh->empty = true;
		mesh->rebind = true;
		ncmesh->empty = true;
		ncmesh->rebind = true;
		return;
	}

	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;
	std::vector<float> texcs;
	int vi = 0;

	std::vector<float> ncvertices;
	std::vector<unsigned int> ncindices;
	std::vector<float> ncnormals;
	std::vector<float> nctexcs;
	int ncvi = 0;

	int tmpx, tmpy, tmpz;
	int i, j, k, l, h, w, u, v, n, r, s, t;

	Face face;
	int x[] {0, 0, 0};
	int q[] {0, 0, 0};
	int du[] {0, 0, 0};
	int dv[] {0, 0, 0};

	BlockType mask[Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE];

	for (bool backFace = true, b = false; b != backFace; backFace = backFace && b, b = !b) {
		for (int d = 0; d < 3; d++) {
			u = (d + 1) % 3;
			v = (d + 2) % 3;

			x[0] = 0; x[1] = 0; x[2] = 0;
			q[0] = 0; q[1] = 0; q[2] = 0; q[d] = 1;

			if (d == 0) {
				face = backFace ? RIGHT : LEFT;
			}
			else if (d == 1) {
				face = backFace ? TOP : BOTTOM;
			}
			else if (d == 2) {
				face = backFace ? FRONT : BACK;
			}
			for (x[d] = 0; x[d] < Chunk::CHUNK_SIZE; x[d]++) {
				n = 0;
				for (x[v] = 0; x[v] < Chunk::CHUNK_SIZE; x[v]++) {
					for (x[u] = 0; x[u] < Chunk::CHUNK_SIZE; x[u]++) {
						tmpx = x[0];
						tmpy = x[1];
						tmpz = x[2];
						mask[n++] = isFaceVisible(tmpx, tmpy, tmpz, face) ? blocks[tmpx][tmpy][tmpz].type : AIR;
					}
				}

				n = 0;
				for (j = 0; j < Chunk::CHUNK_SIZE; j++) {
					for (i = 0; i < Chunk::CHUNK_SIZE;) {
						if (mask[n] != 0) {
							for (w = 1; w + i < Chunk::CHUNK_SIZE && mask[n + w] != 0 && mask[n + w] == mask[n]; w++) {}

							bool done = false;
							for (h = 1; j + h < Chunk::CHUNK_SIZE; h++) {
								for (k = 0; k < w; k++) {
									if (mask[n + k + h * Chunk::CHUNK_SIZE] == 0 || mask[n + k + h * Chunk::CHUNK_SIZE] != mask[n]) {
										done = true;
										break;
									}
								}
								if (done) break;
							}
							x[u] = i;
							x[v] = j;
							du[0] = 0; du[1] = 0; du[2] = 0; du[u] = w;
							dv[0] = 0; dv[1] = 0; dv[2] = 0; dv[v] = h;
							if (!backFace) {
								r = x[0];
								s = x[1];
								t = x[2];
							}
							else {
								r = x[0] + q[0];
								s = x[1] + q[1];
								t = x[2] + q[2];
							}

							float x0 = Block::RENDER_SIZE * r;
							float x1 = Block::RENDER_SIZE * (r + du[0] + dv[0]);
							float y0 = Block::RENDER_SIZE * s;
							float y1 = Block::RENDER_SIZE * (s + du[1] + dv[1]);
							float z0 = Block::RENDER_SIZE * t;
							float z1 = Block::RENDER_SIZE * (t + du[2] + dv[2]);

							float x = tx(mask[n], face);
							float y = ty(mask[n], face);

							std::vector<float>* cvertices = &vertices;
							std::vector<unsigned int>* cindices = &indices;
							std::vector<float>* cnormals = &normals;
							std::vector<float>* ctexcs = &texcs;
							int* cvi = &vi;


							if (isTransparent_(mask[n])){
								cvertices = &ncvertices;
								cindices = &ncindices;
								cnormals = &ncnormals;
								ctexcs = &nctexcs;
								cvi = &ncvi;
							}

							if (face == TOP){
								x += (z1 - z0) / Block::RENDER_SIZE - 1;
								y += (x1 - x0) / Block::RENDER_SIZE - 1;
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
								ctexcs->push_back(x);
								ctexcs->push_back(y);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
							}
							else if (face == BOTTOM){
								x += (z1 - z0) / Block::RENDER_SIZE - 1;
								y += (x1 - x0) / Block::RENDER_SIZE - 1;
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y);
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
							}
							else if (face == LEFT){
								if (isBlock_(mask[n]) && isTransparent_(mask[n])){
									y += (y1 - y0) / Block::RENDER_SIZE - 1;
									x += (z1 - z0) / Block::RENDER_SIZE - 1;
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y);
									ctexcs->push_back(x);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y);
									ctexcs->push_back(x);
									ctexcs->push_back(y);
								}
								else{
									y += (y1 - y0) / Block::RENDER_SIZE - 1;
									x += (z1 - z0) / Block::RENDER_SIZE - 1;
									ctexcs->push_back(x);
									ctexcs->push_back(y);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x);
									ctexcs->push_back(y);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y);
								}
							}
							else if (face == RIGHT){
								y += (y1 - y0) / Block::RENDER_SIZE - 1;
								x += (z1 - z0) / Block::RENDER_SIZE - 1;
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
							}
							else if (face == FRONT){
								if (isBlock_(mask[n]) && isTransparent_(mask[n])){
									y += (y1 - y0) / Block::RENDER_SIZE - 1;
									x += (x1 - x0) / Block::RENDER_SIZE - 1;
									ctexcs->push_back(x);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x);
									ctexcs->push_back(y);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y);
									ctexcs->push_back(x);
									ctexcs->push_back(y);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y + TEXTURE_SIZE);
								}
								else{
									y += (y1 - y0) / Block::RENDER_SIZE - 1;
									x += (x1 - x0) / Block::RENDER_SIZE - 1;
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x);
									ctexcs->push_back(y + TEXTURE_SIZE);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y);
									ctexcs->push_back(x);
									ctexcs->push_back(y);
									ctexcs->push_back(x + TEXTURE_SIZE);
									ctexcs->push_back(y);
									ctexcs->push_back(x);
									ctexcs->push_back(y + TEXTURE_SIZE);
								}
							}
							else{
								y += (y1 - y0) / Block::RENDER_SIZE - 1;
								x += (x1 - x0) / Block::RENDER_SIZE - 1;
								ctexcs->push_back(x);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y);
								ctexcs->push_back(x + TEXTURE_SIZE);
								ctexcs->push_back(y + TEXTURE_SIZE);
								ctexcs->push_back(x);
								ctexcs->push_back(y);
							}

							switch (face) {
							case TOP:
								for (int no = 0; no < 6; no++){
									cnormals->push_back(0);
									cnormals->push_back(1);
									cnormals->push_back(0);
								}
								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);


								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);
								break;
							case BOTTOM:
								for (int no = 0; no < 6; no++){
									cnormals->push_back(0);
									cnormals->push_back(-1);
									cnormals->push_back(0);
								}
								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);


								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);
								break;
							case LEFT:
								for (int no = 0; no < 6; no++){
									cnormals->push_back(-1);
									cnormals->push_back(0);
									cnormals->push_back(0);
								}
								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);


								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);
								break;
							case RIGHT:
								for (int no = 0; no < 6; no++){
									cnormals->push_back(1);
									cnormals->push_back(0);
									cnormals->push_back(0);
								}
								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);


								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);
								break;
							case FRONT:
								for (int no = 0; no < 6; no++){
									cnormals->push_back(0);
									cnormals->push_back(0);
									cnormals->push_back(1);
								}
								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);


								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z1);
								cindices->push_back((*cvi)++);
								break;
							case BACK:
								for (int no = 0; no < 6; no++){
									cnormals->push_back(0);
									cnormals->push_back(0);
									cnormals->push_back(-1);
								}
								cvertices->push_back(x0);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);


								cvertices->push_back(x1);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x1);
								cvertices->push_back(y0);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);

								cvertices->push_back(x0);
								cvertices->push_back(y1);
								cvertices->push_back(z0);
								cindices->push_back((*cvi)++);
								break;
							}

							for (l = 0; l < h; ++l) {
								for (k = 0; k < w; ++k) {
									mask[n + k + l * Chunk::CHUNK_SIZE] = AIR;
								}
							}
							i += w;
							n += w;
						}
						else {
							i++;
							n++;
						}
					}
				}
			}
		}
	}

	if (!(mesh->empty = vertices.size() == 0)){
		mesh->vertices_ = new GLfloat[(mesh->vertices_size = vertices.size())];
		for (unsigned int i = 0; i < vertices.size(); i++)
			mesh->vertices_[i] = vertices[i];
		vertices.clear();

		mesh->indices_ = new unsigned int[(mesh->indices_size = indices.size())];
		for (unsigned int i = 0; i < indices.size(); i++)
			mesh->indices_[i] = indices[i];
		indices.clear();

		mesh->normals_ = new GLfloat[(mesh->normals_size = normals.size())];
		for (unsigned int i = 0; i < normals.size(); i++)
			mesh->normals_[i] = normals[i];
		normals.clear();

		mesh->texcs_ = new GLfloat[(mesh->texcs_size = texcs.size())];
		for (unsigned int i = 0; i < texcs.size(); i++)
			mesh->texcs_[i] = texcs[i];
		texcs.clear();
	}
	mesh->rebind = true;

	if (!(ncmesh->empty = ncvertices.size() == 0)){
		ncmesh->vertices_ = new GLfloat[(ncmesh->vertices_size = ncvertices.size())];
		for (unsigned int i = 0; i < ncvertices.size(); i++)
			ncmesh->vertices_[i] = ncvertices[i];
		ncvertices.clear();

		ncmesh->indices_ = new unsigned int[(ncmesh->indices_size = ncindices.size())];
		for (unsigned int i = 0; i < ncindices.size(); i++)
			ncmesh->indices_[i] = ncindices[i];
		ncindices.clear();

		ncmesh->normals_ = new GLfloat[(ncmesh->normals_size = ncnormals.size())];
		for (unsigned int i = 0; i < ncnormals.size(); i++)
			ncmesh->normals_[i] = ncnormals[i];
		ncnormals.clear();

		ncmesh->texcs_ = new GLfloat[(ncmesh->texcs_size = nctexcs.size())];
		for (unsigned int i = 0; i < nctexcs.size(); i++)
			ncmesh->texcs_[i] = nctexcs[i];
		nctexcs.clear();
	}
	ncmesh->rebind = true;
}

bool Chunk::transparentAt(int x, int y, int z){
	if (x < 0){
		return xn->transparentAt(CHUNK_SIZE + x, y, z);
	}
	else if (x >= CHUNK_SIZE){
		return xp->transparentAt(x - CHUNK_SIZE, y, z);
	}
	else if (y < 0){
		if (Chunk::y == 0)
			return false;
		return yn->transparentAt(x, CHUNK_SIZE + y, z);
	}
	else if (y >= CHUNK_SIZE){
		if (Chunk::y == Chunk::CHUNK_HEIGHT - 1)
			return true;
		return yp->transparentAt(x, y - CHUNK_SIZE, z);
	}
	else if (z < 0){
		return zn->transparentAt(x, y, CHUNK_SIZE + z);
	}
	else if (z >= CHUNK_SIZE){
		return zp->transparentAt(x, y, z - CHUNK_SIZE);
	}

	return blocks[x][y][z].isTransparent();
}

bool Chunk::isBlockAt(int x, int y, int z){
	if (x < 0){
		return xn->isBlockAt(CHUNK_SIZE + x, y, z);
	}
	else if (x >= CHUNK_SIZE){
		return xp->isBlockAt(x - CHUNK_SIZE, y, z);
	}
	else if (y < 0){
		if (Chunk::y == 0)
			return false;
		return yn->isBlockAt(x, CHUNK_SIZE + y, z);
	}
	else if (y >= CHUNK_SIZE){
		if (Chunk::y == Chunk::CHUNK_HEIGHT - 1)
			return false;
		return yp->isBlockAt(x, y - CHUNK_SIZE, z);
	}
	else if (z < 0){
		return zn->isBlockAt(x, y, CHUNK_SIZE + z);
	}
	else if (z >= CHUNK_SIZE){
		return zp->isBlockAt(x, y, z - CHUNK_SIZE);
	}

	return blocks[x][y][z].isBlock();
}