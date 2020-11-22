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

struct sphereStruct
{
    float3 position;
    float3 albedo;
    float3 specular;
    float3 radius;
};

RWTexture2D<float4> result : register(u0);

cbuffer camera : register(b1)
{
    cameraStruct camera;
};

cbuffer light : register(b2)
{
    lightStruct light;
};

StructuredBuffer<sphereStruct> spheres : register(b3);

static const float PI = 3.14159265f;

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

void intersectSphere(ray r, inout rayHit bestHit, sphereStruct sphere)
{
    float3 d = r.origin - sphere.position;
    float p1 = -dot(r.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.radius.x * sphere.radius.x;
    if (p2sqr < 0)
    {
        return;
    }
    
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = r.origin + t * r.direction;
        bestHit.normal = normalize(bestHit.position - sphere.position);
        bestHit.albedo = sphere.albedo;
        bestHit.specular = sphere.specular;
    }
}

void intersectCube(ray r, inout rayHit bestHit, float3 cubePos, float3 cubeExtent)
{
    float tMin = 0.0f;
    float tMax = 1.#INF;
    
    for (int i = 0; i < 3; i++)
    {
        if (abs(r.direction[i]) < 0.00001f)
        {
            // parallell to slab. no intersection
            if (r.origin[i] < cubePos[i] || r.origin[i] > cubePos[i] + cubeExtent[i])
            {
                return;
            }
        }
        else
        {
            float ood = 1.0f / r.direction[i];
            float t1 = (cubePos[i] - r.origin[i]) * ood;
            float t2 = ((cubePos[i] + cubeExtent[i]) - r.origin[i]) * ood;
            
            if (t1 > t2)
            {
                float temp = t1;
                t1 = t2;
                t2 = temp;
            }
            
            tMin = max(tMin, t1);
            tMax = min(tMax, t2);
            
            if (tMin > tMax)
            {
                return;
            }
        }
    }

    if (tMin < bestHit.distance)
    {
        bestHit.position = r.origin + r.direction * tMin;
        bestHit.distance = tMin;
        
        float3 cubeOrigin = cubePos + (cubeExtent / 2);
        float3 cubeOriginDir = bestHit.position - cubeOrigin;
        
        // floating point inaccuracies cause issues so we add some bias to the calc for better casting
        float bias = 1.0001f;
        
        bestHit.normal = float3(int3((cubeOriginDir / (cubeExtent / 2)) * bias));
    }
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

rayHit trace(ray r)
{
    rayHit bestHit = createRayHit();
    intersectGroundPlane(r, bestHit);
    
    uint numSpheres = 0;
    uint stride = 0;
    
    spheres.GetDimensions(numSpheres, stride);
    for (uint i = 0; i < numSpheres; i++)
    {
        intersectSphere(r, bestHit, spheres[i]);
    }

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

[numthreads(8,8,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    int width;
    int height;
    result.GetDimensions(width, height);

    float2 uv = float2((id.xy + float2(0.5f, 0.5f)) / float2(width, height) * 2.0f - 1.0f);
    
    ray r = createCameraRay(uv);
   
    float3 shadeResult = float3(0, 0, 0);
    for (int i = 0; i < 8; i++)
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
