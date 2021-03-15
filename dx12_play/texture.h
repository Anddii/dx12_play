#pragma once
#include <string>
#include <wrl.h>
#include <d3d12.h>

#include <DDSTextureLoader.h>
#include "ResourceUploadBatch.h"

using namespace Microsoft::WRL;

class Texture
{
public:
    std::string m_name;
    std::wstring m_filename;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource = nullptr;

	Texture(std::string filename, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue) {
		this->m_name = filename;
		this->m_filename = std::wstring(filename.begin(), filename.end());
		DirectX::ResourceUploadBatch resourceUpload(device.Get());

		resourceUpload.Begin();

		CreateDDSTextureFromFile(device.Get(), resourceUpload, this->m_filename.c_str(),
			this->m_resource.ReleaseAndGetAddressOf());

		// Upload the resources to the GPU.
		auto uploadResourcesFinished = resourceUpload.End(commandQueue.Get());

		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();
	}
private:

};
