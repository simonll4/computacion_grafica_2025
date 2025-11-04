// -----------------------------------------------------------------------------
//  Archivo: material.hpp
//  Consigna - Requisito 6: definición de al menos tres materiales distintos
//  (difuso, metálico/especular y dieléctrico). El enum `MaterialType` y los
//  parámetros asociados habilitan el comportamiento de reflexión/refracción.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"

// Tipo de material que define el modelo de interacción luz-superficie.
// - Diffuse: refleja la luz de forma lambertiana (mate).
// - Metal: reflexión predominantemente especular; puede tener rugosidad (fuzz).
// - Dielectric: materiales transparentes (vidrio/agua); permiten refracción.
enum class MaterialType
{
  Diffuse,   // Difuso/Lambertiano
  Metal,     // Metálico/Especular conductor
  Dielectric // Dieléctrico/Refractivo
};

struct Material
{
  // Modelo de material a utilizar (difuso, metálico o dieléctrico).
  MaterialType type;

  // Color base del material (reflectancia difusa). Rango esperado [0,1].
  Vec3 albedo;

  // Color de la componente especular (highlights). Útil para metales coloreados.
  Vec3 specularColour;

  // Coeficiente de absorción (Beer-Lambert) por canal RGB para dieléctricos.
  // Valores mayores atenúan más la luz al atravesar el material.
  Vec3 absorption;

  // Coeficientes de iluminación estilo Phong/Blinn-Phong:
  // ka: contribución ambiental (luz ambiente), kd: difusa, ks: especular.
  double ka;
  double kd;
  double ks;

  // Rugosidad para materiales metálicos. 0 = espejo perfecto; >0 = microfacetado.
  // Suele acotarse a [0,1]. Solo aplica a type == Metal.
  double fuzz;

  // Índice de refracción (eta) del material para dieléctricos.
  // Ejemplos: aire≈1.0, agua≈1.33, vidrio≈1.5, diamante≈2.42.
  double refractiveIndex;

  // Exponente de brillo (shininess) del lóbulo especular (Phong).
  // Valores mayores generan highlights más pequeños y concentrados.
  double shininess;

  // Distancia de referencia (en unidades de la escena) para aplicar absorción.
  // Útil para controlar el decaimiento de color en materiales transparentes.
  double absorptionDistance;

  // Constructor por defecto: material difuso gris claro.
  Material()
      : type(MaterialType::Diffuse),
        albedo(0.8, 0.8, 0.8),
        specularColour(1.0, 1.0, 1.0),
        absorption(0.0, 0.0, 0.0),
        ka(0.05), kd(1.0), ks(0.0),
        fuzz(0.0), refractiveIndex(1.0), shininess(32.0), absorptionDistance(0.0) {}

  // Constructor parametrizable para crear materiales específicos:
  // - Difuso:  type=Diffuse,  a=albedo,           kd>0, ks≈0, s no relevante.
  // - Metal:   type=Metal,    a=albedo (tinte),   ks>0, fuzz∈[0,1], s alto.
  // - Dieléc.: type=Dielectric,a=albedo (tinte),  ri≈1.0–2.5, absorption opcional.
  Material(MaterialType t, const Vec3 &a,
           double f = 0.0,                          // rugosidad metálica (fuzz)
           double ri = 1.0,                         // índice de refracción
           double s = 32.0,                         // exponente Phong (shininess)
           double kaVal = 0.05,                     // coeficiente ambiente
           double kdVal = 1.0,                      // coeficiente difuso
           double ksVal = 0.0,                      // coeficiente especular
           const Vec3 &specColour = Vec3(1.0),      // color especular
           const Vec3 &absorptionCoeff = Vec3(0.0), // coef. absorción RGB
           double absorptionDist = 0.0)             // distancia de absorción
      : type(t), albedo(a), specularColour(specColour), absorption(absorptionCoeff),
        ka(kaVal), kd(kdVal), ks(ksVal), fuzz(f), refractiveIndex(ri),
        shininess(s), absorptionDistance(absorptionDist)
  {
  }
};
