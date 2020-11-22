struct lightStruct
{
    float3 direction;
    float3 intensity;
};

struct cameraStruct
{
    float4x4 cameraToWorld;
    float4x4 inverseCameraProjection;
};

layout(binding = 0) RWTexture2D<float4> result;

layout(binding = 1) cbuffer camera
{
    cameraStruct camera;
};

layout(binding = 2) cbuffer light
{
    lightStruct light;
};

layout(binding = 3) Texture3D<float3> voxels;
layout(binding = 3) SamplerState voxelSampler;

static const float PI = 3.14159265f;
static const int c_maxScale = 32;

struct ray
{
    float3 origin;
    float3 direction;
    float3 energy;
};

struct rayHit
{
    float3 position;
    float3 normal;
    float3 albedo;
    float3 specular;
    float distance;
};

rayHit createRayHit()
{
    rayHit rh;
    rh.position = float3(0.0f);
    rh.normal = float3(0.0f);
    rh.albedo = float3(0.0f);
    rh.specular = float3(0.0f);
    rh.distance = 1.#INF;
    return rh;
}

ray createRay(float3 origin, float3 direction)
{
    ray r;
    r.origin = origin;
    r.direction = direction;
    r.energy = float3(1.0f, 1.0f, 1.0f);
    return r;
}

ray createCameraRay(float2 uv)
{
    float3 origin = mul(camera.cameraToWorld, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    
    float3 direction = mul(camera.inverseCameraProjection, float4(uv, 0.0f, 1.0f)).xyz;
    direction = mul(camera.cameraToWorld, float4(direction, 0.0f)).xyz;
    direction = normalize(direction);

    return createRay(origin, direction);
}


float max(float3 v)
{
    return max(max(v.x, v.y), v.z);
}

float max(float3 a, float b)
{
    return float3(max(a.x, b), max(a.y, b), max(a.z, b));
}

bool2 lessThan(float2 a, float2 b)
{
    return bool2(a.x < b.x, a.y < b.y);
}

bool3 lessThan(float3 a, float3 b)
{
    return bool3(a.x < b.x, a.y < b.y, a.z < b.z);
}

bool3 lessThanEqual(float3 a, float3 b)
{
    return bool3(a.x <= b.x, a.y <= b.y, a.z <= b.z);
}

// Implemented using "A Ray-Box Intersection Algorithm and Efficient Dynamic Voxel Rendering"
// Journal of Computer Graphics Techniques Vol. 7, No. 3, 2018
bool intersectCube(ray r, out float distance, out float3 normal, float3 cubeCentre, float3 cubeRadius, const bool canStartInBox, const in bool oriented, in float3 _invRayDir)
{
    float boxRot = 1.f;
    r.origin = r.origin - cubeCentre;
    
    if (oriented)
    {
        r.direction *= boxRot;
        r.origin *= boxRot;
    }

    float winding = canStartInBox && (max(abs(r.origin) * (1.f / cubeRadius)) < 1.f) ? -1 : 1;
    float3 sign = -sign(r.direction);
    float3 distanceVec = cubeRadius * winding * sign - r.origin;

    if (oriented) 
    {
        distanceVec /= r.direction;
    }
    else
    {
        distanceVec *= _invRayDir;
    }

    #define TEST(U, VW) (distanceVec.U >= 0.f) && all(lessThan(abs(r.origin.VW + r.direction.VW * distanceVec.U), cubeRadius.VW))
    bool3 test = bool3(TEST(x, yz), TEST(y, zx), TEST(z, xy));
    sign = test.x ? float3(sign.x, 0.f, 0.f) : (test.y ? float3(0.f, sign.y, 0.f) : float3(0.f, 0.f, test.z ? sign.z : 0.f));
    #undef TEST

    distance = sign.x ? distanceVec.x : (sign.y ? distanceVec.y : distanceVec.z);
    normal = oriented ? (boxRot * sign) : sign;
    return sign.x || sign.y || sign.z;
}

bool pointInCube(float3 p, float3 cubePos, float3 cubeRadius)
{
    float3 s = step(cubePos - cubeRadius, p) - step(cubePos + cubeRadius, p);
    return (s.x * s.y * s.z) != 0.f;
}

void intersectGroundPlane(ray r, inout rayHit bestHit)
{
    float t = -r.origin.y / r.direction.y;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = r.origin + t * r.direction;
        bestHit.normal = float3(0.0f, 1.0f, 0.0f);
        bestHit.specular = float3(0.1f);
        bestHit.albedo = float3(0.4f);
    }
}


float sdBox(float3 samplePosition, float3 boxExtents)
{
    float3 q = abs(samplePosition) - boxExtents;
    return length(max(q, 0.0)) + min(max(q), 0.0);
}

