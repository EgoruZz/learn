#ifndef GEOMETRY_D_CPP
#define GEOMETRY_D_CPP

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#ifndef INSIDE_GEOMETRY_D
#define INSIDE_GEOMETRY_D
#include "../c/c.cpp"
#endif

struct AnalyticGeometry3D : AnalyticGeometry2D {

double dist_point_plane(double A, double B, double C, double D, const Vec3& p) {
    return fabs(A * p.x + B * p.y + C * p.z + D) / hypot(hypot(A, B), C);
}

double dist_skew_lines(Vec3 r0, Vec3 d1, Vec3 s0, Vec3 d2) {
    double dx = s0.x - r0.x, dy = s0.y - r0.y, dz = s0.z - r0.z;
    Vec3 cx = cross_product(d1, d2);
    double denom = sqrt(cx.x*cx.x + cx.y*cx.y + cx.z*cx.z);
    if (fabs(denom) < EPS) return 0;
    return fabs(dx * cx.x + dy * cx.y + dz * cx.z) / denom;
}

tuple<double,double,double,double> plane_through_3_points(const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 n = cross_product({b.x - a.x, b.y - a.y, b.z - a.z},
                           {c.x - a.x, c.y - a.y, c.z - a.z});
    double D = -(n.x * a.x + n.y * a.y + n.z * a.z);
    return {n.x, n.y, n.z, D};
}

Vec3 line_intersection_direction(double A1, double B1, double C1,
                                  double A2, double B2, double C2) {
    return cross_product({A1, B1, C1}, {A2, B2, C2});
}

double volume_sphere_3d(double r) {
    return 4.0 / 3.0 * M_PI * r * r * r;
}

double volume_cylinder_3d(double r, double h) {
    return M_PI * r * r * h;
}

double volume_cone_3d(double r, double h) {
    return M_PI * r * r * h / 3.0;
}

double volume_ellipsoid_3d(double a, double b, double c) {
    return 4.0 / 3.0 * M_PI * a * b * c;
}

double sphere_surface_area(double r) {
    return 4.0 * M_PI * r * r;
}

double cylinder_volume(double r, double h) {
    return M_PI * r * r * h;
}

double cone_volume(double r, double h) {
    return M_PI * r * r * h / 3.0;
}

}; // struct AnalyticGeometry3D

#endif // GEOMETRY_D_CPP
