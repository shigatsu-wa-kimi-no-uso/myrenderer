#include <common/global.h>
#include <device/Renderer.h>
#include <device/OBJLoader.h>
#include <hittable/MeshModel.h>
#include <common/utility.h>
#include <device/shader/NormalShader.h>
#include <device/shader/TextureShader.h>
#include <device/shader/CheckerTextureShader.h>
#include <texture/ImageTexture.h>
#include <material/BlinnPhongWithBump.h>
#include <device/shader/BumpTextureShader.h>
#include <texture/CheckerTexture.h>
#include <material/BRDF/BRDFLambertian.h>
#include <material/BRDF/Luminary.h>

void setCameraConfig1(Camera& camera) {
	camera.aspectRatio = 1;
	camera.fov = 45;
	camera.frustumFar = -50;	//基于lookat的远近值
	camera.frustumNear = -0.1;
	camera.lookfrom = Point3(0, 0, 5);
	//camera.lookfrom = Point3(1, 2, 5);
	camera.lookat = (Point3(0, -1, 0)  - camera.lookfrom).normalized();
	//camera.lookfrom = Point3(0, 1, 5);
	//camera.lookat = Vec3(0, -0.5, -1);
//	camera.vup = toVec3(getRotation(Vec3(1, 0, 0), 90) * toVec4(camera.lookat, 0));
	camera.vup = getOrthoVec(camera.lookat).normalized();
	if (camera.vup.y() < 0) {
		camera.vup = -camera.vup;
	}
	//camera.vup = Vec3(0, 6, -2).normalized();//toVec3(getRotation(Vec3(1, 0, 0), 90) * toVec4(camera.lookat, 0));
}


shared_ptr<MeshModel> getParallelogram(const Vec3& u,const Vec3& v,const Point3& q) {
	shared_ptr<MeshModel> mod = make_shared<MeshModel>();
	Point3 a = q + u;
	Point3 b = q + v;
	Point3 c = q + u + v;
	Triangle t1;
	t1.vertices[0] = toVec4(q,1);  //q
	t1.vertices[1] = toVec4(a, 1); //a
	t1.vertices[2] = toVec4(b, 1); //b
	t1.normals[0] = toVec4((u).cross(v), 0);
	t1.normals[1] = t1.normals[0];
	t1.normals[2] = t1.normals[0];
	Triangle t2 = t1;
	t2.vertices[0] = toVec4(c,1);
	t2.normals[2] = t1.normals[0];
	mod->add(make_shared<Triangle>(t1));
	mod->add(make_shared<Triangle>(t2));
	return mod;
}



std::unordered_map<string, int> shaders;
std::unordered_map<string, int> materials;
std::unordered_map<string, shared_ptr<Texture>> textures;




void registerAssetsConfig1(Renderer& renderer) {
	string texturePath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_texture.png";
	string bumpTexturePath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/hmap.jpg";

	shaders["bump"] = renderer.addShader(make_shared<BumpTextureShader>());
	shaders["normal"] = renderer.addShader(make_shared<NormalShader>());
	shaders["texture"] = renderer.addShader(make_shared<TextureShader>());
	shaders["checker"] = renderer.addShader(make_shared<CheckerTextureShader>());

	textures["spot"] = make_shared<ImageTexture>(texturePath);
	textures["bump_spot"] = make_shared<ImageTexture>(bumpTexturePath);
	textures["checker"] = make_shared<CheckerTexture>(Color255(255, 174, 201), Color255(255, 127, 39));

	materials["checker"] = renderer.addMaterial(make_shared<BlinnPhong>(ColorN(0.005, 0.005, 0.005), ColorN(0.7937, 0.7937, 0.7937), 150,textures["checker"]));
	shared_ptr<BlinnPhong> blinnPhong = make_shared<BlinnPhong>(ColorN(0.005, 0.005, 0.005), ColorN(0.7937, 0.7937, 0.7937), 150, textures["spot"]);
	materials["blinn_phong_spot"] = renderer.addMaterial(blinnPhong);
	materials["blinn_phong_with_bump_spot"] = renderer.addMaterial(make_shared<BlinnPhongWithBump>(*blinnPhong, textures["bump_spot"]));

}


void addSpot(Renderer& renderer) {
	string objPath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_triangulated_good.obj";
	shared_ptr<MeshModel> model = OBJLoader::loadMeshModel(objPath);

	int spot_id = renderer.addMeshModel(model);

	renderer.bindModelProperties(spot_id, shaders["texture"], materials["blinn_phong_spot"]);
	Mat4 modeling = getModeling(Point3(1, -1, -2), Vec3(1, 1, 1), Vec3(0, 1, 0), 140);
	renderer.setModelTransform(spot_id, make_shared<Mat4>(modeling));
}

void addChecker(Renderer& renderer) {
	shared_ptr<MeshModel> checker = getParallelogram(Vec3(5, 0, 0), Vec3(0, 0, -5), Point3(0, 0, 0));
	int checker_id = renderer.addMeshModel(checker);
	Mat4 modeling2 = getModeling(Point3(-3, -2, 0), Vec3(1, 1, 1), Vec3(0, 1, 0), 0);
	renderer.bindModelProperties(checker_id, shaders["checker"],materials["checker"]);
	renderer.setModelTransform(checker_id, make_shared<Mat4>(modeling2));
}

