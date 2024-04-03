#pragma once
#ifndef DEVICE_OBJ_LOADER_H
#define DEVICE_OBJ_LOADER_H
#include <external/OBJ_Loader.h>
#include <common/global.h>
#include <hittable/MeshModel.h>

class OBJLoader {
public:
	static shared_ptr<MeshModel> loadMeshModel(const string& objFilePath) {
		objl::Loader loader;
		bool loadout = loader.LoadFile(objFilePath);
		shared_ptr<MeshModel> model;
		if (loadout) {
			model = make_shared<MeshModel>();
			model->assembleFromObjLoader(loader);
		}
		return model;
	}

};

#endif // OBJ_LOADER_H
