#pragma once

#include "core/types.h"
#include <glm/glm.hpp>

namespace hasmet {

enum BxDFType {
    BxDF_Diffuse      = 1 << 0,
    BxDF_Glossy       = 1 << 1,
    BxDF_Specular     = 1 << 2,
    BxDF_Reflection   = 1 << 3,
    BxDF_Transmission = 1 << 4,
    BxDF_All = BxDF_Diffuse | BxDF_Glossy | BxDF_Specular | BxDF_Reflection | BxDF_Transmission
};

class BxDF {
public:
    BxDF(BxDFType type) : type_(type) {}
    virtual ~BxDF() = default;

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const = 0;
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const = 0;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const = 0;

    BxDFType type_;
};

// --- Original Phong ---
class OriginalPhong : public BxDF {
public:
    OriginalPhong(const Color &kd, const Color &ks, float exponent)
        : BxDF(BxDFType(BxDF_Diffuse | BxDF_Glossy)), kd_(kd), ks_(ks), exponent_(exponent) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override;
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override;

private:
    Color kd_, ks_;
    float exponent_;
};

// --- Original Blinn-Phong ---
class OriginalBlinnPhong : public BxDF {
public:
    OriginalBlinnPhong(const Color &kd, const Color &ks, float exponent)
        : BxDF(BxDFType(BxDF_Diffuse | BxDF_Glossy)), kd_(kd), ks_(ks), exponent_(exponent) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override;
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override;

private:
    Color kd_, ks_;
    float exponent_;
};

// --- Modified Phong ---
class ModifiedPhong : public BxDF {
public:
    ModifiedPhong(const Color &kd, const Color &ks, float exponent, bool normalized)
        : BxDF(BxDFType(BxDF_Diffuse | BxDF_Glossy)), kd_(kd), ks_(ks), exponent_(exponent), normalized_(normalized) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override;
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override;

private:
    Color kd_, ks_;
    float exponent_;
    bool normalized_;
};

// --- Modified Blinn-Phong ---
class ModifiedBlinnPhong : public BxDF {
public:
    ModifiedBlinnPhong(const Color &kd, const Color &ks, float exponent, bool normalized)
        : BxDF(BxDFType(BxDF_Diffuse | BxDF_Glossy)), kd_(kd), ks_(ks), exponent_(exponent), normalized_(normalized) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override;
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override;

private:
    Color kd_, ks_;
    float exponent_;
    bool normalized_;
};

// --- Torrance-Sparrow ---
class TorranceSparrowBRDF : public BxDF {
public:
    TorranceSparrowBRDF(const Color &kd, const Color &ks, float exponent, float ior, bool kdfresnel)
        : BxDF(BxDFType(BxDF_Diffuse | BxDF_Glossy)), kd_(kd), ks_(ks), exponent_(exponent), ior_(ior), kdfresnel_(kdfresnel) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override;
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override;

private:
    Color kd_, ks_;
    float exponent_;
    float ior_;
    bool kdfresnel_;
};

// --- Mirror ---
class MirrorBRDF : public BxDF {
public:
    MirrorBRDF(const Color &refl) : BxDF(BxDFType(BxDF_Specular | BxDF_Reflection)), refl_(refl) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override { return Color(0.0f); }
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override { return 0.0f; }

private:
    Color refl_;
};

// --- Dielectric ---
class DielectricBSDF : public BxDF {
public:
    DielectricBSDF(float ior) : BxDF(BxDFType(BxDF_Specular | BxDF_Reflection | BxDF_Transmission)), ior_(ior) {}

    virtual Color evaluate(const Vec3 &wo, const Vec3 &wi) const override { return Color(0.0f); }
    virtual Color sample(const Vec3 &wo, Vec3 &wi, const glm::vec2 &u, float &pdf) const override;
    virtual float pdf(const Vec3 &wo, const Vec3 &wi) const override { return 0.0f; }

private:
    float ior_;
};

} // namespace hasmet