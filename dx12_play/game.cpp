#include "game.h"

void Game::Init() 
{
    auto mesh = std::make_unique<WaveFrontReader<uint16_t>>();

    if (FAILED(mesh->Load(L"obj/barrel.obj"))) 
    {
        // Error
        return;
    }

    size_t nFaces = mesh->indices.size() / 3;
    size_t nVerts = mesh->vertices.size();

    std::unique_ptr<XMFLOAT3[]> pos(new XMFLOAT3[nVerts]);
    for (size_t j = 0; j < nVerts; ++j)
        pos[j] = mesh->vertices[j].position;

    std::vector<Meshlet> meshlets;
    std::vector<uint8_t> uniqueVertexIB;
    std::vector<MeshletTriangle> primitiveIndices;
    if (FAILED(ComputeMeshlets(mesh->indices.data(), nFaces,
        pos.get(), nVerts,
        nullptr,
        meshlets, uniqueVertexIB, primitiveIndices, MAX_VERTS, MAX_PRIMS)))
    {
        // Error
    }

    std::vector<Vertex> triangleVertices = {};
    for (size_t j = 0; j < nVerts; ++j)
        triangleVertices.push_back({ mesh->vertices[j].position, mesh->vertices[j].normal });

    m_motor->CreateVertexBuffer(triangleVertices, meshlets, uniqueVertexIB, primitiveIndices);
}

void Game::Update()
{

}