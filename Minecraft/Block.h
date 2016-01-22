#pragma once

#include "Color.h"

static const float TEXTURE_SIZE = 1.0f / 16.0f;

enum Face{
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	FRONT,
	BACK
};

enum BlockType{
	UNDEFINED = -1,

	AIR = 0,
	STONE = 1,
	GRASS = 2,
	DIRT = 3,
	WOOD = 4,
	LEAVES = 5,
	BEDROCK = 6,
	TNT = 7,
	GLASS = 8,
	PLANKS = 9,
	COBBLESTONE = 10
};

static const float blocktype_tx[11] = {
	0, // AIR
	1, // STONE
	3, // GRASS
	2, // DIRT
	4, // WOOD
	4, // LEAVES
	1, // BEDROCK
	8, // TNT
	1, // GLASS
	4, // PLANKS
	0  // COBBLESTONE
};

static const float blocktype_ty[11] = {
	0,  // AIR
	15, // STONE
	15, // GRASS
	15, // DIRT
	14, // WOOD
	12, // LEAVES
	14, // BEDROCK
	15, // TNT
	12, // GLASS
	15, // PLANKS
	14  // COBBLESTONE
};

static const float tx(BlockType type, Face face){
	if (type == GRASS){
		if (face == TOP){
			return 0;
		}
		else if (face == BOTTOM){
			type = DIRT;
		}
	}
	if (type == TNT){
		if (face == TOP){
			return 9 / 16.0f;
		}
		else if (face == BOTTOM){
			return 10 / 16.0f;
		}
	}
	if (type == WOOD){
		if (face == TOP){
			return 5 / 16.0f;
		}
		else if (face == BOTTOM){
			return 5 / 16.0f;
		}
	}
	return blocktype_tx[type] / 16.0f;
}

static const float ty(BlockType type, Face face){
	if (type == GRASS){
		if (face == TOP){
			return 15 / 16.0f;
		}
		else if (face == BOTTOM){
			type = DIRT;
		}
	}
	if (type == TNT){
		if (face == TOP){
			return 15 / 16.0f;
		}
		else if (face == BOTTOM){
			return 15 / 16.0f;
		}
	}
	if (type == WOOD){
		if (face == TOP){
			return 14 / 16.0f;
		}
		else if (face == BOTTOM){
			return 14 / 16.0f;
		}
	}
	return blocktype_ty[type] / 16.0f;
}

static const bool isTransparent_(BlockType type){
	return type == AIR || type == LEAVES || type == GLASS;
}

static const bool isBlock_(BlockType type){
	return type != AIR;
}

class Block{
public:
	static const long RENDER_SIZE = 2;

	Block();
	~Block();

	bool isTransparent();

	bool isBlock();

	BlockType type;
};