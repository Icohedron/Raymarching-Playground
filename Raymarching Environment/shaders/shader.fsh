#version 330 core

#define FOV 60.0
#define MAX_STEPS 128
#define MAX_RAY_DISTANCE 256.0
#define MIN_PROXIMITY 1e-4
#define SHADOW_RAY_OFFSET 6e-2
#define NORMAL_H 1e-4
#define PENUMBRA_SHADOW_MULTIPLIER 8.0

#define FOG_MULTIPLIER 1e-2
#define FOG_COLOR vec3(0.5, 0.6, 0.7)

// Material IDs
#define PEARL 1.0
#define GOLD 2.0
#define RED_RUBBER 3.0

uniform float time;
uniform vec2 resolution;

uniform vec3 cameraPos;
uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

out vec4 fragColor;

// Materials
// http://devernay.free.fr/cours/opengl/materials.html

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

Material getMaterial(float mID) {
  if (mID <= PEARL) {
    return Material(vec3(0.25, 0.20725, 0.20725),
                    vec3(1.0, 0.829, 0.829),
                    vec3(0.296648, 0.296648, 0.296648),
                    0.088 * 128.0);
  } else if (mID <= GOLD) {
    return Material(vec3(0.24725, 0.1995, 0.0745),
                    vec3(0.75164, 0.60648, 0.22648),
                    vec3(0.628281, 0.555802, 0.366065),
                    0.4 * 128.0);
  } else if (mID <= RED_RUBBER) {
    return Material(vec3(0.05, 0.0, 0.0),
                    vec3(0.5, 0.4, 0.4),
                    vec3(0.7, 0.04, 0.04),
                    0.078125 * 128.0);
  }
  return Material(vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), 10.0);
}

// Distance Fields

float sdMandelbulb(vec3 pos) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < 4 ; i++) {
		r = length(z);
		if (r>4.0) break;

		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr =  pow( r, 8.0-1.0)*8.0*dr + 1.0;

		// scale and rotate the point
		float zr = pow( r,8.0);
		theta = theta*8.0;
		phi = phi*8.0;

		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
}

float sdTorus(vec3 p, vec2 t) {
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdSphere(vec3 p, float r) {
  return length(p) - r;
}

float sdPlane(vec3 p) {
  return p.y;
}

// Distance Field Operators

vec2 opU(vec2 d1, vec2 d2) {
  return d1.x < d2.x ? d1 : d2;
}

// The rest of the code

vec2 scene(vec3 p) {
  vec2 data = vec2(sdMandelbulb(p), PEARL);
  data = opU(data, vec2(sdPlane(p - vec3(0.0, -1.0, 0.0)), RED_RUBBER));
  return data;
}

vec2 march(vec3 ro, vec3 rd, float start) {
  float tdist = start;
  float mID = 0.0;
  for (int i = 0; i < MAX_STEPS; i++) {
    vec2 scData = scene(ro + rd * tdist);
    tdist += scData.x;
    mID = scData.y;
    if (scData.x < MIN_PROXIMITY) break;
  }
  return vec2(tdist, mID);
}

// Lighting Effects

vec3 getNormal(vec3 p) {
  vec2 eps = vec2(NORMAL_H, 0.0);
  return normalize(vec3(scene(p + eps.xyy).x - scene(p - eps.xyy).x,
                        scene(p + eps.yxy).x - scene(p - eps.yxy).x,
                        scene(p + eps.yyx).x - scene(p - eps.yyx).x));
}

float softShadow(vec3 ro, vec3 rd, float mindist, float maxdist) {
  float res = 1.0;
  float tdist = mindist;
  for (int i = 0; i < MAX_STEPS; i++) {
    float dist = scene(ro + rd * tdist).x;
    if (dist < MIN_PROXIMITY) return 0.0;
    res = min(res, PENUMBRA_SHADOW_MULTIPLIER * dist / tdist);
    tdist += dist;
    if (tdist > maxdist) break;
  }
  return res;
}

void applyLighting(inout vec3 color, vec3 sp, vec3 vd, float mID) {
  // Lighting Setup
  vec3 lightPos = vec3(2.0 * sin(time), 2.0, 2.0 * cos(time));
  vec3 lightColor = vec3(0.9, 0.8, 0.75);
  float intensity = 2.0;

  vec3 lightDir = lightPos - sp;
  float distToLight = length(lightDir);
  float attenuation = intensity / distToLight;
  lightDir = normalize(lightDir);

  vec3 normal = getNormal(sp);
  Material material = getMaterial(mID);

  float shadow = softShadow(sp, lightDir, SHADOW_RAY_OFFSET, distToLight);

  vec3 ambient = lightColor * material.ambient;
  vec3 diffuse = lightColor * attenuation * shadow * max(dot(normal, lightDir), 0.0) * material.diffuse;
  vec3 specular = lightColor * attenuation * shadow * pow(max(dot(reflect(-lightDir, normal), vd), 0.0), material.shininess);

  color = color * (ambient + diffuse + specular);
}

void applyFog(inout vec3 color, vec3 fogColor, float dist) {
  float fog = 1.0 - exp(-dist * FOG_MULTIPLIER);
  color = mix(color, fogColor, fog);
}

void main() {
  vec2 uv = gl_FragCoord.xy / resolution.xy;
  uv = 2.0 * uv - 1.0;
  uv.x *= resolution.x / resolution.y;

  float fov = tan(radians(FOV / 2.0));
  uv *= fov;

  vec3 rayOrigin = cameraPos;
  vec3 rayDir = normalize(cameraForward + cameraRight * uv.x + cameraUp * uv.y);

  vec2 scData = march(rayOrigin, rayDir, 0.0);
  float rayMagnitude = scData.x;
  float materialID = scData.y;

  vec3 color = vec3(1.0);

  if (rayMagnitude > MAX_RAY_DISTANCE) {
    applyFog(color, FOG_COLOR, rayMagnitude);
    fragColor = vec4(color, 1.0);
    return;
  }

  vec3 surfacePos = rayOrigin + rayDir * rayMagnitude;
  vec3 viewDir = normalize(rayOrigin - surfacePos);

  applyLighting(color, surfacePos, viewDir, materialID);
  applyFog(color, FOG_COLOR, rayMagnitude);

  fragColor = vec4(color, 1.0);
}
