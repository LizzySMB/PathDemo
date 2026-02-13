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

    float jitterX = distribution(generator) - 0.5f;
    float jitterY = distribution(generator) - 0.5f;

    // to get rid of jagged edges :)

    Vector3f d((2.f * (x + 0.5f + jitterX) / m_width) - 1,
               1 - (2.f * (y + 0.5f + jitterY) / m_height), -1);


    Ray r(p, d);
    r = r.transform(invViewMatrix);
    return radiance(r.o, r.d, true, scene, 1.f);
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

            float gamma = 1/2.2f;

            // Old Mr. Reinhard
            float colorr = (color[0] * (1 + color[0])) / (color[0] + 1.f);
            float colorg = (color[1] * (1 + color[1])) / (color[1] + 1.f);
            float colorb = (color[2] * (1 + color[2])) / (color[2] + 1.f);
            color = Vector3f(colorr, colorg, colorb);
            color = color.array().pow(gamma);

            int r = static_cast<int>(std::min(255.0f, color.x() * 255.0f));
            int g = static_cast<int>(std::min(255.0f, color.y() * 255.0f));
            int b = static_cast<int>(std::min(255.0f, color.z() * 255.0f));

            imageData[offset] = qRgb(r, g, b);
        }
    }

}

Vector3f PathTracer::radiance(Vector3f& x, Vector3f& w, bool countEmitted, const Scene& scene, float previor) {
    IntersectionInfo i;
    Vector3f L = Vector3f(0,0,0);
    Ray r = Ray(x, w);
    if(scene.getIntersection(r, &i)) {
        const Triangle *t = static_cast<const Triangle *>(i.data);//Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = t->getMaterial();//Get the material of the triangle from the mesh
        const tinyobj::real_t *d = mat.diffuse;//Diffuse color as array of floats
        const std::string diffuseTex = mat.diffuse_texname;//Diffuse texture name

        Vector3f diffuse = Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        Vector3f spec = Vector3f(mat.specular[0], mat.specular[1], mat.specular[2]);
        Vector3f refrac = Vector3f(mat.transmittance[0], mat.transmittance[1], mat.transmittance[2]);
        float ior = mat.ior; // material quality i think

        bool refracts = false;
        bool isIdealSpecular = false;

        float illum = mat.illum;

        if (illum >= 6) {
            refracts = true;
        }
        else if (illum < 6 && illum >= 3) {
            isIdealSpecular = true;
        }


        Vector3f negw = -w;

        if (!isIdealSpecular && !refracts) {
            L = directLighting(i, negw, scene);
        }

        // added russian roulette

        float pdf_rr = settings.pathContinuationProb;

        if (distribution(generator) < pdf_rr && !settings.directLightingOnly) {
            Vector3f brdf;
            float pdf;

            Vector3f hitPoint = x + w * i.t;
            Vector3f normal = t->getNormal(i).normalized();
            Vector3f wi;
            Vector3f Li;

            float cos;

            // refraction check
            if (refracts) {
                Vector3f refracnorm, newDir;
                float ni, nt;
                float nextior;

                if (w.dot(normal) < 0) {
                    refracnorm = normal;
                    ni = 1.f;
                    nt = ior;
                    nextior = ior;
                }
                else {
                    refracnorm = -normal;
                    ni = ior;
                    nt = 1.f;
                    nextior = 1.f;
                }
                float nint = ni/nt;

                // Fresnel through Schlick's approximation
                float costhetai = -w.dot(refracnorm);
                float sin2thetat = nint * nint * (1.0f - costhetai * costhetai);

                float fresnel;

                float R0 = ((ni - nt) / (ni + nt));
                R0 = R0 * R0;

                // percent incoming light reflected vs refracted

                fresnel = R0 + (1.f - R0) * pow(1.f - costhetai, 5.f);


                if (distribution(generator) < fresnel) {
                    newDir = w - 2.0f * w.dot(refracnorm) * refracnorm;
                    newDir.normalize();
                    pdf = fresnel;
                    Vector3f Li = radiance(hitPoint, newDir, true, scene, 1.f);
                    Li = Li.cwiseMin(10.f);
                    L += Li.cwiseProduct(spec) / (pdf * pdf_rr);
                }
                else {
                    Vector3f refracted;
                    float costhetat = sqrt(1.0f - sin2thetat);

                    if (refract(w, refracnorm, nint, refracted)) {

                        Vector3f wi = nint * w + (nint * costhetai - costhetat) * refracnorm;
                        wi.normalize();
                        Vector3f Li = radiance(hitPoint, refracted, true, scene, nextior);

                        Li = Li.cwiseMin(10.f);
                        L += Li.cwiseProduct(Vector3f(1,1,1)) / ((1.0f - fresnel) * pdf_rr);
                    } else {
                        Vector3f wi = nint * w + (nint * costhetai - costhetat) * refracnorm;
                        wi.normalize();
                        Vector3f Li = radiance(hitPoint, wi, true, scene, 1.f);
                        Li = Li.cwiseMin(10.f);
                        L += Li.cwiseProduct(Vector3f(1,1,1)) / pdf_rr;
                    }

                }

            }

            // reflective material
            else if (isIdealSpecular) {
                wi = w - 2.f * w.dot(normal) * normal;
                brdf = spec;
                Li = radiance(hitPoint, wi, true, scene, ior);
                L += Li.cwiseProduct(brdf) / (pdf_rr);
            }
            else if (illum == 2) {

                // other specular glossy notes. split with diffuse from same material by specProb
                float specProb = spec.norm() / (diffuse.norm() + spec.norm());
                // for specular only like in image, uncomment:
                // specProb = 1.f;

                if (distribution(generator) < specProb) {
                    float shininess = mat.shininess;

                    Vector3f reflected = w - 2.f * w.dot(normal) * normal;
                    reflected.normalize();

                    wi = sampleNextDir(reflected, shininess);
                    float cosspec = std::max(0.f, wi.dot(reflected));

                    // phong brdf with importance sampling
                    brdf = spec * (shininess + 2.f) / (2.f * M_PI) * pow(cosspec, shininess);

                    // pdf for specular with importance sampling
                    pdf = (shininess + 1.f) / (2.f * M_PI) * pow(cosspec, shininess);
                    pdf *= specProb;

                    cos = std::max(0.f, wi.dot(normal));

                } else {
                    // diffuse portion of samples
                    wi = sampleNextDir(normal, 0);
                    brdf = diffuse / M_PI;
                    pdf = std::max(wi.dot(normal), 0.0f) / M_PI;
                    pdf *= (1.0f - specProb);
                    cos = wi.dot(normal);
                }

                if (pdf > 0.001f) {
                    Vector3f Li = radiance(hitPoint, wi, false, scene, ior);
                    L += Li.cwiseProduct(brdf) * cos / (pdf * pdf_rr);
                }
            }
            // normal material
            else {
                wi = sampleNextDir(normal, 0);

                brdf = diffuse / M_PI;
                pdf = std::max(wi.dot(normal), 0.0f) / M_PI;

                Li = radiance(hitPoint, wi, false, scene, ior);
                cos = std::max(wi.dot(normal), 0.0f);
                L += Li.cwiseProduct(brdf) * cos / (pdf * pdf_rr);

            }
        }

        if (countEmitted) {
            L += Vector3f(mat.emission[0], mat.emission[1], mat.emission[2]);
        }
    }
    return L;
}

