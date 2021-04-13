#pragma once
#include "d3d12_motor.h"
#include <wrl/client.h>

#include "DirectXMesh.h"
#include "mesh.h"

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>

using namespace Microsoft::WRL;

class Game
{
private:
	std::shared_ptr<D3D12Motor>m_motor;

public:
	Game(std::shared_ptr<D3D12Motor> motor) { this->m_motor = motor; };
	~Game() {};
	void Init(std::string fileName);
	void Update();
};

