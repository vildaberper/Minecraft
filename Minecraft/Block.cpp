#include "Block.h"

Block::Block(){
	type = AIR;
}


Block::~Block(){

}

bool Block::isTransparent(){
	return isTransparent_(type);
}

bool Block::isBlock(){
	return isBlock_(type);
}