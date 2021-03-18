#include "game.h"

void Game::Init() 
{
    auto mesh = std::make_unique<WaveFrontReader<uint16_t>>();
    
    if (FAILED(mesh->Load(L"obj/barrel.obj")))
    {
        // Error
    }

    size_t nVerts = mesh->vertices.size();

    std::vector<Vertex> triangleVertices = {};
    for (size_t j = 0; j < nVerts; ++j)
        triangleVertices.push_back({ mesh->vertices[j].position, mesh->vertices[j].textureCoordinate, mesh->vertices[j].normal });


    m_motor->CreateVertexBuffer(triangleVertices);
    m_motor->CreateIndexBuffer(mesh->indices);
}

void Game::Update()
{

}