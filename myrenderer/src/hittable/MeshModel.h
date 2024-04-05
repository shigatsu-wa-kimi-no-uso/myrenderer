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

    void calculateTangent(Triangle& t) {
        for (int i = 0; i < 3; i++) {
            Vec3 e1 = toVec3(t.vertices[(i+1)%3] - t.vertices[i]);
            Vec3 e2 = toVec3(t.vertices[(i + 2) % 3] - t.vertices[i]);
            Vec2 delta2 = t.texCoords[(i + 2) % 3] - t.texCoords[i];
            Vec2 delta1 = t.texCoords[(i + 1) % 3] - t.texCoords[i];
            double ratio = 1.0 / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);
            Vec3 tangent = ratio * (delta2[1] * e1 - delta1[1] * e2);
            Vec3 n = toVec3(t.normals[i]).normalized();
            t.tangents[i] = (tangent - tangent.dot(n) * n).normalized(); //正交化
        }
    }

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
                //TODO: 设置材质！如果文件中有（或者使用全局材质）
                // 
                //
                calculateTangent(t); //本应从文件中读取,由于文件中没有这里先计算
                meshList.push_back(t);
            }
        }
	}
};

#endif // MESH_MODEL_H
