#pragma once
#ifndef HITTABLE_MESH_MODEL_H
#define HITTABLE_MESH_MODEL_H
#include <vector>
#include <hittable/Triangle.h>
#include <hittable/BVH.h>
#include <external/OBJ_Loader.h>
#include <hittable/HittableList.h>


class MeshModel : public HittableList {
public:
    std::vector<shared_ptr<Triangle>>& meshList = *reinterpret_cast<std::vector<shared_ptr<Triangle>>*>(&_hittables);

    static void assembleFromObjLoader(MeshModel& model, const objl::Loader& loader) {
        for (auto mesh : loader.LoadedMeshes)
        {
            size_t len = mesh.Vertices.size();
            for (int i = 0; i < len; i += 3)
            {
                shared_ptr<Triangle> t = make_shared<Triangle>();
                for (int j = 0; j < 3; j++)
                {
                    t->vertices[j] = Vec4(mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z, 1.0);
                    t->normals[j] = Vec4(mesh.Vertices[i + j].Normal.X, mesh.Vertices[i + j].Normal.Y, mesh.Vertices[i + j].Normal.Z, 0.0);
                    t->texCoords[j] = Vec2(mesh.Vertices[i + j].TextureCoordinate.X, mesh.Vertices[i + j].TextureCoordinate.Y);
                }
                //TODO: ���ò��ʣ�����ļ����У�����ʹ��ȫ�ֲ��ʣ�
                // 
                //
                t->calculateOthers(); //�����������Լ�����������
                model.add(t);
                //���а�Χ��,����Ҫ�����Χ��(BVH::build())
                model.build();
            }
        }
    }
};


class MeshModel_BVH : public BVH {
public:
	static void assembleFromObjLoader(MeshModel_BVH& model,const objl::Loader& loader) {
        for (auto mesh : loader.LoadedMeshes)
        {
            size_t len = mesh.Vertices.size();
            for (int i = 0; i < len; i += 3)
            {
                shared_ptr<Triangle> t = make_shared<Triangle>();
                for (int j = 0; j < 3; j++)
                {
                    t->vertices[j] = Vec4(mesh.Vertices[i + j].Position.X, mesh.Vertices[i + j].Position.Y, mesh.Vertices[i + j].Position.Z, 1.0);
                    t->normals[j] = Vec4(mesh.Vertices[i + j].Normal.X, mesh.Vertices[i + j].Normal.Y, mesh.Vertices[i + j].Normal.Z,0.0);
                    t->texCoords[j] = Vec2(mesh.Vertices[i + j].TextureCoordinate.X, mesh.Vertices[i + j].TextureCoordinate.Y);
                }
                //TODO: ���ò��ʣ�����ļ����У�����ʹ��ȫ�ֲ��ʣ�
                // 
                //
                t->calculateOthers(); //�����������Լ�����������
                model.add(t); 
                //���а�Χ��,����Ҫ�����Χ��(BVH::build())
                model.build();
            }
        }
	}

};

#endif // MESH_MODEL_H
