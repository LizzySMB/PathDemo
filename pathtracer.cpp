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
    Vector3f d((2.f * (x + 0.5f) / m_width) - 1, 1 - (2.f * (y + 0.5f) / m_height), -1);
    d.normalize();

    Ray r(p, d);
    r = r.transform(invViewMatrix);
    return radiance(r.o, r.d, true, scene);
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

            /* I messed around with gamma usage (not sure if I did it right
             * so it looks a bit different than the brighter expected output
             * but it should look the first expected output without the gamma
            **/

            float gamma = 1/2.2f;

            // Old Mr. Reinhard
            float colorr = (color[0] * (1 + color[0])) / (color[0] + 1.f);
            float colorg = (color[1] * (1 + color[1])) / (color[1] + 1.f);
            float colorb = (color[2] * (1 + color[2])) / (color[2] + 1.f);
            color = Vector3f(colorr, colorg, colorb);
            color = color.array().pow(gamma);
            color = color.cwiseMax(0.f).cwiseMin(1.f);

            int r = static_cast<int>(std::min(255.0f, color.x() * 255.0f));
            int g = static_cast<int>(std::min(255.0f, color.y() * 255.0f));
            int b = static_cast<int>(std::min(255.0f, color.z() * 255.0f));

            imageData[offset] = qRgb(r, g, b);
        }
    }

}

Vector3f PathTracer::radiance(Vector3f& x, Vector3f& w, bool countEmitted, const Scene& scene) {
    IntersectionInfo i;
    Vector3f L = Vector3f(0,0,0);
    Ray r = Ray(x, w);
    if(scene.getIntersection(r, &i)) {
        const Triangle *t = static_cast<const Triangle *>(i.data);//Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = t->getMaterial();//Get the material of the triangle from the mesh
        const tinyobj::real_t *d = mat.diffuse;//Diffuse color as array of floats
        const std::string diffuseTex = mat.diffuse_texname;//Diffuse texture name

        Vector3f negw = -w;

        L = directLighting(i, negw, scene);

        Vector3f diffuse = Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

        Vector3f brdf = diffuse / M_PI;

        // added russian roulette

        float pdf_rr = settings.pathContinuationProb;

        if (distribution(generator) < pdf_rr && !settings.directLightingOnly) {

            Vector3f hitPoint = x + w * i.t;
            Vector3f normal = t->getNormal(i);

            Vector3f wi = sampleNextDir(normal);
            float pdf = std::max(wi.dot(normal), 0.0f) / M_PI;
            bool isIdealSpecular = diffuse[0] < 0.1f && diffuse[1] < 0.1f && diffuse[2] < 0.1f;

            Vector3f Li = radiance(hitPoint, wi, isIdealSpecular, scene);

            float cosine = std::max(wi.dot(normal), 0.0f);

            L += Li.cwiseProduct(brdf) * cosine / (pdf * pdf_rr);
        }

        if (countEmitted) {
            L += Vector3f(mat.emission[0], mat.emission[1], mat.emission[2]);
        }
    }
    return L;
}

Vector3f PathTracer::sampleNextDir(const Vector3f& normal)
{
    float sample1 = distribution(generator);
    float sample2 = distribution(generator);

    float phi = 2.0f * M_PI * sample1;
    float sinTheta = sqrt(sample2);
    float cosTheta = sqrt(1.0f - sample2);

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


Vector3f PathTracer::directLighting(IntersectionInfo i, Vector3f& w, const Scene& scene) {
    Vector3f L = Vector3f(0,0,0);

    // i at surface point

    const Triangle *objTri = static_cast<const Triangle *>(i.data);
    const tinyobj::material_t& surfaceMat = objTri->getMaterial();
    Vector3f diffuse = Vector3f(surfaceMat.diffuse[0], surfaceMat.diffuse[1], surfaceMat.diffuse[2]);
    Vector3f spec = Vector3f(surfaceMat.specular[0], surfaceMat.specular[1], surfaceMat.specular[2]);

    Vector3f brdf = diffuse / M_PI;

    Vector3f normal = objTri->getNormal(i);


    for (Triangle* light: scene.getEmissives()) {
        Vector3f lightVal = Vector3f(0,0,0);

        // sampling random points on the light triangle
        for (int j = 0; j < settings.numDirectLightingSamples; j++) {
            float r1 = distribution(generator);
            float r2 = distribution(generator);

            // work on sampling
            float sqrt_r1 = sqrt(r1);
            float u = 1.0f - sqrt_r1;
            float v = r2 * sqrt_r1;
            float w_bary = 1.0f - u - v;

            // triangle vertices
            Vector3f v0 = light->getVertices()[0];
            Vector3f v1 = light->getVertices()[1];
            Vector3f v2 = light->getVertices()[2];

            // calculate random point + light direction
            Vector3f lightPoint = u * v0 + v * v1 + w_bary * v2;
            Vector3f lightDir = lightPoint - i.hit;
            float distanceToLight = lightDir.norm();
            lightDir.normalize();

            float cosTheta = normal.dot(lightDir);
            if (cosTheta <= 0.0f) {
                continue;
            }

            // shadow check
            Ray shadowRay(i.hit, lightDir);
            IntersectionInfo shadowi;

            bool shadowed = false;
            if (scene.getIntersection(shadowRay, &shadowi)) {
                if (shadowi.t < distanceToLight - 0.001f) {
                    shadowed = true;
                }
            }

            if (!shadowed) {

                Vector3f lightNormal = light->getNormal(i.hit);
                float cosPhiLight = -lightDir.dot(lightNormal);

                if (cosPhiLight > 0.0f) {
                    const tinyobj::material_t& lightMat = light->getMaterial();
                    Vector3f emission = Vector3f(lightMat.emission[0], lightMat.emission[1], lightMat.emission[2]);

                    float lightArea = light->getBBox().surfaceArea();
                    float pdf = (distanceToLight * distanceToLight) / (lightArea * cosPhiLight);

                    lightVal += emission.cwiseProduct(brdf) * cosTheta / pdf;
                }


            }
        }

        L += lightVal / settings.numDirectLightingSamples;
    }
    return L;
}