void sceneConfig1(Renderer& renderer,Camera& camera) {
	setCameraConfig1(camera);
	renderer.setCamera(make_shared<Camera>(camera));
	Light l1;
	l1.position = Point3(8, 3, 4);
	l1.intensity = Vec3(150, 150, 150);
	l1.width = 24;
	l1.aspectRatio = 1;
	l1.maxDistance = 25;
	l1.shadowMap.width = 1400;
	l1.shadowMap.height = 1400;
	l1.direction = (Vec3(0, -1, 0) - l1.position).normalized();
	l1.up = getOrthoVec(l1.direction);

	Light l2 = l1;
	l2.position = Point3(4, 3, 4);
	l2.intensity = Vec3(250, 250, 250);
	l2.direction = (Vec3(0, -1, 0) - l2.position).normalized();
	l2.width = 16;
	l2.up = getOrthoVec(l2.direction);
	//l2.up = toVec3(getRotation(Vec3(1, 0, 0), 90) * toVec4(l2.direction, 0));
	renderer.addLight(l1);
	renderer.addLight(l2);

	renderer.setCanvasWidth(700);
	renderer.setCanvasAspectRatio(1);

	registerAssetsConfig1(renderer);
	addChecker(renderer);
	addSpot(renderer);

	renderer.setRenderingMethod(Renderer::RASTERIZATION);
	renderer.setRendererConfig(Renderer::WITH_SHADOW);
	renderer.setSSAAMultiple(4);
}

void closeStdLog() {
	std::ostringstream nullStream;
	std::clog.rdbuf(nullStream.rdbuf());
}



void setCameraConfig2(Camera& camera) {
	camera.aspectRatio = 1;
	camera.defocusAngle = 0;
	camera.fov = 40;
	camera.focusDist = 1;
	camera.lookfrom = Point3(278, 273, -800);
	camera.lookat = Vec3(278, 273, 0);
	camera.vup = getOrthoVec( camera.lookat - camera.lookfrom ).normalized();
	if (camera.vup.y() < 0) {
		camera.vup = -camera.vup;
	}
	camera.initialize();
}

void sceneConfig2(Renderer& renderer, Camera& camera) {
	setCameraConfig2(camera);
	renderer.setCamera(make_shared<Camera>(camera));
	renderer.setCanvasWidth(784);
	renderer.setCanvasAspectRatio(1);
	/*
	shared_ptr<MeshModel_BVH> floor;
	shared_ptr<MeshModel_BVH> shortbox;
	shared_ptr<MeshModel_BVH> tallbox;
	shared_ptr<MeshModel_BVH> left;
	shared_ptr<MeshModel_BVH> right;
	shared_ptr<MeshModel_BVH> light;
	shared_ptr<MeshModel_BVH> spot;
	*/
	shared_ptr<MeshModel> floor;
	shared_ptr<MeshModel> shortbox;
	shared_ptr<MeshModel> tallbox;
	shared_ptr<MeshModel> left;
	shared_ptr<MeshModel> right;
	shared_ptr<MeshModel> light;
	shared_ptr<MeshModel> spot;

	OBJLoader::loadMeshModel("models/cornellbox/floor.obj", floor);
	OBJLoader::loadMeshModel("models/cornellbox/shortbox.obj",shortbox);
	OBJLoader::loadMeshModel("models/cornellbox/tallbox.obj", tallbox);
	OBJLoader::loadMeshModel("models/cornellbox/left.obj",left);
	OBJLoader::loadMeshModel("models/cornellbox/right.obj", right);
	OBJLoader::loadMeshModel("models/cornellbox/light.obj",light);
	OBJLoader::loadMeshModel("models/spot/spot_triangulated_good.obj", spot);
	shared_ptr<Material> green = make_shared<BRDFLambertian>(ColorN(0.14, 0.45, 0.091));
	shared_ptr<Material> red = make_shared<BRDFLambertian>(ColorN(0.63, 0.065, 0.05));
	shared_ptr<Material> white = make_shared<BRDFLambertian>(ColorN(0.725, 0.71, 0.68));
	Vec3 emit = 8.0f * Vec3(0.747 + 0.058, 0.747 + 0.258, 0.747) 
		+ 15.6f * Vec3(0.740 + 0.287, 0.740 + 0.160, 0.740) 
		+ 18.4f * Vec3(0.737 + 0.642, 0.737 + 0.159, 0.737);
	shared_ptr<Material> luminary = make_shared<Luminary>(emit);
	shared_ptr<Material> texture_spot = make_shared<BRDFLambertian>(make_shared<ImageTexture>("models/spot/spot_texture.png"));


	floor->material = white;
	shortbox->material = white;
	tallbox->material = white;
	left->material = red;
	right->material = green;
	light->material = luminary;
	spot->material = texture_spot;
	int floor_id = renderer.addMeshModel(floor);
	int shortbox_id = renderer.addMeshModel(shortbox);
	int tallbox_id = renderer.addMeshModel(tallbox);
	int left_id = renderer.addMeshModel(left);
	int right_id = renderer.addMeshModel(right);
	int light_id = renderer.addMeshModel(light);
	int spot_id = renderer.addMeshModel(spot);
	renderer.setSamplesPerPixel(8);
	renderer.setRenderingMethod(Renderer::RASTERIZATION);
	renderer.setRendererConfig(Renderer::USE_BVH);
}

int main() {
	//closeStdLog();
	Renderer renderer;
	Camera camera;

	sceneConfig2(renderer, camera);

	renderer.render();
	renderer.output(std::cout);

}
