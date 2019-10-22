#pragma once
#include "libs.h"

#include "MAP.h"
#include <vector>

using namespace std;

struct CachedImage {
	double top;
	double bot;
	double left;
	double right;
	CImage* image;
};

class LIBSAPI CacheManager {
	private:
		vector<CachedImage> *cache;
		double sensitivity;
	public:
		CacheManager();
		CacheManager(double sensitivity);
		bool hasImage(double top, double bot, double left, double right);
		CImage* getImage(double top, double bot, double left, double right);
		void storeImage(CImage* image, double top, double bot, double left, double right);
};