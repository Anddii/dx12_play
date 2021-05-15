#pragma once
#include <directxmath.h>

#include <cereal/types/memory.hpp>

using namespace DirectX;
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 uv;

    template <class Archive>
    void serialize(Archive& ar)
    {
        cereal::NameValuePair<float&> pos[3] = { cereal::make_nvp("x", position.x), cereal::make_nvp("y", position.y), cereal::make_nvp("z", position.z) };
        cereal::NameValuePair<float&> norm[3] = { cereal::make_nvp("x", normal.x), cereal::make_nvp("y", normal.y), cereal::make_nvp("z", normal.z) };
        cereal::NameValuePair<float&> uv[2] = { cereal::make_nvp("x", this->uv.x), cereal::make_nvp("y", this->uv.y) };
        ar(CEREAL_NVP(pos), CEREAL_NVP(norm), CEREAL_NVP(uv));
    }
};