Vector3f PathTracer::sampleNextDir(const Vector3f& normal, float shininess) {
    float sample1 = distribution(generator);
    float sample2 = distribution(generator);

    float phi = 2.0f * M_PI * sample1;
    float cosTheta;
    float sinTheta;
    if (shininess < 0.01f) { // diffuse importance sampling
        cosTheta = sqrt(1.0f - sample2);
        sinTheta = sqrt(sample2);
    }
    else { // specular importance sampling
        cosTheta = pow(sample2, 1.0f / (shininess + 1.0f));
        sinTheta = sqrt(1.f - cosTheta * cosTheta);
    }


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

    Vector3f normal = objTri->getNormal(i).normalized();

    Vector3f brdf = diffuse / M_PI;


    for (Triangle* light: scene.getEmissives()) {
        Vector3f lightVal = Vector3f(0,0,0);


        // triangle vertices
        Vector3f v0 = light->getVertices()[0];
        Vector3f v1 = light->getVertices()[1];
        Vector3f v2 = light->getVertices()[2];

        Vector3f t = ((v1 - v0).cross(v2 - v0));
        Vector3f lightNormal = t.normalized();
        const tinyobj::material_t& lightMat = light->getMaterial();
        Vector3f emission = Vector3f(lightMat.emission[0], lightMat.emission[1], lightMat.emission[2]);



        // sampling random points on the light triangle
        for (int j = 0; j < settings.numDirectLightingSamples; j++) {
            float r1 = distribution(generator);
            float r2 = distribution(generator);

            // work on sampling
            float sqrt_r1 = sqrt(r1);
            float u = 1.0f - sqrt_r1;
            float v = r2 * sqrt_r1;
            float w2 = 1.0f - u - v;

            // calculate random point + light direction
            Vector3f lightPoint = u * v0 + v * v1 + w2 * v2;
            Vector3f lightDir = lightPoint - i.hit;
            float distanceToLight = lightDir.norm();
            lightDir.normalize();

            float cosTheta = normal.dot(lightDir);
            float cosPhiLight = -lightDir.dot(lightNormal);

            if (cosTheta <= 0.f || cosPhiLight <= 0.f) {
                continue;
            }

            // shadow check
            Ray shadowRay(i.hit + normal * 0.0001f, lightDir);
            IntersectionInfo shadowi;

            bool shadowed = false;
            if (scene.getIntersection(shadowRay, &shadowi)) {
                if (shadowi.t < distanceToLight - 0.001f) {
                    shadowed = true;
                }
            }

            if (!shadowed) {

                const tinyobj::material_t& lightMat = light->getMaterial();
                Vector3f emission = Vector3f(lightMat.emission[0], lightMat.emission[1], lightMat.emission[2]);

                // light area calculations
                float lightArea = 0.5f * t.norm();

                float pdf = (distanceToLight * distanceToLight) / (lightArea * cosPhiLight);

                Vector3f totalContribution = Vector3f(0,0,0);


                if (spec.norm() > 0.1f) {
                    float shininess = surfaceMat.shininess;
                    Vector3f reflected = lightDir - 2.0f * lightDir.dot(normal) * normal;
                    reflected.normalize();
                    float speccos = std::max(0.0f, -w.dot(reflected));

                    if (speccos > 0.f) {
                        Vector3f specbrdf = spec * (shininess + 2.0f) / (2.0f * M_PI) * pow(speccos, shininess);
                        totalContribution += emission.cwiseProduct(specbrdf) * cosTheta / pdf;
                    }
                }
                else if (diffuse.norm() > 0.1f) {
                    totalContribution += emission.cwiseProduct(brdf) * cosTheta / pdf;
                }
                lightVal += totalContribution;


            }
        }

        L += lightVal / settings.numDirectLightingSamples;
    }
    return L;
}

bool PathTracer::refract(const Vector3f& wi, const Vector3f& normal, float nint, Vector3f& refracted) {
    // nint is ni/nt from the formula in lecture 6

    float cosi = std::clamp(wi.dot(normal), -1.f, 1.f);
    float sin2t = nint * nint * (1.0f - cosi * cosi);

    // check cosi - if negative, make positive. else swap nint
    if (cosi < 0.f) {
        cosi *= -1.f;
    }
    else {
        //swap nint
    }

    // check for total internal reflection
    if (sin2t >= 1.0f) {
        return false;
    }

    float cosT = sqrt(1.0f - sin2t);

    // Snell's law ni/nt * wi + (ni/nt * costhetai - costhetat) * normal
    refracted = (nint * wi + (nint * cosi - cosT) * normal);
    refracted.normalize();

    return true;
}
