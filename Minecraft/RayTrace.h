#pragma once

#include "Camera.h"
#include "Block.h"
#include "ChunkManager.h"
#include "Util.h"

#define RAYTRACE_PRECISION 0.1
// Lower is more accurate

class RayTrace{
public:
	Block* target = NULL;
	Block* beforeTarget = NULL;
	long tx, ty, tz;
	long btx, bty, btz;

	static RayTrace trace(Camera* camera, ChunkManager* chunkmanager){
		RayTrace rt = RayTrace();

		float yrotrad = (camera->yrot / 180 * PI);
		float xrotrad = (camera->xrot / 180 * PI);

		float
			xd = cos(xrotrad) * sin(yrotrad) * RAYTRACE_PRECISION,
			yd = -sin(xrotrad) * RAYTRACE_PRECISION,
			zd = cos(xrotrad) * -cos(yrotrad) * RAYTRACE_PRECISION;

		float
			sx = camera->x,
			sy = camera->y + camera->height,
			sz = camera->z;

		if (sx < 0)
			sx--;
		if (sy < 0)
			sy--;
		if (sz < 0)
			sz--;

		int
			lx = 0,
			ly = 0,
			lz = 0;
		for (int count = 0; count < 50; count++){
			int
				nx = rt.tx = sx + count*xd,
				ny = rt.ty = sy + count*yd,
				nz = rt.tz = sz + count*zd;
			if (lx != nx || ly != ny || lz != nz){
				rt.beforeTarget = rt.target;
				rt.target = chunkmanager->blockAt(rt.tx, rt.ty, rt.tz);
				if ((rt.ty >= 0 && rt.ty < Chunk::CHUNK_SIZE * Chunk::CHUNK_HEIGHT) && (rt.target == NULL || rt.target->type != AIR))
					break;
			}
			rt.btx = lx = nx;
			rt.bty = ly = ny;
			rt.btz = lz = nz;
		}
		return rt;
	}
};