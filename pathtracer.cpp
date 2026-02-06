#include "pathtracer.h"

#include <iostream>

#include <Eigen/Dense>

#include <util/Common.h>

#include <random>

using namespace Eigen;

namespace {
    thread_local std::mt19937 generator;
    thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
}

PathTracer::PathTracer(int width, int height)
    : m_width(width), m_height(height)
{
}

void PathTracer::traceScene(QRgb *imageData, const Scene& scene)
{
    std::vector<Vector3f> intensityValues(m_width * m_height);
    Matrix4f invViewMat = (scene.getCamera().getScaleMatrix() * scene.getCamera().getViewMatrix()).inverse();
    for(int y = 0; y < m_height; ++y) {
        //#pragma omp parallel for
        for(int x = 0; x < m_width; ++x) {
            int offset = x + (y * m_width);
            Vector3f color = Vector3f(0,0,0);
            for(int s = 0; s < settings.samplesPerPixel; ++s) {
                color += tracePixel(x, y, scene, invViewMat);
            }

            intensityValues[offset] = color / (settings.samplesPerPixel);
        }
    }
    toneMap(imageData, intensityValues);
}

Vector3f PathTracer::tracePixel(int x, int y, const Scene& scene, const Matrix4f &invViewMatrix)
{
    Vector3f p(0, 0, 0);
    Vector3f d((2.f * x / m_width) - 1, 1 - (2.f *  y / m_height), -1);
    d.normalize();

    Ray r(p, d);
    r = r.transform(invViewMatrix);
    return radiance(r.o, r.d, scene);
}

Vector3f PathTracer::traceRay(const Ray& r, const Scene& scene)
{
    IntersectionInfo i;
    Ray ray(r);
    if(scene.getIntersection(ray, &i)) {
          //** Example code for accessing materials provided by a .mtl file **
        const Triangle *t = static_cast<const Triangle *>(i.data);//Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = t->getMaterial();//Get the material of the triangle from the mesh
        const tinyobj::real_t *d = mat.diffuse;//Diffuse color as array of floats
        const std::string diffuseTex = mat.diffuse_texname;//Diffuse texture name

        if (mat.emission[0] > 0 || mat.emission[1] > 0 || mat.emission[2] > 0) {
            return Vector3f(mat.emission[0], mat.emission[1], mat.emission[2]);
        }

        Vector3f icoords = ray.o + ray.d * i.t;


        return Vector3f(1,1,1);
    } else {
        return Vector3f(0,0,0);
    }
}

void PathTracer::toneMap(QRgb *imageData, std::vector<Vector3f> &intensityValues) {
    for(int y = 0; y < m_height; ++y) {
        for(int x = 0; x < m_width; ++x) {
            int offset = x + (y * m_width);
            Vector3f color = intensityValues[offset];

            // Old Mr. Reinhard
            float colorr = color[0] / (color[0] + 1.f);
            float colorg = color[1] / (color[1] + 1.f);
            float colorb = color[2] / (color[2] + 1.f);
            color = Vector3f(colorr, colorg, colorb);

            int r = static_cast<int>(std::min(255.0f, color.x() * 255.0f));
            int g = static_cast<int>(std::min(255.0f, color.y() * 255.0f));
            int b = static_cast<int>(std::min(255.0f, color.z() * 255.0f));

            imageData[offset] = qRgb(r, g, b);
        }
    }

}

Vector3f PathTracer::radiance(Vector3f& x, Vector3f& w, const Scene& scene) {
    IntersectionInfo i;
    Vector3f L = Vector3f(0,0,0);
    if(scene.getIntersection(Ray(x, w), &i)) {
        const Triangle *t = static_cast<const Triangle *>(i.data);//Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = t->getMaterial();//Get the material of the triangle from the mesh
        L = Vector3f(mat.emission[0], mat.emission[1], mat.emission[2]);
        const tinyobj::real_t *d = mat.diffuse;//Diffuse color as array of floats
        const std::string diffuseTex = mat.diffuse_texname;//Diffuse texture name

        Vector3f diffuse = Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

        // hard coding diffuse for milestone spheres - comment out otherwise
        if (diffuse[0] < 0.1f && diffuse[1] < 0.1f && diffuse[2] < 0.1f) {
            diffuse = Vector3f(1.0f, 1.0f, 1.0f);
        }

        Vector3f hitPoint = x + w * i.t;
        Vector3f normal = t->getNormal(i);

        Vector3f wi = sampleNextDir(normal);
        float pdf = std::max(wi.dot(normal), 0.0f) / M_PI;

        Vector3f brdf = diffuse / M_PI;

        Vector3f Li = radiance(hitPoint, wi, scene);

        float cosine = std::max(wi.dot(normal), 0.0f);

        L += Li.cwiseProduct(brdf) * cosine / pdf;

       // wi, pdf = sampleNextDir()
      //  L += radiance(i.hit, wi) * p.brdf(wi, w) * dot(wi, p.N) / pdf
    }
    return L;
}

Vector3f PathTracer::sampleNextDir(const Vector3f& normal)
{
    float sample1 = distribution(generator);
    float sample2 = distribution(generator);

    float phi = 2.0f * M_PI * sample1;
    float sinTheta = sqrt(sample2);
    float cosTheta = sqrt(1.0f - sinTheta);

    Vector3f nextDir(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);

    Vector3f tanx, tany;
    if(fabs(normal.x()) > 0.9f) {
        tanx = Vector3f(0.0f, 1.0f, 0.0f);
    } else {
        tanx = Vector3f(1.0f, 0.0f, 0.0f);
    }
    tanx = tanx.cross(normal).normalized();
    tany = normal.cross(tanx);

    return (nextDir.x() * tanx + nextDir.y() * tany + nextDir.z() * normal).normalized();
}
