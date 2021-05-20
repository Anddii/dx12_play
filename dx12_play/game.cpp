#include "game.h"

void Game::Init(HWND window) 
{
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    {
        Mesh newMesh("./models/cube.me", 1);
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
        m_motor->mesh.push_back(newMesh);
        m_motor->mesh[0].SetPosition(0, XMVectorSet(0, 0, 0, 0));
        m_motor->mesh[0].SetScale(0, XMVectorSet(100, 100, 100, 0));
        m_motor->mesh[0].m_textureIndex = 0;
    }
    {
        Mesh newMesh("./models/plane.me", 1);
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
        m_motor->mesh.push_back(newMesh);
        m_motor->mesh[1].SetPosition(0, XMVectorSet(0, -1, 0, 0));
        m_motor->mesh[1].SetScale(0, XMVectorSet(20, 1, 20, 0));
    }
    {
        Mesh newMesh("./models/horse.me", 3);
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
        m_motor->mesh.push_back(newMesh);
        
        m_motor->mesh[2].m_textureIndex = 2;
        m_motor->mesh[2].SetScale(0, XMVectorSet(0.6f, 0.6f, 0.6f, 0.0f));
        m_motor->mesh[2].SetPosition(0, XMVectorSet(6, 7.7f, 0, 0));

        m_motor->mesh[2].SetRotation(1, XMVectorSet(0.0f, XMConvertToRadians(-90), 0, 0));
        m_motor->mesh[2].SetPosition(1, XMVectorSet(15, 7.7f, 10, 0));

        m_motor->mesh[2].SetRotation(2, XMVectorSet(XMConvertToRadians(90), 0, 0, 0));
        m_motor->mesh[2].SetPosition(2, XMVectorSet(10, 1.5f, -15, 0));
    }
    {
        Mesh newMesh("./models/pillar.me", 10);
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
        m_motor->mesh.push_back(newMesh);

        m_motor->mesh[3].SetPosition(0, XMVectorSet(-5, -1, 0, 0));
        m_motor->mesh[3].SetPosition(1, XMVectorSet(5, -1, 0, 0));

        m_motor->mesh[3].SetPosition(2, XMVectorSet(-5, -1, -5, 0));
        m_motor->mesh[3].SetPosition(3, XMVectorSet(5, -1, -5, 0));

        m_motor->mesh[3].SetPosition(4, XMVectorSet(-5, -1, 5, 0));
        m_motor->mesh[3].SetPosition(5, XMVectorSet(5, -1, 5, 0));

        m_motor->mesh[3].SetPosition(6, XMVectorSet(-5, -1, 10, 0));
        m_motor->mesh[3].SetPosition(7, XMVectorSet(5, -1, 10, 0));

        m_motor->mesh[3].SetPosition(8, XMVectorSet(-5, -1, -10, 0));
        m_motor->mesh[3].SetPosition(9, XMVectorSet(5, -1, -10, 0));
    }
    {
        Mesh newMesh("./models/sphere.me", 10);
        newMesh.InitMesh(m_motor->m_device.Get(), m_motor->m_commandQueue.Get(), m_motor->m_commandAllocators[m_motor->m_frameIndex].Get(), m_motor->m_commandList.Get());
        m_motor->mesh.push_back(newMesh);

        m_motor->mesh[4].SetPosition(0, XMVectorSet(0, 0, 0, 0));
        m_motor->mesh[4].SetPosition(1, XMVectorSet(5, 5, 0, 0));

        m_motor->mesh[4].SetPosition(2, XMVectorSet(-5, 5, -5, 0));
        m_motor->mesh[4].SetPosition(3, XMVectorSet(5, 5, -5, 0));

        m_motor->mesh[4].SetPosition(4, XMVectorSet(-5, 5, 5, 0));
        m_motor->mesh[4].SetPosition(5, XMVectorSet(5, 5, 5, 0));

        m_motor->mesh[4].SetPosition(6, XMVectorSet(-5, 5, 10, 0));
        m_motor->mesh[4].SetPosition(7, XMVectorSet(5, 5, 10, 0));

        m_motor->mesh[4].SetPosition(8, XMVectorSet(-5, 5, -10, 0));
        m_motor->mesh[4].SetPosition(9, XMVectorSet(5, 5, -10, 0));
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
    int cameraSpeed = 14;
    if (kb.D)
    {
        m_motor->m_cameraPosition = XMVectorAdd(
            m_motor->m_cameraPosition,
            XMVector4Normalize(
                XMVector3Cross(m_motor->m_cameraFront, m_motor->m_cameraUp)  
            ) 
            * -cameraSpeed * deltaTime.count());
    }
    if (kb.A)
    {
        m_motor->m_cameraPosition = XMVectorAdd(
            m_motor->m_cameraPosition,
            XMVector4Normalize(
                XMVector3Cross(m_motor->m_cameraFront, m_motor->m_cameraUp)
            )
            * cameraSpeed * deltaTime.count());
    }
    if (kb.E)
    {
        XMVECTOR v2 = m_motor->m_cameraPosition;
        XMFLOAT4 v2F;    //the float where we copy the v2 vector members
        XMStoreFloat4(&v2F, v2);   //the function used to copy

        m_motor->m_cameraPosition = XMVectorSet(v2F.x, v2F.y + cameraSpeed * deltaTime.count(), v2F.z, 0);
    }
    if (kb.Q)
    {
        XMVECTOR v2 = m_motor->m_cameraPosition;
        XMFLOAT4 v2F;    //the float where we copy the v2 vector members
        XMStoreFloat4(&v2F, v2);   //the function used to copy

        m_motor->m_cameraPosition = XMVectorSet(v2F.x, v2F.y + -cameraSpeed * deltaTime.count(), v2F.z, 0);
    }
    if (kb.W)
    {
        m_motor->m_cameraPosition = XMVectorAdd(m_motor->m_cameraPosition, m_motor->m_cameraFront * cameraSpeed * deltaTime.count());
    }
    if (kb.S)
    {
        m_motor->m_cameraPosition = XMVectorAdd(m_motor->m_cameraPosition, m_motor->m_cameraFront * -cameraSpeed * deltaTime.count());
    }

    auto mouse = m_mouse->GetState();
    if (m_first)
    {
        m_mousePos = Pos(mouse.x, mouse.y);
        m_first = false;
    }

    float xOffset = m_mousePos.x- mouse.x;
    float yOffset = m_mousePos.y - mouse.y;
    xOffset *= 0.2f; // SENSSI
    yOffset *= 0.2f; // SENSSI
    m_mousePos = Pos(mouse.x, mouse.y);

    m_motor->m_cameraYaw += xOffset;
    m_motor->m_cameraPitch += yOffset;
    m_motor->m_cameraPitch = std::clamp(m_motor->m_cameraPitch, -89.0f, 89.0f);

    XMFLOAT4 direction = XMFLOAT4(0,0,0,0);
    direction.x = cos(XMConvertToRadians(m_motor->m_cameraYaw)) * cos(XMConvertToRadians(m_motor->m_cameraPitch));
    direction.y = sin(XMConvertToRadians(m_motor->m_cameraPitch));
    direction.z = sin(XMConvertToRadians(m_motor->m_cameraYaw)) * cos(XMConvertToRadians(m_motor->m_cameraPitch));
    m_motor->m_cameraFront = XMVector4Normalize(XMVectorSet(direction.x, direction.y, direction.z, direction.w));

    XMVECTOR v2 = m_motor->m_cameraPosition;
    XMFLOAT4 v2F;    //the float where we copy the v2 vector members
    XMStoreFloat4(&v2F, v2);   //the function used to copy
    m_motor->mesh[0].SetPosition(0, XMVectorSet(v2F.x/100, v2F.y/100, v2F.z/100, 0));

}