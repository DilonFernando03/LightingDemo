// --------------------------------------------------------------------------------------
// File: GeometricObject.cpp
//
// Adapted from files in the Microsoft DirectX Toolkit to use the SimpleMath library
// and simplify the usage.  
// 
// Normals are always returned set to (0, 0, 0) since it is expected that the normals 
// will be calculated.
// 
// Parts copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "GeometricObject.h"
#include "teapot.h"

inline void CheckIndexOverflow(size_t value)
{
    // Use >=, not > comparison, because some D3D level 9_x hardware does not support 0xFFFF index values.
    if (value >= USHRT_MAX)
        throw std::out_of_range("Index value out of range: cannot tesselate primitive so finely");
}

inline void IndexPushBack(vector<UINT>& indices, size_t value)
{
    CheckIndexOverflow(value);
    indices.push_back(static_cast<unsigned int>(value));
}


// Helper for flipping winding of geometric primitives for LH vs. RH coords
inline void ReverseWinding(vector<UINT>& indices, vector<ObjectVertexStruct>& vertices)
{
    assert((indices.size() % 3) == 0);
    for (auto it = indices.begin(); it != indices.end(); it += 3)
    {
        std::swap(*it, *(it + 2));
    }
}

//--------------------------------------------------------------------------------------
// Cube (or Box)
//--------------------------------------------------------------------------------------

void ComputeBox(vector<ObjectVertexStruct>& vertices, vector<UINT>& indices, const Vector3& size)
{
    vertices.clear();
    indices.clear();

    // A box has six faces, each one pointing in a different direction.
    constexpr int FaceCount = 6;

    static const XMVECTORF32 faceNormals[FaceCount] =
    {
        { { {  0,  0,  1, 0 } } },
        { { {  0,  0, -1, 0 } } },
        { { {  1,  0,  0, 0 } } },
        { { { -1,  0,  0, 0 } } },
        { { {  0,  1,  0, 0 } } },
        { { {  0, -1,  0, 0 } } },
    };

    XMVECTOR tsize = XMLoadFloat3(&size);
    tsize = XMVectorDivide(tsize, g_XMTwo);

    // Create each face in turn.
    for (int i = 0; i < FaceCount; i++)
    {
        const XMVECTOR normal = faceNormals[i];

        // Get two vectors perpendicular both to the face normal and to each other.
        const XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

        const XMVECTOR side1 = XMVector3Cross(normal, basis);
        const XMVECTOR side2 = XMVector3Cross(normal, side1);

        // Six indices (two triangles) per face.
        const size_t vbase = vertices.size();
        IndexPushBack(indices, vbase + 2);
        IndexPushBack(indices, vbase + 1);
        IndexPushBack(indices, vbase + 0);

        IndexPushBack(indices, vbase + 3);
        IndexPushBack(indices, vbase + 2);
        IndexPushBack(indices, vbase + 0);

        // Four vertices per face.
        // (normal - side1 - side2) * tsize // normal // t0
        ObjectVertexStruct vertex;
        vertex.Position = XMVectorMultiply(XMVectorSubtract(XMVectorSubtract(normal, side1), side2), tsize);
        vertex.Normal = Vector3(0, 0, 0);
        vertex.TextureCoordinate = Vector2(0.0f, 0.0f);
        vertices.push_back(vertex);

        // (normal - side1 + side2) * tsize // normal // t1
        vertex.Position = XMVectorMultiply(XMVectorAdd(XMVectorSubtract(normal, side1), side2), tsize);
        vertex.Normal = Vector3(0, 0, 0);
        vertex.TextureCoordinate = Vector2(0.0f, 1.0f);
        vertices.push_back(vertex);

        // (normal + side1 + side2) * tsize // normal // t2
        vertex.Position = XMVectorMultiply(XMVectorAdd(normal, XMVectorAdd(side1, side2)), tsize);
        vertex.Normal = Vector3(0, 0, 0);
        vertex.TextureCoordinate = Vector2(1.0f, 1.0f);
        vertices.push_back(vertex);

        // (normal + side1 - side2) * tsize // normal // t3
        vertex.Position = XMVectorMultiply(XMVectorSubtract(XMVectorAdd(normal, side1), side2), tsize);
        vertex.Normal = Vector3(0, 0, 0);
        vertex.TextureCoordinate = Vector2(1.0f, 0.0f);
        vertices.push_back(vertex);
    }
}


