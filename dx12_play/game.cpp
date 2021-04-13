#include "game.h"

void Game::Init(std::string fileName) 
{
    {
        std::ifstream is(fileName, std::ios::binary);
        Mesh newMesh(L"");
        try {
            cereal::BinaryInputArchive archive(is);
            archive(newMesh);
        }
        catch (const std::exception& e) {
            OutputDebugStringA(e.what());
            return;
        }
        newMesh.m_instanceCount = 3;
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());

        newMesh.SetPosition(0, XMVectorSet(0, 0, 0, 0));
        newMesh.SetPosition(1, XMVectorSet(1, 0, 0, 0));
        newMesh.SetPosition(2, XMVectorSet(-1, 0, 0, 0));

        m_motor->mesh.push_back(newMesh);
    }
    {
        Mesh newMesh(L"obj/o.obj");
        newMesh.m_instanceCount = 4;
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
        newMesh.SetPosition(0, XMVectorSet(1, -1, 1, 0));
        newMesh.SetPosition(1, XMVectorSet(1, 1, -1, 0));
        newMesh.SetPosition(2, XMVectorSet(-1, -1, -1, 0));
        newMesh.SetPosition(3, XMVectorSet(-1, 1, 1, 0));
        m_motor->mesh.push_back(newMesh);
    }
    //{
    //    std::shared_ptr<Mesh> newMesh = std::shared_ptr<Mesh>(new Mesh(L"obj/o.obj"));
    //    newMesh->m_instanceCount = 2;
    //    newMesh->InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
    //    newMesh->SetPosition(0, XMVectorSet(0, -1, -1, 0));
    //    newMesh->SetPosition(1, XMVectorSet(0, 1, -1, 0));
    //    m_motor->mesh.push_back(newMesh);
    //}
}

void Game::Update()
{

}