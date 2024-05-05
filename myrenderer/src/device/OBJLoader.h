#pragma once
#ifndef DEVICE_OBJ_LOADER_H
#define DEVICE_OBJ_LOADER_H
#include <external/OBJ_Loader.h>
#include <common/global.h>
#include <hittable/MeshModel.h>

class OBJLoader {
public:
	static shared_ptr<MeshModel> loadMeshModel(const string& objFilePath) {
		std::clog << "loading " << objFilePath << "...";
		objl::Loader loader;
		bool loadout = loader.LoadFile(objFilePath);
		shared_ptr<MeshModel> model;
		if (loadout) {
			model = make_shared<MeshModel>();
			MeshModel::assembleFromObjLoader(*model,loader);
		}
		std::clog << "ok\n";
		return model;
	}

	static void loadMeshModel(const string& objFilePath, shared_ptr<MeshModel>& model) {

		model = loadMeshModel(objFilePath);
	}


	static void loadMeshModel(const string& objFilePath, shared_ptr<MeshModel_BVH>& model) {

		model = loadMeshModel_BVH(objFilePath);
	}


	static shared_ptr<MeshModel_BVH> loadMeshModel_BVH(const string& objFilePath) {
		std::clog << "loading " << objFilePath << "...";
		objl::Loader loader;
		bool loadout = loader.LoadFile(objFilePath);
		shared_ptr<MeshModel_BVH> model;
		if (loadout) {
			model = make_shared<MeshModel_BVH>();
			MeshModel_BVH::assembleFromObjLoader(*model, loader);
		}
		std::clog << "ok\n";
		return model;
	}

};

#endif // OBJ_LOADER_H
