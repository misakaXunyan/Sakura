/*****************************************************************************************
							 Copyrights   SaeruHikari
CreateDate:							2019.10.21
Description:	          D3D12 Renderer Graphics Manager version v0.
Details:
*******************************************************************************************/
#pragma once
#include "../SakuraD3D12GraphicsManager.hpp"
#include <memory>
#include "../../../GraphicTypes/D3D12/SD3DCamera.h"
#include "Framework/GraphicTypes/D3D12/SRenderTarget.hpp"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace HikaD3DUtils;

namespace SGraphics
{
	class SsaoPass;
	class SGBufferPass;
	class SkySpherePass;
	class SHDR2CubeMapPass;
	class SCubeMapConvPass;

	class SRenderTarget2D;

	namespace SRenderLayers
	{
		enum ERenderLayer
		{
			E_Opaque = 0,
			E_ScreenQuad = 1,
			E_GBufferDebug = 2,
			E_SKY = 3,
			E_Cube = 4,
			E_Count = 5
		};
	}
	namespace SPasses
	{
		enum EPasses
		{
			E_GBuffer = 0,
			E_SSAO = 1,
			E_Deferred = 2,
			E_GBufferDebug = 3,
			E_Count = 4
		};
	}

	class SDxRendererGM : public SakuraD3D12GraphicsManager
	{
	public:
		SDxRendererGM()
			:SakuraD3D12GraphicsManager() {};
		SDxRendererGM(const SDxRendererGM& rhs) = delete;
		SDxRendererGM& operator=(const SDxRendererGM& rhs) = delete;
		~SDxRendererGM() {
			if (md3dDevice != nullptr)
				FlushCommandQueue();
		}

	public:
		inline auto GetGBufferSrvCPU(int offset)
		{
			auto srvCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mGBufferSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			srvCPU.Offset(offset, mCbvSrvUavDescriptorSize);
			return srvCPU;
		}
		inline auto GetGBufferSrvGPU(int offset)
		{
			auto srvGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mGBufferSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			srvGPU.Offset(offset, mCbvSrvUavDescriptorSize);
			return srvGPU;
		}
		inline auto GetDeferredSrvCPU(int offset)
		{
			auto srvCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDeferredSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			srvCPU.Offset(offset, mCbvSrvUavDescriptorSize);
			return srvCPU;
		}
		inline auto GetDeferredSrvGPU(int offset)
		{
			auto srvGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDeferredSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			srvGPU.Offset(offset, mCbvSrvUavDescriptorSize);
			return srvGPU;
		}
	public:
		virtual bool Initialize() override;
		virtual void Draw() override;
		virtual void Finalize() override;
		virtual void Tick(double deltaTime) override;


		virtual void OnMouseDown(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) override;
		virtual void OnMouseMove(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) override;
		virtual void OnMouseUp(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) override;
		virtual void OnKeyDown(double deltaTime) override;

	protected:
		virtual void OnResize(UINT Width, UINT Height) override;

		void AnimateMaterials();
		void UpdateObjectCBs();
		void UpdateMaterialCBs();
		void UpdateMainPassCB();
		void UpdateSsaoPassCB();

		void LoadTextures();
		void BuildGBufferPassRootSignature();
		void BuildDeferredPassRootSignature();

		// Create Descriptor Heaps for RenderTargetView & Depth/Stencil View
		virtual void CreateRtvAndDsvDescriptorHeaps() override;

		void BuildDescriptorHeaps();
		void BuildGBufferPassDescriptorHeaps();
		void BuildDeferredShadingPassDescriptorHeaps();
		void BindPassResources();

		void BuildShaderAndInputLayout();
		void BuildGBufferPassShaderAndInputLayout();
		void BuildGBufferDebugShaderAndInputLayout();

		void BuildDeferredShadingPassShaderAndInputLayout();
		void BuildGeneratedMeshes();
		void BuildGeometry();

		void BuildPSOs();
		void BuildFrameResources();
		void BuildMaterials();

		void BuildRenderItems();

		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<SRenderItem*>& ritems);


	private:
		std::vector<std::unique_ptr<SFrameResource>> mFrameResources;
		SFrameResource* mCurrFrameResource = nullptr;
		int mCurrFrameResourceIndex = 0;

		UINT mCbvSrvDescriptorSize = 0;

		std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> mRootSignatures;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
		std::unordered_map<std::string, std::unique_ptr<DisneyPBRMaterial>> mMaterials;
		std::unordered_map<std::string, std::unique_ptr<SD3DTexture>> mTextures;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayouts[SPasses::E_Count];

		// List of all the render items
		std::vector<std::unique_ptr<SRenderItem>> mAllRitems;

		// Render items divided by PSO
		std::vector<SRenderItem*> mRenderLayers[SRenderLayers::E_Count];

		// loose, need refactoring
		SPassConstants mMainPassCB;
		SsaoConstants mSsaoCB;

		SD3DCamera mCamera;
		SD3DCamera mCubeMapCamera[6];

		int GBufferResourceSrv = 0;
		int GBufferMaterials = 0;
		inline static const int GBufferRTNum = 4;
		inline static const int LUTNum = 1;
		inline static const int SkyCubeMips = 8;
		inline static const int SkyCubePrefilters = 5;
		inline static const int SkyCubeConvNum = 1;
		inline static const int SkyCubeConvFilterNum = SkyCubePrefilters + SkyCubeConvNum;
		inline static const int GBufferSrvStartAt = SkyCubeMips + SkyCubePrefilters + SkyCubeConvNum + LUTNum;
		std::shared_ptr<SRenderTarget2D>* GBufferRTs;

		POINT mLastMousePos;

	protected:
		void BuildCubeFaceCamera(float x, float y, float z);

	protected:
		std::shared_ptr<SHDR2CubeMapPass> mHDRUnpackPass = nullptr;
		std::shared_ptr<SGBufferPass> mGbufferPass = nullptr;
		std::shared_ptr<SsaoPass> mSsaoPass = nullptr;
		std::shared_ptr<SkySpherePass> mDrawSkyPass = nullptr;
		std::shared_ptr<SCubeMapConvPass> mCubeMapConvPass = nullptr;

		ComPtr<ID3D12DescriptorHeap> mGBufferSrvDescriptorHeap = nullptr;
		ComPtr<ID3D12DescriptorHeap> mDeferredSrvDescriptorHeap = nullptr;


		std::vector<ID3D12Resource*> mGBufferSrvResources;
		// GBuffer N D output
		std::vector<ID3D12Resource*> mSsaoSrvResources;

		// HDRI & IBL
		std::shared_ptr<SD3DTexture> mHDRTexture;
		std::shared_ptr<SD3DTexture> mBRDF_LUT;
		std::vector<ID3D12Resource*> mConvAndPrefilterSkyCubeResource[SkyCubeConvFilterNum];
		std::vector<ID3D12Resource*> mSkyCubeResource;
		// SRenderTargetCubeMultiLevels<5>
		std::shared_ptr<SRenderTargetCube> mConvAndPrefilterCubeRTs[SkyCubeConvFilterNum];
		std::shared_ptr<SRenderTargetCube> mSkyCubeRT[SkyCubeMips];
	};

}
	