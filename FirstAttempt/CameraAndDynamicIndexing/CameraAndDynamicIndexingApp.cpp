//***************************************************************************************
// CameraAndDynamicIndexingApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "FrameResource.h"
#include "Waves.h"
#include "../../Common/Camera.h"
#include <random>
// Iostream - STD I/O Library
#include <iostream>
#include <fstream>

// All physics calculations taking place here
#include "PhysicsWorld.h"

// All physics calculations taking place here
#include "PhysicsWorld.h"
#include "GameObject.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	PhysicsObject* physics = nullptr;
};

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};

class CameraAndDynamicIndexingApp : public D3DApp
{
public:
	CameraAndDynamicIndexingApp(HINSTANCE hInstance);
	CameraAndDynamicIndexingApp(const CameraAndDynamicIndexingApp& rhs) = delete;
	CameraAndDynamicIndexingApp& operator=(const CameraAndDynamicIndexingApp& rhs) = delete;
	~CameraAndDynamicIndexingApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	virtual void OnMouseScroll(short zDelta)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void PhysicsUpdate(const GameTimer& gt);
	void UpdateGameLoop();
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt);

	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildObjGeometry(char* filePath, std::string geoName, std::string drawArgName);
	void BuildWavesGeometry();
	void BuildBoxGeometry();
	void BuildTreeSpritesGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();

	void BuildRenderItems();
	// Build object+physics
	void BuildTank(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex);
	void generateBullet();
	void BuildHouse(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex);
	void BuildBullet(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex);
	void BuildTree(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex);

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	float GetHillsHeight(float x, float z)const;
	XMFLOAT3 GetHillsNormal(float x, float z)const;

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;

	RenderItem* mWavesRitem = nullptr;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	std::unique_ptr<Waves> mWaves;



	PassConstants mMainPassCB;

	/*XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;*/

	Camera mCamera;
	float pi = 3.14;

	POINT mLastMousePos;

	bool mIsWireframe = false;

	std::unique_ptr<std::mt19937> m_random;
	float explodeDelay;

	// Setup Physics
	PhysicsWorld physicsWorld;
	std::vector<std::unique_ptr<PhysicsObject>> allPhysicsObjects;
	std::vector<std::unique_ptr<GameObject>> allGameObjects;
	std::vector<GameObject*> gameObjects;
	GameObject* playerGameObject = nullptr;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		CameraAndDynamicIndexingApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

CameraAndDynamicIndexingApp::CameraAndDynamicIndexingApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

CameraAndDynamicIndexingApp::~CameraAndDynamicIndexingApp()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool CameraAndDynamicIndexingApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mWaves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	/*m_explode = std::make_unique<SoundEffect>(m_audEngine.get(),
		L"explo1.wav");
	m_ambient = std::make_unique<SoundEffect>(m_audEngine.get(),
		L"NightAmbienceSimple_02.wav");*/

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildObjGeometry("Models/tank2.obj", "objGeoTank", "objTank"); // load OBJ test
	BuildObjGeometry("Models/house2.obj", "objGeoHouse", "objHouse"); // load OBJ test
	BuildObjGeometry("Models/bullet2.obj", "objGeoBullet", "objBullet"); // load OBJ test
	BuildLandGeometry();
	BuildWavesGeometry();
	BuildBoxGeometry();
	BuildTreeSpritesGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();


	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	mCamera.SetPosition(0.0f, 10.0f, -80.0f);

	std::random_device rd;
	m_random = std::make_unique<std::mt19937>(rd());

	explodeDelay = 2.f;
	return true;
}

void CameraAndDynamicIndexingApp::OnResize()
{
	D3DApp::OnResize();
	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	/*XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);*/
}

void CameraAndDynamicIndexingApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	PhysicsUpdate(gt);
	UpdateCamera(gt);
	UpdateGameLoop();

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}


	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
	UpdateWaves(gt);

	/*explodeDelay -= elapsedTime;
	if (explodeDelay < 0.f)
	{
		m_explode->Play();

		std::uniform_real_distribution<float> dist(1.f, 10.f);
		explodeDelay = dist(*m_random);
	}*/
}

