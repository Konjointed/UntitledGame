#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

struct Texture {
	unsigned int mId;
	std::string mType;
	std::string mFilepath;
};

void LoadTexture(const std::string& path, const std::string& name);

#endif 