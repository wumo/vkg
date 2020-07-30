#ifndef VKG_ATMOSPHERE_LIGHTING_H
#define VKG_ATMOSPHERE_LIGHTING_H

#ifdef RADIANCE_API_ENABLED
RadianceSpectrum GetSolarRadiance() {
  return ATMOSPHERE.solar_irradiance /
         (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius);
}
RadianceSpectrum GetSkyRadiance(
  Position camera, Direction view_ray, Length shadow_length, Direction sun_direction,
  out DimensionlessSpectrum transmittance) {
  return GetSkyRadiance(
    ATMOSPHERE, transmittanceTex, scatteringTex, camera, view_ray, shadow_length,
    sun_direction, transmittance);
}
RadianceSpectrum GetSkyRadianceToPoint(
  Position camera, Position point, Length shadow_length, Direction sun_direction,
  out DimensionlessSpectrum transmittance) {
  return GetSkyRadianceToPoint(
    ATMOSPHERE, transmittanceTex, scatteringTex, camera, point, shadow_length,
    sun_direction, transmittance);
}
IrradianceSpectrum GetSunAndSkyIrradiance(
  Position p, Direction normal, Direction sun_direction,
  out IrradianceSpectrum sky_irradiance) {
  return GetSunAndSkyIrradiance(
    ATMOSPHERE, transmittanceTex, irradianceTex, p, normal, sun_direction,
    sky_irradiance);
}
#endif

Luminance3 GetSolarLuminance() {
  return ATMOSPHERE.solar_irradiance /
         (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius) *
         SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminance(
  Position camera, Direction view_ray, Length shadow_length, Direction sun_direction,
  out DimensionlessSpectrum transmittance) {
  return GetSkyRadiance(
           ATMOSPHERE, transmittanceTex, scatteringTex, camera, view_ray, shadow_length,
           sun_direction, transmittance) *
         SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminanceToPoint(
  Position camera, Position point, Length shadow_length, Direction sun_direction,
  out DimensionlessSpectrum transmittance) {
  return GetSkyRadianceToPoint(
           ATMOSPHERE, transmittanceTex, scatteringTex, camera, point, shadow_length,
           sun_direction, transmittance) *
         SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Illuminance3 GetSunAndSkyIlluminance(
  Position p, Direction normal, Direction sun_direction,
  out IrradianceSpectrum sky_irradiance) {
  IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(
    ATMOSPHERE, transmittanceTex, irradianceTex, p, normal, sun_direction,
    sky_irradiance);
  sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
  return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}

//#define USE_LUMINANCE
//#ifdef USE_LUMINANCE
//  #define GetSolarRadiance GetSolarLuminance
//  #define GetSkyRadiance GetSkyLuminance
//  #define GetSkyRadianceToPoint GetSkyLuminanceToPoint
//  #define GetSunAndSkyIrradiance GetSunAndSkyIlluminance
//#endif

vec3 atmosphereLight(vec3 pos, vec3 normal, vec3 eye) {
  vec3 sky_irradiance;
  vec3 sun_irradiance = GetSunAndSkyIlluminance(
    pos - earth_center.xyz, normal, sun_direction.xyz, sky_irradiance);
  vec3 radiance = (sun_irradiance + sky_irradiance);
  //  vec3 transmittance;
  //  vec3 in_scatter = GetSkyLuminanceToPoint(
  //    eye - earth_center.xyz, pos - earth_center.xyz, 0, sun_direction.xyz, transmittance);
  //  radiance = radiance * transmittance + in_scatter;
  return sun_intensity * (vec3(1.0) - exp(-radiance / white_point.rgb * exposure));
}

vec3 skyBackground(vec3 eye, vec3 viewDir) {
  vec3 p = eye - earth_center.xyz;
  float p_dot_v = dot(p, viewDir);
  float p_dot_p = dot(p, p);
  float ray_earth_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
  float distance_to_intersection =
    -p_dot_v - sqrt(earth_center.w * earth_center.w - ray_earth_center_squared_distance);

  float ground_alpha = 0.0;
  vec3 ground_radiance = vec3(0.0);
  if(distance_to_intersection > 0.0) {
    vec3 point = cam.eye.xyz + viewDir * distance_to_intersection;
    vec3 normal = normalize(point - earth_center.xyz);

    // Compute the radiance reflected by the ground.
    vec3 sky_irradiance;
    vec3 sun_irradiance = GetSunAndSkyIlluminance(
      point - earth_center.xyz, normal, sun_direction.xyz, sky_irradiance);
    const vec3 kGroundAlbedo = vec3(0.0, 0.0, 0.04);
    ground_radiance = kGroundAlbedo * (1.0 / PI) * (sun_irradiance + sky_irradiance);

    vec3 transmittance;
    vec3 in_scatter = GetSkyLuminanceToPoint(
      p, point - earth_center.xyz, 0, sun_direction.xyz, transmittance);
    ground_radiance = ground_radiance * transmittance + in_scatter;
    ground_alpha = 1.0;
  }

  vec3 transmittance;
  vec3 radiance = GetSkyLuminance(p, viewDir, 0, sun_direction.xyz, transmittance);

  radiance = mix(radiance, ground_radiance, ground_alpha);

  // If the view ray intersects the Sun, add the Sun radiance.
  if(dot(viewDir, sun_direction.xyz) > sun_size.y) {
    radiance = radiance + transmittance * GetSolarLuminance();
  }
//  return sun_intensity * (vec3(1.0) - exp(-radiance / white_point.rgb * exposure));
  return (vec3(1.0) - exp(-radiance / white_point.rgb * exposure));
}

#endif //VKG_ATMOSPHERE_LIGHTING_H
