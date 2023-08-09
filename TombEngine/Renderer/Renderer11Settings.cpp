#include "framework.h"
#include <filesystem>
#include <codecvt>

#include "Renderer/Renderer11.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

namespace TEN::Renderer 
{

	Vector2i ClipResolutionToReferenceResolutionAspectRatio(Vector2i Resolution, Vector2i ReferenceResolution) {
		float referenceAspectRatio = static_cast<float>(ReferenceResolution.x) / ReferenceResolution.y;

		if (static_cast<float>(Resolution.x) / Resolution.y > referenceAspectRatio) {
			Resolution.x = static_cast<int>(Resolution.y * referenceAspectRatio);
		}
		else {
			Resolution.y = static_cast<int>(Resolution.x / referenceAspectRatio);
		}

		return Resolution;
	}

	void Renderer11::UpdateScreenResolution() 
	{
		if (m_windowHeight <= 0 || m_windowWidth <= 0) {
			return;
		}

		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_context->OMSetRenderTargets(0, nullViews, NULL);

		m_backBufferTexture->Release();
		m_backBufferRTV->Release();
		m_depthStencilView->Release();
		m_depthStencilTexture->Release();
		m_context->Flush();
		m_context->ClearState();

		IDXGIOutput* output;
		Utils::throwIfFailed(m_swapChain->GetContainingOutput(&output));

		DXGI_SWAP_CHAIN_DESC scd;
		Utils::throwIfFailed(m_swapChain->GetDesc(&scd));

		if (!g_Configuration.ScaleFramebuffer) {
			Vector2i clipped_resolution = ClipResolutionToReferenceResolutionAspectRatio(Vector2i(m_windowWidth, m_windowHeight), Vector2i(g_Configuration.ScreenWidth, g_Configuration.ScreenHeight));

			m_screenWidth = clipped_resolution.x;
			m_screenHeight = clipped_resolution.y;
		} else {
			m_screenWidth = g_Configuration.ScreenWidth;
			m_screenHeight = g_Configuration.ScreenHeight;
		}

		m_windowed = g_Configuration.EnableWindowedMode;

		InitializeScreen(m_screenWidth, m_screenHeight, WindowsHandle, true);
		SetFullScreen();
	}

	std::string Renderer11::GetDefaultAdapterName()
	{
		IDXGIFactory* dxgiFactory = NULL;
		Utils::throwIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

		IDXGIAdapter* dxgiAdapter = NULL;

		dxgiFactory->EnumAdapters(0, &dxgiAdapter);

		DXGI_ADAPTER_DESC adapterDesc = {};

		dxgiAdapter->GetDesc(&adapterDesc);
		dxgiFactory->Release();
		
		return TEN::Utils::ToString(adapterDesc.Description);
	}

	void Renderer11::SetTextureOrDefault(Texture2D& texture, std::wstring path)
	{
		texture = Texture2D();

		if (std::filesystem::is_regular_file(path))
			texture = Texture2D(m_device.Get(), path);
		else
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			TENLog("Texture file not found: " + converter.to_bytes(path), LogLevel::Warning);
		}
	}
}