//--------------------------------------------------------------------------------------
// Sphere
//--------------------------------------------------------------------------------------
void ComputeSphere(vector<ObjectVertexStruct>& vertices, vector<UINT>& indices, float diameter, size_t tessellation)
{
    vertices.clear();
    indices.clear();

    if (tessellation < 3)
        throw std::invalid_argument("tesselation parameter must be at least 3");

    const size_t verticalSegments = tessellation;
    const size_t horizontalSegments = tessellation * 2;

    const float radius = diameter / 2;

    // Create rings of vertices at progressively higher latitudes.
    for (size_t i = 0; i <= verticalSegments; i++)
    {
        const float v = 1 - float(i) / float(verticalSegments);

        const float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
        float dy, dxz;

        XMScalarSinCos(&dy, &dxz, latitude);

        // Create a single ring of vertices at this latitude.
        for (size_t j = 0; j <= horizontalSegments; j++)
        {
            const float u = float(j) / float(horizontalSegments);

            const float longitude = float(j) * XM_2PI / float(horizontalSegments);
            float dx, dz;

            XMScalarSinCos(&dx, &dz, longitude);

            dx *= dxz;
            dz *= dxz;

            ObjectVertexStruct vertex;
            vertex.Position = Vector3(dx * radius, dy * radius, dz * radius);
            vertex.Normal = Vector3(0, 0, 0);
            vertices.push_back(vertex);
        }
    }

    // Fill the index buffer with triangles joining each pair of latitude rings.
    const size_t stride = horizontalSegments + 1;

    for (size_t i = 0; i < verticalSegments; i++)
    {
        for (size_t j = 0; j <= horizontalSegments; j++)
        {
            const size_t nextI = i + 1;
            const size_t nextJ = (j + 1) % stride;

            IndexPushBack(indices, i * stride + nextJ);
            IndexPushBack(indices, nextI * stride + j);
            IndexPushBack(indices, i * stride + j);

            IndexPushBack(indices, nextI * stride + nextJ);
            IndexPushBack(indices, nextI * stride + j);
            IndexPushBack(indices, i * stride + nextJ);
        }
    }
}

//--------------------------------------------------------------------------------------
// Cylinder / Cone
//--------------------------------------------------------------------------------------

// Helper computes a point on a unit circle, aligned to the x/z plane and centered on the origin.
inline XMVECTOR GetCircleVector(size_t i, size_t tessellation) noexcept
{
    const float angle = float(i) * XM_2PI / float(tessellation);
    float dx, dz;

    XMScalarSinCos(&dx, &dz, angle);

    const XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
    return v;
}

inline XMVECTOR GetCircleTangent(size_t i, size_t tessellation) noexcept
{
    const float angle = (float(i) * XM_2PI / float(tessellation)) + XM_PIDIV2;
    float dx, dz;

    XMScalarSinCos(&dx, &dz, angle);

    const XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
    return v;
}

// Helper creates a triangle fan to close the end of a cylinder / cone
void CreateCylinderCap(vector<ObjectVertexStruct>& vertices, vector<UINT>& indices, size_t tessellation, float height, float radius, bool isTop)
{
    // Create cap indices.
    for (size_t i = 0; i < tessellation - 2; i++)
    {
        size_t i1 = (i + 1) % tessellation;
        size_t i2 = (i + 2) % tessellation;

        if (isTop)
        {
            std::swap(i1, i2);
        }

        const size_t vbase = vertices.size();
        IndexPushBack(indices, vbase + i2);
        IndexPushBack(indices, vbase + i1);
        IndexPushBack(indices, vbase);
    }

    // Which end of the cylinder is this?
    XMVECTOR normal = g_XMIdentityR1;

    if (!isTop)
    {
        normal = XMVectorNegate(normal);
    }

    // Create cap vertices.
    for (size_t i = 0; i < tessellation; i++)
    {
        const XMVECTOR circleVector = GetCircleVector(i, tessellation);

        const XMVECTOR position = XMVectorAdd(XMVectorScale(circleVector, radius), XMVectorScale(normal, height));

        ObjectVertexStruct vertex;
        vertex.Position = position;
        vertex.Normal = Vector3(0, 0, 0);
        vertices.push_back(vertex);
    }
}