void CameraAndDynamicIndexingApp::Draw(const GameTimer& gt)
{
	//explodeDelay = 2.f;

	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	if (mIsWireframe)
	{
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque_wireframe"].Get()));
	}
	else
	{
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
	}

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&mMainPassCB.FogColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTested]);

	mCommandList->SetPipelineState(mPSOs["treeSprites"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;


	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void CameraAndDynamicIndexingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CameraAndDynamicIndexingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void CameraAndDynamicIndexingApp::OnMouseScroll(short zDelta)
{
	if (zDelta < 0) {
		//mCamera.Walk(-(0.1f * (zDelta / 120.0f) * (zDelta / 120.0f)));
		mCamera.Walk(-5.0f);
	}
	else {
		if (zDelta - 0.1f * (zDelta / 120.0f) * (zDelta / 120.0f) >= 0.0f) {
			//mCamera.Walk(0.1f * (zDelta / 120.0f) * (zDelta / 120.0f));
			mCamera.Walk(5.0f);
		}
	}
}
void CameraAndDynamicIndexingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	//if ((btnState & MK_LBUTTON) != 0)
	//{
	//	// Make each pixel correspond to a quarter of a degree.
	//	float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
	//	float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

	//	// Update angles based on input to orbit camera around box.
	//	mTheta += dx;
	//	mPhi += dy;

	//	// Restrict the angle mPhi.
	//	mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	//}
	//else if ((btnState & MK_RBUTTON) != 0)
	//{
	//	// Make each pixel correspond to 0.2 unit in the scene.
	//	float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
	//	float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

	//	// Update the camera radius based on input.
	//	mRadius += dx - dy;

	//	// Restrict the radius.
	//	mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
	//}

	//mLastMousePos.x = x;
	//mLastMousePos.y = y;

	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

		mCamera.Walk(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CameraAndDynamicIndexingApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000) {
		if (playerGameObject != nullptr) {
			XMVECTOR originalForwardVelocityVector = XMVectorSet(0.0f, 0.0f, 30.0f, 1.0f);
			XMVECTOR actualForwardVelocityVector = XMVector3Rotate(originalForwardVelocityVector, XMLoadFloat4(&playerGameObject->ObjectPhysicsData()->RotationQuaternion()));
			XMFLOAT3 forwardVelocity;
			XMStoreFloat3(&forwardVelocity, actualForwardVelocityVector);
			playerGameObject->ObjectPhysicsData()->setVelocity(forwardVelocity.x, forwardVelocity.y, forwardVelocity.z);
			/*
			XMVECTOR originalForwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
			XMVECTOR actualForwardVector = XMVector3Rotate(originalForwardVector, XMLoadFloat4(&playerGameObject->ObjectPhysicsData()->RotationQuaternion()));
			XMFLOAT3 forwardForce;
			XMStoreFloat3(&forwardForce, actualForwardVelocityVector);
			playerGameObject->ObjectPhysicsData()->setVelocity(forwardVelocity.x, forwardVelocity.y, forwardVelocity.z);

			*/
		}

		mCamera.Walk(100.0f * dt);
	}

	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		mCamera.Pitch(-1.0f * dt);
	}

	if (GetAsyncKeyState('S') & 0x8000) {
		if (playerGameObject != nullptr) {
			XMVECTOR originalForwardVelocityVector = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
			XMVECTOR actualForwardVelocityVector = XMVector3Rotate(originalForwardVelocityVector, XMLoadFloat4(&playerGameObject->ObjectPhysicsData()->RotationQuaternion()));
			XMFLOAT3 forwardVelocity;
			XMStoreFloat3(&forwardVelocity, actualForwardVelocityVector);
			playerGameObject->ObjectPhysicsData()->setVelocity(forwardVelocity.x, forwardVelocity.y, forwardVelocity.z);
		}

		mCamera.Walk(-100.0f * dt);
	}

	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		mCamera.Pitch(1.0f * dt);
	}

	if (GetAsyncKeyState('A') & 0x8000) {
		if (playerGameObject != nullptr) {
			playerGameObject->ChangeOrientationRadians(-0.0001 * 180 / pi);
		}

		mCamera.Strafe(-100.0f * dt);
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		mCamera.RotateY(-1.0f * dt);
	}

	if (GetAsyncKeyState('D') & 0x8000) {
		if (playerGameObject != nullptr) {
			playerGameObject->ChangeOrientationRadians(0.0001 * 180 / pi);
		}

		mCamera.Strafe(100.0f * dt);
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		mCamera.RotateY(1.0f * dt);
	}

	if (GetAsyncKeyState('L') & 0x8000) {
		mCamera.Roll(-1.0f * dt);
	}
	if (GetAsyncKeyState('K') & 0x8000) {
		mCamera.Roll(1.0f * dt);
	}


	if (GetAsyncKeyState('8') & 0x8000) {
		playerGameObject = allGameObjects[0].get();
	}
	if (GetAsyncKeyState('9') & 0x8000) {
		playerGameObject = nullptr;
	}

	/// <summary>
	/// On space button press, generate bullet
	/// </summary>
	/// <param name="gt"></param>
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		//generateBullet(UINT& objCBIndex);
		//generateBullet();

	}

	if (GetAsyncKeyState('2') & 0x8000) {
		mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	}
	if (GetAsyncKeyState('1') & 0x8000) {
		//const XMFLOAT3 pos = mCamera.GetPosition3f();

		//mCamera.SetPosition(pos.x, pos.y + 5, pos.z - 1);

		mCamera.SetLens(0.35f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

		//set character in front of the camera 
	}

	/*if (GetAsyncKeyState('1') & 0x8000)
		mMainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.8f };
		mMainPassCB.Lights[1].Strength = { 0.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[2].Strength = { 0.0f, 0.0f, 0.0f };
	if (GetAsyncKeyState('2') & 0x8000) {
		mMainPassCB.Lights[0].Strength = { 0.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		mMainPassCB.Lights[2].Strength = { 0.0f, 0.0f, 0.0f };
	}
	if (GetAsyncKeyState('3') & 0x8000) {
		mMainPassCB.Lights[0].Strength = { 0.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[1].Strength = { 0.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };
	}*/

	if (GetAsyncKeyState('3') & 0x8000)
		mIsWireframe = true;
	else
		mIsWireframe = false;

	if (playerGameObject == nullptr)
		mCamera.UpdateViewMatrix();
}