/*
    1) "Thin" Heightmap where voxels are represented to a depth of N meters on a heightmap
    2) Buildings are represented in a BVH as voxel grids
    3) We raytrace buildings through non voxel space and then do a DDA when we hit a voxel grid
    4) No tunnels or underground natively, but can probably work around that. Make depth  8 meters on heightmap? (800cm?)
*/

void traverseOctree(ray r, inout rayHit bestHit, uint depth)
{

}

void traverseGrid(ray r, inout rayHit bestHit, uint depth)
{
    const float3 gridPosition = float3(0.f, 128.f, -128.f);
    const int3 gridSize = float3(160, 110, 100);
    const float voxelSize = 1.f;

    float distance = 1.#INF;
    float3 normal;

    float3 originalOrigin = r.origin;
    r.origin -= gridPosition;

    float distanceToGrid = sdBox(r.origin, gridSize);
    float3 gridIntersect;
    if (distanceToGrid <= 0)
    {
        gridIntersect = r.origin;
        return;
    }
    else
    {
        if (!intersectCube(r, distance, normal, gridSize / 2.f, gridSize / 2.f, false, false, 1.f / r.direction))
        {
            return;
        }
        gridIntersect = r.origin + distance * r.direction;
    }

    int3 origin = floor(gridIntersect);
    int3 step = sign(r.direction);

    float3 tDelta = abs(float3(length(r.direction)) / r.direction);
    float3 tMax = (sign(r.direction) * (float3(origin) - r.origin) + (sign(r.direction) * 0.5) + 0.5) * tDelta;
    bool3 mask = false;

    bool hit = false;
    for (int i = 0; i < 640000; i++)
    {
        mask = lessThanEqual(tMax.xyz, min(tMax.yzx, tMax.zxy));
        tMax += float3(mask) * tDelta;
        origin += int3(mask) * step;

        float3 texturePos = float3(origin) / float3(gridSize);
        bool outMax = any(lessThan(float3(1.f), texturePos));
        bool outMin = any(lessThan(texturePos, float(0.f)));

        if (outMax || outMin || any(voxels.SampleLevel(voxelSampler, texturePos, 0)))
        {
            hit = any(voxels.SampleLevel(voxelSampler, texturePos, 0));
            break;
        }
    }

    if (hit)
    {
        r.origin = originalOrigin;
        intersectCube(r, distance, normal, gridPosition + float3(origin) + (voxelSize / 2.f), voxelSize / 2.f, true, false, 1.f / r.direction);
        if (distance < bestHit.distance)
        {
            bestHit.distance = distance;
            bestHit.position = r.origin + distance * r.direction;
            bestHit.normal = normal;
            bestHit.specular = voxels.SampleLevel(voxelSampler, float3(origin) / float3(gridSize), 0);
            bestHit.albedo = float3(0.4f);
        }
    }
}

rayHit trace(ray r)
{
    rayHit bestHit = createRayHit();
    intersectGroundPlane(r, bestHit);
    traverseOctree(r, bestHit, 14); // 14 is about 1cm resolution for 128 wide world
    traverseGrid(r, bestHit, 3);
    
    return bestHit;
}

float3 shade(inout ray r, rayHit hit)
{
    if (hit.distance < 1.#INF)
    {   
        r.origin = hit.position + hit.normal * 0.001f;
        r.direction = reflect(r.direction, hit.normal);
        r.energy *= hit.specular;
        
        bool shadow = false;
        ray shadowRay = createRay(hit.position + hit.normal * 0.001f, -light.direction);
        rayHit shadowHit = trace(shadowRay);
        if (shadowHit.distance != 1.#INF)
        {
            return float3(0.0f, 0.0f, 0.0f);
        }

        return saturate(-dot(hit.normal, light.direction)) * light.intensity.x * hit.albedo;
    }
    else
    {   
        r.energy = 0.0f;
        // return skybox texture
        return float3(1.0f - r.direction.x, r.direction.x, r.direction.y);
    }
}

// 1000fps is no code
[numthreads(8,8,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    int width;
    int height;
    result.GetDimensions(width, height);

    float2 uv = float2((id.xy + float2(0.5f, 0.5f)) / float2(width, height) * 2.0f - 1.0f);
    
    ray r = createCameraRay(uv);
   
    float3 shadeResult = float3(0, 0, 0);
    for (int i = 0; i < 2; i++)
    {
        rayHit hit = trace(r);
        shadeResult += r.energy * shade(r, hit);
        if (!any(r.energy))
        {
            break;
        }
    }

    result[id.xy] = float4(shadeResult, 1.0f);
}
