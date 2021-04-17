#pragma once
#include "d3d12_motor.h"
#include <wrl/client.h>

#include "DirectXMesh.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "mesh.h"

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>

using namespace Microsoft::WRL;
using namespace DirectX;

class Game
{
private:

	struct Pos
	{
		int x;
		int y;
		Pos() {};
		Pos(int _x, int _y)
		{
			x = _x;
			y = _y;
		};
	};

	std::shared_ptr<D3D12Motor>m_motor;

	std::chrono::system_clock::time_point m_then;
	Pos m_mousePos;
	int m_scrollWheelValue;

public:
	std::unique_ptr<Keyboard> m_keyboard;
	std::unique_ptr<Mouse> m_mouse;

	Game(std::shared_ptr<D3D12Motor> motor) { this->m_motor = motor; };
	~Game() {};
	void Init(std::string fileName, HWND window);
	void Update();

	std::unique_ptr<Mesh> mesh1;
};

