#include <common/global.h>
#include <device/Renderer.h>
#include <device/OBJ_Loader.h>
#include <hittable/MeshModel.h>
#include <common/utility.h>
#include <device/shader/NormalShader.h>

int main() {
	
	string objpath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_triangulated.obj";

	objl::Loader loader;
	bool loadout = loader.LoadFile(objpath);
	MeshModel model;
	model.assembleFromObjLoader(loader);
	Mat4 modeling = getModeling(Point3(2, 2, -2), Vec3(1, 1, 1), Vec3(0, 0, 0), 0);
	Renderer renderer;
	NormalShader shader;
	Camera camera;
	renderer.setShader(make_shared<NormalShader>(shader));
	camera.aspect_ratio = 1;
	camera.fov = 45;
	camera.frustum_far = 50;
	camera.frustum_near = 0.1;
	camera.lookfrom = Point3(0, 0, 5);
	camera.lookat = Vec3(0, 0, -1);
	camera.vup = Vec3(0, 1, 0);
	renderer.setCanvasWidth(700);
	renderer.setCanvasAspectRatio(1);
	int idx = renderer.addMeshModel(make_shared<MeshModel>(model));
	renderer.setModelTranslation(idx, make_shared<Mat4>(modeling));
	renderer.setCamera(make_shared<Camera>(camera));
	renderer.setRenderMethod(Renderer::RASTERIZATION);
	renderer.setSSAAMultiple(4);
	renderer.clearBuffer();
	renderer.render();
	renderer.output(std::cout);

}