void ComputeCylinder(vector<ObjectVertexStruct>& vertices, vector<UINT>& indices, float height, float diameter, size_t tessellation)
{
    vertices.clear();
    indices.clear();

    if (tessellation < 3)
        throw std::invalid_argument("tesselation parameter must be at least 3");

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale(g_XMIdentityR1, height);

    const float radius = diameter / 2;
    const size_t stride = tessellation + 1;

    // Create a ring of triangles around the outside of the cylinder.
    for (size_t i = 0; i <= tessellation; i++)
    {
        const XMVECTOR normal = GetCircleVector(i, tessellation);

        const XMVECTOR sideOffset = XMVectorScale(normal, radius);

        const float u = float(i) / float(tessellation);

        ObjectVertexStruct vertex;
        vertex.Position = XMVectorAdd(sideOffset, topOffset);
        vertex.Normal = Vector3(0, 0, 0);
        vertices.push_back(vertex);
        vertex.Position = XMVectorSubtract(sideOffset, topOffset);
        vertices.push_back(vertex);

        IndexPushBack(indices, i * 2 + 1);
        IndexPushBack(indices, (i * 2 + 2) % (stride * 2));
        IndexPushBack(indices, i * 2);

        IndexPushBack(indices, (i * 2 + 3) % (stride * 2));
        IndexPushBack(indices, (i * 2 + 2) % (stride * 2));
        IndexPushBack(indices, i * 2 + 1);
    }

    // Create flat triangle fan caps to seal the top and bottom.
    CreateCylinderCap(vertices, indices, tessellation, height, radius, true);
    CreateCylinderCap(vertices, indices, tessellation, height, radius, false);
}

void ComputeCone(vector<ObjectVertexStruct>& vertices, vector<UINT>& indices, float diameter, float height, size_t tessellation)
{
    vertices.clear();
    indices.clear();

    if (tessellation < 3)
        throw std::invalid_argument("tesselation parameter must be at least 3");

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale(g_XMIdentityR1, height);

    const float radius = diameter / 2;
    const size_t stride = tessellation + 1;

    // Create a ring of triangles around the outside of the cone.
    for (size_t i = 0; i <= tessellation; i++)
    {
        const XMVECTOR circlevec = GetCircleVector(i, tessellation);

        const XMVECTOR sideOffset = XMVectorScale(circlevec, radius);

        const float u = float(i) / float(tessellation);

        const XMVECTOR pt = XMVectorSubtract(sideOffset, topOffset);

        XMVECTOR normal = XMVector3Cross(
            GetCircleTangent(i, tessellation),
            XMVectorSubtract(topOffset, pt));
        normal = XMVector3Normalize(normal);

        // Duplicate the top vertex for distinct normals
        ObjectVertexStruct vertex;
        vertex.Position = topOffset;
        vertex.Normal = Vector3(0, 0, 0);
        vertices.push_back(vertex);
        vertex.Position = pt;
        vertices.push_back(vertex);

        IndexPushBack(indices, (i * 2 + 1) % (stride * 2));
        IndexPushBack(indices, (i * 2 + 3) % (stride * 2));
        IndexPushBack(indices, i * 2);
    }

    // Create flat triangle fan caps to seal the bottom.
    CreateCylinderCap(vertices, indices, tessellation, height, radius, false);
}

void ComputeTeapot(vector<ObjectVertexStruct>& vertices, vector<UINT>& indices, float size)
{
    vertices.clear();
    indices.clear();

    ObjectVertexStruct vertex;
    vertex.Normal = Vector3(0, 0, 0);

    for (int i = 0; i < ARRAYSIZE(teapotVertexFloats); i += 3)
    {
        vertex.Position.x = teapotVertexFloats[i] * size;
        vertex.Position.y = teapotVertexFloats[i + 1] * size;
        vertex.Position.z = teapotVertexFloats[i + 2] * size;
        vertices.push_back(vertex);
    }
    for (int i = 0; i < ARRAYSIZE(teapotIndices); i++)
    {
        indices.push_back(teapotIndices[i]);
    }
}
