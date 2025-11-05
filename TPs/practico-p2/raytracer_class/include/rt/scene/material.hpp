// -----------------------------------------------------------------------------
// Definición de tipos de material y parámetros de shading
// -----------------------------------------------------------------------------
//  Requisito 6 (consigna): mínimo tres materiales distintos
//   - Difuso: superficie mate (Lambert)
//   - Metal: reflexión especular con rugosidad
//   - Dieléctrico: refracción + reflexión (Fresnel)
//
//  Parámetros Phong:
//   ka, kd, ks: coeficientes ambiente, difuso, especular
//   shininess: concentración del highlight (exponente Phong)
//
//  Parámetros específicos:
//   fuzz: rugosidad del metal [0,1]
//   refractiveIndex: η del dieléctrico (1.0=aire, 1.5=vidrio, 2.42=diamante)
//   absorption: atenuación Beer-Lambert por canal RGB
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"

// Tipo de interacción luz-superficie
enum class MaterialType
{
  Diffuse,   // Difuso/Lambertiano (mate)
  Metal,     // Especular con rugosidad
  Dielectric // Transparente con refracción
};

struct Material
{
  MaterialType type;

  Vec3 albedo;         // Color base (reflectancia difusa)
  Vec3 specularColour; // Color del highlight especular
  Vec3 absorption;     // Coeficiente Beer-Lambert (dieléctricos)

  double ka; // Coeficiente ambiente (Phong)
  double kd; // Coeficiente difuso (Phong)
  double ks; // Coeficiente especular (Phong)

  double fuzz;               // Rugosidad metálica [0,1]
  double refractiveIndex;    // Índice de refracción (η)
  double shininess;          // Exponente Phong (concentra highlight)
  double absorptionDistance; // Distancia de absorción (Beer-Lambert)

  // Constructor por defecto: difuso gris claro
  Material()
      : type(MaterialType::Diffuse),
        albedo(0.8, 0.8, 0.8),
        specularColour(1.0, 1.0, 1.0),
        absorption(0.0, 0.0, 0.0),
        ka(0.05), kd(1.0), ks(0.0),
        fuzz(0.0), refractiveIndex(1.0), shininess(32.0), absorptionDistance(0.0) {}

  // Constructor parametrizable
  Material(MaterialType t, const Vec3 &a,
           double f = 0.0,
           double ri = 1.0,
           double s = 32.0,
           double kaVal = 0.05,
           double kdVal = 1.0,
           double ksVal = 0.0,
           const Vec3 &specColour = Vec3(1.0),
           const Vec3 &absorptionCoeff = Vec3(0.0),
           double absorptionDist = 0.0)
      : type(t), albedo(a), specularColour(specColour), absorption(absorptionCoeff),
        ka(kaVal), kd(kdVal), ks(ksVal), fuzz(f), refractiveIndex(ri),
        shininess(s), absorptionDistance(absorptionDist)
  {
  }
};
