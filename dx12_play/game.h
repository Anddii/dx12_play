#pragma once
#include "d3d12_motor.h"
#include <wrl/client.h>
#include "wavefront_reader.h"

#include "DirectXMesh.h"

using namespace Microsoft::WRL;

class Game
{
private:
	std::shared_ptr<D3D12Motor>m_motor;

public:
	Game(std::shared_ptr<D3D12Motor> motor) { this->m_motor = motor; };
	~Game() {};
	void Init();
	void Update();
};

