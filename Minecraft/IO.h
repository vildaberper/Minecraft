#pragma once

#include "Chunk.h"
#include <iostream>
#include <fstream>
#include <string>
#include <future>
#include <sstream>

using namespace std;

class IO{
public:
	static string filename(Chunk* chunk){
		return "world\\" + to_string(chunk->x) + "_" + to_string(chunk->y) + "_" + to_string(chunk->z) + ".chunk";
	}

	static bool fileexists(string fileName){
		ifstream infile(fileName);
		return infile.good();
	}

	static bool saveChunk(Chunk* chunk){
		std::stringstream ss;

		for (int x = 0; x < Chunk::CHUNK_SIZE; x++){
			for (int y = 0; y < Chunk::CHUNK_SIZE; y++){
				for (int z = 0; z < Chunk::CHUNK_SIZE; z++){
					ss << (chunk->empty ? AIR : chunk->blocks[x][y][z].type) << '\n';
				}
			}
		}

		ofstream myfile(filename(chunk));
		if (myfile.is_open())
		{
			myfile << ss.str();
			myfile.close();
		}
		else{
			printf("Unable to save\n");
			return false;
		}
		return true;
	}

	static void load(Chunk* chunk){
		string line;
		ifstream myfile(filename(chunk));

		if (myfile.is_open())
		{
			chunk->empty = true;
			for (int x = 0; x < Chunk::CHUNK_SIZE; x++){
				for (int y = 0; y < Chunk::CHUNK_SIZE; y++){
					for (int z = 0; z < Chunk::CHUNK_SIZE; z++){
						getline(myfile, line);
						BlockType bt = (BlockType)stoi(line, nullptr, 0);
						if (bt != AIR){
							chunk->empty = false;
						}
						chunk->blocks[x][y][z].type = bt;
					}
				}
			}
			myfile.close();
			chunk->modified = true;
		}
	}

	static bool loadChunk(Chunk* chunk){
		if (fileexists(filename(chunk))){
			//future<void> result(std::async(load, chunk));
			load(chunk);
			//result.get();
			return true;
		}
		return false;
	}
};