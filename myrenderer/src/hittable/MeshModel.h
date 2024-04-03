#pragma once
#ifndef HITTABLE_MESH_MODEL_H
#define HITTABLE_MESH_MODEL_H
#include <vector>
#include <hittable/Triangle.h>
#include <external/OBJ_Loader.h>

class MeshModel {
public:
	using MeshList = std::vector<Triangle>;
	MeshList meshList;

	void assembleFromObjLoader(const objl::Loader& loader) {
        for (auto mesh : loader.LoadedMeshes)
        {
            size_t len = mesh.Vertices.size();
            for (int i = 0; i < len; i += 3)
            {
                Triangle t;
                for (int j = 0; j < 3; j++)
                {
                    t.vertices[j] = Vec4(mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z, 1.0);
                    t.normals[j] = Vec4(mesh.Vertices[i + j].Normal.X, mesh.Vertices[i + j].Normal.Y, mesh.Vertices[i + j].Normal.Z,0.0);
                    t.texCoords[j] = Vec2(mesh.Vertices[i + j].TextureCoordinate.X, mesh.Vertices[i + j].TextureCoordinate.Y);
                }
                //TODO: ÉèÖÃ²ÄÖÊ£¡
                // 
                //
                meshList.push_back(t);
            }
        }
	}
};

#endif // MESH_MODEL_H