void CameraAndDynamicIndexingApp::UpdateCamera(const GameTimer& gt)
{
	if (playerGameObject != nullptr) {
		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(playerGameObject->ObjectPhysicsData()->Position().x, playerGameObject->ObjectPhysicsData()->Position().y, playerGameObject->ObjectPhysicsData()->Position().z, 1.0f);

		XMVECTOR originalForwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
		XMVECTOR actualForwardVector = XMVector3Normalize(XMVector3Rotate(originalForwardVector, XMLoadFloat4(&playerGameObject->ObjectPhysicsData()->RotationQuaternion())));
		XMVECTOR target = actualForwardVector * 1000;

		pos = pos - actualForwardVector * 50;
		pos = XMVectorSetY(pos, 20.0f);

		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		mCamera.LookAt(pos, target, up);
		mCamera.UpdateViewMatrix();
	}
}

void CameraAndDynamicIndexingApp::PhysicsUpdate(const GameTimer& gt)
{
	float dt = gt.DeltaTime();
	physicsWorld.Update(dt);

	for (auto& e : mAllRitems)
	{
		if (e->physics != nullptr) {
			auto& currentPhysicsObject = e->physics;
		//if (e->ObjCBIndex < allPhysicsObjects.size()) {
		//	auto& currentPhysicsObject = allPhysicsObjects[e->ObjCBIndex];

			XMMATRIX worldMatrix = XMLoadFloat4x4(&e->World);
			XMVECTOR scaleVector = XMVectorZero();
			XMVECTOR rotationOriginPoint = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
			XMVECTOR rotationVector = XMVectorZero();
			XMVECTOR translationVector = XMVectorZero();
			XMMatrixDecompose(&scaleVector, &rotationVector, &translationVector, worldMatrix);

			// Set properties based on physics objects
			translationVector = XMVectorSet(currentPhysicsObject->Position().x, currentPhysicsObject->Position().y, currentPhysicsObject->Position().z, 1.0f);
			rotationVector = XMVectorSet(currentPhysicsObject->RotationQuaternion().x, currentPhysicsObject->RotationQuaternion().y, currentPhysicsObject->RotationQuaternion().z, currentPhysicsObject->RotationQuaternion().w);

			// Don't let object move below ground plane.
			float adjustedY = MathHelper::Max(XMVectorGetY(translationVector), 0.0f);
			translationVector = XMVectorSetY(translationVector, adjustedY);

			// Create new world matrix for moving object
			XMMATRIX newWorldMatrix = XMMatrixAffineTransformation(scaleVector, rotationOriginPoint, rotationVector, translationVector);
			XMStoreFloat4x4(&e->World, newWorldMatrix);

			e->NumFramesDirty = gNumFrameResources;
		}
	}
}

void CameraAndDynamicIndexingApp::UpdateGameLoop()
{
	
	//playerGameObject = nullptr;
}

void CameraAndDynamicIndexingApp::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat = mMaterials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * gt.DeltaTime();
	tv += 0.02f * gt.DeltaTime();

	if (tu >= 1.0f)
		tu -= 1.0f;

	if (tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = gNumFrameResources;
}

void CameraAndDynamicIndexingApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			//objConstants.MaterialIndex = e->Mat->MatCBIndex;

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void CameraAndDynamicIndexingApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void CameraAndDynamicIndexingApp::UpdateMainPassCB(const GameTimer& gt)
{
	/*XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;*/
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mCamera.GetPosition3f();

	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.8f };

	//mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Direction = { 0.57735f, 0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };

	//mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Direction = { 0.0f, 0.707f, 0.707f };
	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void CameraAndDynamicIndexingApp::UpdateWaves(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.5f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		mWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	mWaves->Update(gt.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = mCurrFrameResource->WavesVB.get();
	for (int i = 0; i < mWaves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = mWaves->Position(i);
		v.Normal = mWaves->Normal(i);

		// Derive tex-coords from position by 
		// mapping [-w/2,w/2] --> [0,1]
		v.TexC.x = 0.5f + v.Pos.x / mWaves->Width();
		v.TexC.y = 0.5f - v.Pos.z / mWaves->Depth();

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	mWavesRitem->Geo->VertexBufferGPU = currWavesVB->Resource();
}
void CameraAndDynamicIndexingApp::LoadTextures()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"../../Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->Name = "waterTex";
	waterTex->Filename = L"../../Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), waterTex->Filename.c_str(),
		waterTex->Resource, waterTex->UploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->Name = "fenceTex";
	fenceTex->Filename = L"../../Textures/WoodCrate02.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), fenceTex->Filename.c_str(),
		fenceTex->Resource, fenceTex->UploadHeap));

	auto treeArrayTex = std::make_unique<Texture>();
	treeArrayTex->Name = "treeArrayTex";
	treeArrayTex->Filename = L"../../Textures/treeArray2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), treeArrayTex->Filename.c_str(),
		treeArrayTex->Resource, treeArrayTex->UploadHeap));

	mTextures[grassTex->Name] = std::move(grassTex);
	mTextures[waterTex->Name] = std::move(waterTex);
	mTextures[fenceTex->Name] = std::move(fenceTex);
	mTextures[treeArrayTex->Name] = std::move(treeArrayTex);
}

void CameraAndDynamicIndexingApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void CameraAndDynamicIndexingApp::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = mTextures["grassTex"]->Resource;
	auto waterTex = mTextures["waterTex"]->Resource;
	auto fenceTex = mTextures["fenceTex"]->Resource;
	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);
}

void CameraAndDynamicIndexingApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_0");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_0");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

/**
* Creates geometry for the passed in .obj model, using the passed in geoName and drawArgName.
*
* NOTE: .obj file must be triangular (i.e. NO QUADS!)
*/
void CameraAndDynamicIndexingApp::BuildObjGeometry(char* filePath, std::string geoName, std::string drawArgName)
{
	UINT numberOfFaces = 0;
	std::vector<UINT> vertexIndices, uvIndices, normalIndices;
	std::vector<XMFLOAT3> temp_vertices;
	std::vector<XMFLOAT3> temp_normals;
	std::vector< XMFLOAT2 > temp_uvs;
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	// Open file
	FILE* file = fopen(filePath, "r");
	if (file == NULL) {
		MessageBox(0, L"NULL FILE", 0, 0);
	}

	// Parse file
	char lineHeader[128];
	while (fscanf(file, "%s \n", lineHeader) != EOF) {
		// Vertex positions
		if (strcmp(lineHeader, "v") == 0) {
			XMFLOAT3 vertex;
			int a = fscanf(file, "%f %f %f \n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		// Texture coordinates
		else if (strcmp(lineHeader, "vt") == 0) {
			XMFLOAT2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		// Vertex normals
		else if (strcmp(lineHeader, "vn") == 0) {
			XMFLOAT3 normals;
			fscanf(file, "%f %f %f \n", &normals.x, &normals.y, &normals.z);
			temp_normals.push_back(normals);
		}
		// Faces (file must use triangular faces)
		else if (strcmp(lineHeader, "f") == 0) {
			UINT vertexIndex[3], uvIndex[3], normalIndex[3];

			// Note that this assumes the faces contain all components: position, texture coordinate and normal
			// texture coordinate and normal are optional in .obj but required here to be read

			// This also requires the .obj to use triangular faces i.e.
			// f 1/2/3 1/2/3 1/2/3
			int matches = fscanf(file, "%i/%i/%i %i/%i/%i %i/%i/%i \n",
				&vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			if (matches != 9) {
				MessageBox(0, L"Error reading file", 0, 0);
				break;
			}

			// Load face data
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
			numberOfFaces++;
		}
	}

	// Create vertex and index list
	numberOfFaces *= 3;
	int cont = 0;
	int index = -1;
	for (int i = 0; i < numberOfFaces; i++) {

		UINT vertexIndex = vertexIndices[i];
		UINT normalIndex = normalIndices[i];
		UINT uvIndex = uvIndices[i];
		Vertex temp;
		temp.Pos = temp_vertices[vertexIndex - 1];
		temp.Normal = temp_normals[normalIndex - 1];
		//temp.Tex = temp_uvs[uvIndex - 1]; // For texture; not currently implemented

		indices.push_back(++index);
		vertices.push_back(temp);

		cont++;
	}

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)vertexIndices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs[drawArgName] = submesh;

	mGeometries[geo->Name] = std::move(geo);
}

void CameraAndDynamicIndexingApp::BuildLandGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(460.0f, 460.0f, 30, 20);

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;

	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);
}

void CameraAndDynamicIndexingApp::BuildWavesGeometry()
{
	std::vector<std::uint16_t> indices(3 * mWaves->TriangleCount()); // 3 indices per face
	assert(mWaves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = mWaves->RowCount();
	int n = mWaves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = mWaves->VertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["waterGeo"] = std::move(geo);
}

void CameraAndDynamicIndexingApp::BuildBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		/*float x = MathHelper::RandF(-5.0f, 5.0f);
		float z = MathHelper::RandF(-5.0f, 5.0f);
		float y = GetHillsHeight(x, z);*/

		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;

		//vertices[i].Pos = XMFLOAT3(x, y, z);

		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	//const UINT vbByteSize = (UINT)vertices.size() * sizeof(BoxSpriteVertex);
	//const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["box"] = submesh;

	mGeometries["boxGeo"] = std::move(geo);
}

void CameraAndDynamicIndexingApp::BuildTreeSpritesGeometry()
{
	struct TreeSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int treeCount = 20;
	std::array<TreeSpriteVertex, treeCount> vertices;
	for (UINT i = 0; i < treeCount; ++i)
	{
		float x = MathHelper::RandF(-230.0f, 230.0f);
		float z = MathHelper::RandF(-230.0f, 230.0f);
		float y = GetHillsHeight(x, z);

		// Move tree slightly above land height.
		y += 19.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(40.0f, 40.0f);
	}

	std::array<std::uint16_t, treeCount> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(TreeSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	mGeometries["treeSpritesGeo"] = std::move(geo);
}

void CameraAndDynamicIndexingApp::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));


	//
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));
}

void CameraAndDynamicIndexingApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size(), mWaves->VertexCount()));
	}
}

void CameraAndDynamicIndexingApp::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseSrvHeapIndex = 0;
	grass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.225f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseSrvHeapIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->MatCBIndex = 2;
	wirefence->DiffuseSrvHeapIndex = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wirefence->Roughness = 0.95f;

	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 3;
	treeSprites->DiffuseSrvHeapIndex = 3;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 4;
	bricks0->DiffuseSrvHeapIndex = 3;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;

	auto tank = std::make_unique<Material>();
	tank->Name = "tank";
	tank->MatCBIndex = 5;
	tank->DiffuseSrvHeapIndex = 2;
	tank->DiffuseAlbedo = XMFLOAT4(Colors::DarkGreen);
	tank->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tank->Roughness = 0.1f;

	auto wood = std::make_unique<Material>();
	wood->Name = "wood";
	wood->MatCBIndex = 6;
	wood->DiffuseSrvHeapIndex = 2;
	wood->DiffuseAlbedo = XMFLOAT4(Colors::SaddleBrown);
	wood->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wood->Roughness = 0.1f;

	auto bullet = std::make_unique<Material>();
	bullet->Name = "bullet";
	bullet->MatCBIndex = 7;
	bullet->DiffuseSrvHeapIndex = 2;
	bullet->DiffuseAlbedo = XMFLOAT4(Colors::DarkGoldenrod);
	bullet->FresnelR0 = XMFLOAT3(1.0f, 1.0f, 1.0f);
	bullet->Roughness = 0.1f;

	mMaterials["grass"] = std::move(grass);
	mMaterials["water"] = std::move(water);
	mMaterials["wirefence"] = std::move(wirefence);
	mMaterials["treeSprites"] = std::move(treeSprites);
	mMaterials["bricks0"] = std::move(bricks0);
	mMaterials["tank"] = std::move(tank);
	mMaterials["wood"] = std::move(wood);
	mMaterials["bullet"] = std::move(bullet);
}

XMFLOAT4 getRotateObjectQuaternionAroundY(float angleRadians) {
	XMVECTOR rotationQuaternionVector = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), angleRadians);
	XMFLOAT4 rotationQuaternion;
	XMStoreFloat4(&rotationQuaternion, rotationQuaternionVector);
	return rotationQuaternion;
}


