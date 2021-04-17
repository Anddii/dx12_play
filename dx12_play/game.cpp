#include "game.h"

void Game::Init(std::string fileName, HWND window) 
{
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    {
        std::ifstream is(fileName, std::ios::binary);

        if (!is) {
            throw std::invalid_argument("received negative value");
            return;
        }

        Mesh newMesh(L"");
        try {
            cereal::BinaryInputArchive archive(is);
            archive(newMesh);
        }
        catch (const std::exception& e) {
            OutputDebugStringA(e.what());
            return;
        }

        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());

        m_motor->mesh.push_back(newMesh);
    }
}

void Game::Update()
{
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
    std::chrono::duration<double> deltaTime = time - m_then;
    m_then = time;

    auto kb = m_keyboard->GetState();
    if (kb.Escape)
    {

    }

    auto mouse = m_mouse->GetState();
    if (mouse.leftButton) {
        XMVECTOR v2 = m_motor->mesh[0].m_rotation;
        XMFLOAT4 v2F;    //the float where we copy the v2 vector members
        XMStoreFloat4(&v2F, v2);   //the function used to copy

        int newPosX = m_mousePos.x - mouse.x;
        int newPosY = m_mousePos.y - mouse.y;
        m_motor->mesh[0].SetRotation(0, XMVectorSet(v2F.x + newPosY * deltaTime.count(), v2F.y + -newPosX * deltaTime.count(), 0, 0));
    }

    if (mouse.rightButton) {
        XMVECTOR v2 = m_motor->mesh[0].m_cameraPosition;
        XMFLOAT4 v2F;    //the float where we copy the v2 vector members
        XMStoreFloat4(&v2F, v2);   //the function used to copy

        int newPosX = m_mousePos.x - mouse.x;
        int newPosY = m_mousePos.y - mouse.y;
        m_motor->mesh[0].SetCameraPosition(0,XMVectorSet(v2F.x+-newPosX * deltaTime.count()*0.5f, v2F.y + -newPosY * deltaTime.count()* 0.5f, v2F.z,0));
    }

    if (m_scrollWheelValue != mouse.scrollWheelValue) {
        XMVECTOR v2 = m_motor->mesh[0].m_cameraPosition;
        XMFLOAT4 v2F;    //the float where we copy the v2 vector members
        XMStoreFloat4(&v2F, v2);   //the function used to copy
        int newPos = m_scrollWheelValue - mouse.scrollWheelValue;
        m_motor->mesh[0].SetCameraPosition(0, XMVectorSet(v2F.x, v2F.y, v2F.z+ newPos * deltaTime.count() * 0.1f, 0));
        m_scrollWheelValue = mouse.scrollWheelValue;
    }

    m_mousePos = Pos(mouse.x, mouse.y);
}