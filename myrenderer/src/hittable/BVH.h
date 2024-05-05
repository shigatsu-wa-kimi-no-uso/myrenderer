#pragma once
#ifndef HITTABLE_BVH_H
#define HITTABLE_BVH_H
#include <hittable/HittableContainer.h>
#include <common/Bounds3.h>

struct BVHNode {
	shared_ptr<Hittable> hittable; //ΪҶ�ӽڵ�ʱ,���ֶβ�Ϊnull
	Bounds3 bounds;
	double hittableAreaSum = 0;
	shared_ptr<BVHNode> left; //��ΪҶ�ӽڵ�ʱ,left��right��Ϊnull
	shared_ptr<BVHNode> right; //��֤Ҫôleft right��Ϊnull Ҫô����Ϊnull
};


class BVH :public HittableContainer
{
protected:
	HitRecord _getHit(shared_ptr<BVHNode> node, const Ray& r, const Interval& range)const {
		if (node->hittable) {
			HitRecord rec;
			node->hittable->hit(r, range, rec);
			if (rec.hitAnything && !rec.material) {
				rec.material = material; //��ģ���޲���,�ø�ģ�͵�
			}
			return rec;
		}
		HitRecord recL, recR;
		if (node->left->bounds.hasIntersection(r)) {
			recL = _getHit(node->left, r, range);
		}
		if (node->right->bounds.hasIntersection(r)) {
			recR = _getHit(node->right, r, range);
		}
		//������ʱ,tΪ�����,������ʱһ����Ϊ�����
		return recL.t < recR.t ? recL : recR;
	}
	shared_ptr<BVHNode> _root;
	std::vector<shared_ptr<Hittable>> _hittables;

	void _sortHittablesByAxis(size_t begin, size_t end,int axis) {
		std::sort(_hittables.begin() + begin, _hittables.begin() + end, [=](shared_ptr<Hittable> lhs, shared_ptr<Hittable> rhs) {
			return lhs->getBoundingBox().centroid()(axis) <
				rhs->getBoundingBox().centroid()(axis);
			});
	}

	shared_ptr<BVHNode> _recursiveBuild(size_t begin, size_t end) {
		//1)��[begin,end)������ģ�͵İ�Χ�еĲ� end - begin == hittable�ĸ���,��Ϊ1��ΪҶ�ӽڵ�
		//2)д������
		//2)����,����,�ݹ�����Թ���left��right
		shared_ptr<BVHNode> node = make_shared<BVHNode>();
		if (end - begin == 1) {
			node->bounds = _hittables[begin]->getBoundingBox();
			node->hittable = _hittables[begin];
			node->hittableAreaSum = _hittables[begin]->getArea();
			return node;
		}

		for (size_t i = begin; i < end; i++) {
			node->bounds = Bounds3::getUnion(_hittables[i]->getBoundingBox(), node->bounds);
			node->hittableAreaSum += _hittables[i]->getArea();
		}

		_sortHittablesByAxis(begin, end, node->bounds.maxExtent());

		size_t mid = begin + (end - begin) / 2;
		node->left = _recursiveBuild(begin, mid);
		node->right = _recursiveBuild(mid, end);
		return node;
	}

	void _getSample(shared_ptr<BVHNode> node, double p,HitRecord& rec, double& pdf) const {
		if (node->hittable) {
			node->hittable->samplePoint(rec, pdf);
		} else {
			shared_ptr<BVHNode> nextNode;
			if (p > node->left->hittableAreaSum) {
				p -= node->left->hittableAreaSum;
				nextNode = node->right;
			} else {
				nextNode = node->left;
			}
			_getSample(nextNode, p, rec, pdf);
		}
	}

public:
	BVH(){}
	~BVH(){}

	bool hit(const Ray& r, const Interval& range, HitRecord& rec) const {
		HitRecord tempRec = _getHit(_root, r, range);
		if (tempRec.hitAnything) {
			rec = tempRec;
			return true;
		}
		return false;
	}

	void clear() override {
		_hittables.clear();
	}

	void add(shared_ptr<Hittable> object) override {
		_hittables.push_back(object);
	}

	void build() override{
		_root = _recursiveBuild(0, _hittables.size());
	}

	double getArea() const {
		return _root->hittableAreaSum;
	}


	Bounds3 getBoundingBox() const {
		return _root->bounds;
	}

	void samplePoint(HitRecord& rec, double& pdf) const override {
		double p = sqrt(random_double()) * getArea();
		_getSample(_root, p, rec, pdf);
		if (!rec.material) {
			rec.material = material;
		}
		pdf = 1 / getArea();
	}
};


#endif // !HITTABLE_BVH_H