void CameraAndDynamicIndexingApp::BuildTank(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex) {
	auto objTankitem = std::make_unique<RenderItem>();
	XMFLOAT4 rotationQuaternion = getRotateObjectQuaternionAroundY(orientationRadians);
	XMVECTOR tankRotationQuaternion = XMLoadFloat4(&rotationQuaternion);
	XMStoreFloat4x4(&objTankitem->World, XMMatrixRotationQuaternion(tankRotationQuaternion) * XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixTranslation(translation.x, translation.y, translation.z));
	objTankitem->TexTransform = MathHelper::Identity4x4();
	objTankitem->ObjCBIndex = objCBIndex++;
	objTankitem->Mat = mMaterials["tank"].get();
	objTankitem->Geo = mGeometries["objGeoTank"].get();
	objTankitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	objTankitem->IndexCount = objTankitem->Geo->DrawArgs["objTank"].IndexCount;
	objTankitem->StartIndexLocation = objTankitem->Geo->DrawArgs["objTank"].StartIndexLocation;
	objTankitem->BaseVertexLocation = objTankitem->Geo->DrawArgs["objTank"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(objTankitem.get());

	// Create Physics Objects
	float mass = 15.0f;
	float coefficientFriction = 1.0;
	float stepTime = 0.0f;
	XMFLOAT3 position = XMFLOAT3(translation.x, translation.y, translation.z);

	XMFLOAT3 objectDimensions = XMFLOAT3(4.0f * scaling.x, 2.5f * scaling.y, 7.5f * scaling.z);
	XMFLOAT3 center = XMFLOAT3((objectDimensions.x / 2), -1.0f + (objectDimensions.y / 2), 0); // Assuming bottom corner fo object
	XMFLOAT3 extents = XMFLOAT3(objectDimensions.x / 2, objectDimensions.y / 2, objectDimensions.z / 2);
	BoundingBox boundingBox = BoundingBox(center, extents);

	float boundingBoxScale = 1.0f;
	XMVECTOR positionVector = XMLoadFloat3(&position);
	boundingBox.Transform(boundingBox, boundingBoxScale, tankRotationQuaternion, positionVector);

	auto physicsObject = std::make_unique<PhysicsObject>(position, center, rotationQuaternion, boundingBox, mass, coefficientFriction, stepTime);
	objTankitem->physics = physicsObject.get();

	// Define Game Objects
	auto gameObject = std::make_unique<GameObject>(physicsObject.get(), orientationRadians, XMFLOAT3(10.0f, 10.0f, 10.0f));

	mAllRitems.push_back(std::move(objTankitem));
	allPhysicsObjects.push_back(std::move(physicsObject));
	allGameObjects.push_back(std::move(gameObject));
}


void CameraAndDynamicIndexingApp::BuildHouse(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex) {
	auto objHouseItem = std::make_unique<RenderItem>();
	XMFLOAT4 rotationQuaternion = getRotateObjectQuaternionAroundY(orientationRadians);
	XMVECTOR houseRotationQuaternion = XMLoadFloat4(&rotationQuaternion);
	XMStoreFloat4x4(&objHouseItem->World, XMMatrixRotationQuaternion(houseRotationQuaternion) * XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixTranslation(translation.x, translation.y, translation.z));
	objHouseItem->TexTransform = MathHelper::Identity4x4();
	objHouseItem->ObjCBIndex = objCBIndex++;
	objHouseItem->Mat = mMaterials["wood"].get();
	objHouseItem->Geo = mGeometries["objGeoHouse"].get();
	objHouseItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	objHouseItem->IndexCount = objHouseItem->Geo->DrawArgs["objHouse"].IndexCount;
	objHouseItem->StartIndexLocation = objHouseItem->Geo->DrawArgs["objHouse"].StartIndexLocation;
	objHouseItem->BaseVertexLocation = objHouseItem->Geo->DrawArgs["objHouse"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(objHouseItem.get());

	// Create Physics Objects
	float mass = 15.0f;
	float coefficientFriction = 0;
	float stepTime = 0.0f;
	XMFLOAT3 position = XMFLOAT3(translation.x, translation.y, translation.z);

	XMFLOAT3 objectDimensions = XMFLOAT3(4.0f * scaling.x, 3.0f * scaling.y, 5.25f * scaling.z);
	XMFLOAT3 center = XMFLOAT3(0, (objectDimensions.y / 2), -3.5f); // hosue built around center point
	XMFLOAT3 extents = XMFLOAT3(objectDimensions.x / 2, objectDimensions.y / 2, objectDimensions.z / 2);
	BoundingBox boundingBox = BoundingBox(center, extents);

	float boundingBoxScale = 1.0f;
	XMVECTOR positionVector = XMLoadFloat3(&position);
	boundingBox.Transform(boundingBox, boundingBoxScale, houseRotationQuaternion, positionVector);

	auto physicsObject = std::make_unique<PhysicsObject>(position, center, rotationQuaternion, boundingBox, mass, coefficientFriction, stepTime);
	objHouseItem->physics = physicsObject.get();

	mAllRitems.push_back(std::move(objHouseItem));
	allPhysicsObjects.push_back(std::move(physicsObject));
}

void CameraAndDynamicIndexingApp::BuildTree(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex) {
	auto treeSpritesRitem = std::make_unique<RenderItem>();
	XMFLOAT4 rotationQuaternion = getRotateObjectQuaternionAroundY(orientationRadians);
	XMVECTOR treeRotationQuaternion = XMLoadFloat4(&rotationQuaternion);
	XMStoreFloat4x4(&treeSpritesRitem->World, XMMatrixRotationQuaternion(treeRotationQuaternion) * XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixTranslation(translation.x, translation.y, translation.z));
	treeSpritesRitem->World = MathHelper::Identity4x4();
	treeSpritesRitem->ObjCBIndex = objCBIndex++;
	treeSpritesRitem->Mat = mMaterials["treeSprites"].get();
	treeSpritesRitem->Geo = mGeometries["treeSpritesGeo"].get();
	treeSpritesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	treeSpritesRitem->IndexCount = treeSpritesRitem->Geo->DrawArgs["points"].IndexCount;
	treeSpritesRitem->StartIndexLocation = treeSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
	treeSpritesRitem->BaseVertexLocation = treeSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());

	mAllRitems.push_back(std::move(treeSpritesRitem));
}

void CameraAndDynamicIndexingApp::BuildBullet(XMFLOAT3 scaling, XMFLOAT3 translation, float orientationRadians, UINT& objCBIndex) {
	auto objBulletItem = std::make_unique<RenderItem>();
	XMFLOAT4 rotationQuaternion = getRotateObjectQuaternionAroundY(orientationRadians);
	XMVECTOR bulletRotationQuaternion = XMLoadFloat4(&rotationQuaternion);
	XMStoreFloat4x4(&objBulletItem->World, XMMatrixRotationQuaternion(bulletRotationQuaternion) * XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixTranslation(translation.x, translation.y, translation.z));
	objBulletItem->TexTransform = MathHelper::Identity4x4();
	objBulletItem->ObjCBIndex = objCBIndex++;
	objBulletItem->Mat = mMaterials["bullet"].get();
	objBulletItem->Geo = mGeometries["objGeoBullet"].get();
	objBulletItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	objBulletItem->IndexCount = objBulletItem->Geo->DrawArgs["objBullet"].IndexCount;
	objBulletItem->StartIndexLocation = objBulletItem->Geo->DrawArgs["objBullet"].StartIndexLocation;
	objBulletItem->BaseVertexLocation = objBulletItem->Geo->DrawArgs["objBullet"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(objBulletItem.get());

	// Create Physics Objects
	float mass = 1.0f;
	float coefficientFriction = 0;
	float stepTime = 0.0f;
	XMFLOAT3 position = XMFLOAT3(translation.x, translation.y, translation.z);

	XMFLOAT3 objectDimensions = XMFLOAT3(2.0f * scaling.x, 2.0f * scaling.y, 8.0f * scaling.z);
	XMFLOAT3 center = XMFLOAT3((objectDimensions.x / 2), (objectDimensions.y / 2), 0); // hosue built around center point
	XMFLOAT3 extents = XMFLOAT3(objectDimensions.x / 2, objectDimensions.y / 2, objectDimensions.z / 2);
	BoundingBox boundingBox = BoundingBox(center, extents);

	float boundingBoxScale = 1.0f;
	XMVECTOR positionVector = XMLoadFloat3(&position);
	boundingBox.Transform(boundingBox, boundingBoxScale, bulletRotationQuaternion, positionVector);

	auto physicsObject = std::make_unique<PhysicsObject>(position, center, rotationQuaternion, boundingBox, mass, coefficientFriction, stepTime);
	objBulletItem->physics = physicsObject.get();

	// Define Game Objects
	auto gameObject = std::make_unique<GameObject>(physicsObject.get(), orientationRadians, XMFLOAT3(1000.0f, 1000.0f, 1000.0f));

	mAllRitems.push_back(std::move(objBulletItem));
	allPhysicsObjects.push_back(std::move(physicsObject));
	allGameObjects.push_back(std::move(gameObject));
}

void CameraAndDynamicIndexingApp::generateBullet() {
	UINT objCBIndex = mAllRitems.size();
	XMFLOAT3 bulletScaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 bulletTranslation = XMFLOAT3(0.0f, 20.0f,0.0f);
	float bulletOrientationRadians = 0.0f;
	BuildBullet(bulletScaling, bulletTranslation, bulletOrientationRadians, objCBIndex);
}

void CameraAndDynamicIndexingApp::BuildRenderItems()
{
	UINT objCBIndex = 0;
	float x1 = MathHelper::RandF(-130.0f, 130.0f);
	float z1 = MathHelper::RandF(-130.0f, 130.0f);
	float y2 = GetHillsHeight(x1, z1);

	XMFLOAT3 tankScaling = XMFLOAT3(4.0f, 4.0f, 4.0f);
	XMFLOAT3 tankTranslation = XMFLOAT3(35.0f, -1.0f, 1.0f);
	float tankOrientationRadians = 0.5 * pi;
	BuildTank(tankScaling, tankTranslation, tankOrientationRadians, objCBIndex);

	// This object is player object
	playerGameObject = allGameObjects[objCBIndex - 1].get();

	XMFLOAT3 houseScaling = XMFLOAT3(14.0f, 14.0f, 14.0f);
	XMFLOAT3 houseTranslation = XMFLOAT3(150.0f, 0.0f, 0.0f);
	float houseOrientationRadians = 0.0f;
	BuildHouse(houseScaling, houseTranslation, houseOrientationRadians, objCBIndex);

	XMFLOAT3 treeScaling = XMFLOAT3(10.0f, 10.0f, 10.0f);
	XMFLOAT3 treeTranslation = XMFLOAT3(-100.0f, 0.0f, 0.0f);
	float treeOrientationRadians = 0.0f;
	BuildTree(treeScaling, treeTranslation, treeOrientationRadians, objCBIndex);

	XMFLOAT3 tank2Scaling = XMFLOAT3(4.0f, 4.0f, 4.0f);
	XMFLOAT3 tank2Translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float tank2OrientationRadians = 0.0f;
	BuildTank(tank2Scaling, tank2Translation, tank2OrientationRadians, objCBIndex);

	auto wavesRitem = std::make_unique<RenderItem>();
	wavesRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&wavesRitem->TexTransform, XMMatrixScaling(15.0f, 15.0f, 1.0f));
	wavesRitem->ObjCBIndex = objCBIndex++;
	wavesRitem->Mat = mMaterials["water"].get();
	wavesRitem->Geo = mGeometries["waterGeo"].get();
	wavesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mWavesRitem = wavesRitem.get();

	mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRitem->ObjCBIndex = objCBIndex++;
	gridRitem->Mat = mMaterials["grass"].get();
	gridRitem->Geo = mGeometries["landGeo"].get();
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	auto objMod = std::make_unique<RenderItem>();
	objMod->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&objMod->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	objMod->ObjCBIndex = objCBIndex++;
	objMod->Mat = mMaterials["water"].get();
	objMod->Geo = mGeometries["waterGeo"].get();
	objMod->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	objMod->IndexCount = objMod->Geo->DrawArgs["grid"].IndexCount;
	objMod->StartIndexLocation = objMod->Geo->DrawArgs["grid"].StartIndexLocation;
	objMod->BaseVertexLocation = objMod->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Transparent].push_back(objMod.get());

	mAllRitems.push_back(std::move(objMod));
	mAllRitems.push_back(std::move(wavesRitem));
	mAllRitems.push_back(std::move(gridRitem));
	/*mAllRitems.push_back(std::move(boxRitem));*/

	XMFLOAT3 bulletScaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 bulletTranslation = XMFLOAT3(5.0f, 7.0f, -200.0f);
	//XMFLOAT3 bulletTranslation = XMFLOAT3(0.0f, 5.0f, 0.0f);
	float bulletOrientationRadians = pi;
	BuildBullet(bulletScaling, bulletTranslation, bulletOrientationRadians, objCBIndex);
	mAllRitems[objCBIndex - 1].get()->physics->setVelocity(0, 0, 100);

	// All physics objects shoudl be poart of pysics world
	for (auto& physicsObject : allPhysicsObjects)
		physicsWorld.AddObject(physicsObject.get());

	for (auto& gameObject : allGameObjects)
		gameObjects.push_back(gameObject.get());
}

void CameraAndDynamicIndexingApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CameraAndDynamicIndexingApp::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

float CameraAndDynamicIndexingApp::GetHillsHeight(float x, float z)const
{
	//return 0.2f * (z * sinf(0.2f * x) + x * cosf(0.01f * z));
	//return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	//return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	//return 5.0f * sin(x/20 - pi/2) + 5.0f;
	return 0.5f;
}

XMFLOAT3 CameraAndDynamicIndexingApp::GetHillsNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}
