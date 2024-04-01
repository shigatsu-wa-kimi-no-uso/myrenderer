#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
#include <eigen3/Eigen/Eigen>
#include <chrono>
#include <random>
#include <limits>

constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;


using Point3 = Eigen::Vector3d;
using Point3i = Eigen::Vector3i;
using Vec3 = Eigen::Vector3d;
using Vec4 = Eigen::Vector4d;
using Point2 = Eigen::Vector2d;
using Point2i = Eigen::Vector2i;
using Vec2 = Eigen::Vector2d;
using Mat4 = Eigen::Matrix4d;
using Mat3 = Eigen::Matrix3d;
using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using std::make_pair;
using std::pair;
using std::string;

#include <common/Color.h>

#endif // GLOBAL_H
