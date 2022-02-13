//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "pch.h"
#include "modelclass.h"
#include "Shader.h"
#include "Light.h"
#include "SkyboxEffect.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice() noexcept { m_retryAudio = true; };
#endif

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer m_timer;

    // User input 
    std::unique_ptr<DirectX::Keyboard> m_keyboard;
    std::unique_ptr<DirectX::Mouse> m_mouse;
    DirectX::Mouse::ButtonStateTracker m_MouseTracker;
    std::unique_ptr<DirectX::GamePad> m_gamePad;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::BasicEffect> m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory> m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch> m_sprites;
    std::unique_ptr<DirectX::SpriteFont> m_font;

    // Scene Objects
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_batchInputLayout;

    // Matrices for calculations in scene 
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;
    DirectX::SimpleMath::Matrix m_world;

    // Camera 
    DirectX::SimpleMath::Vector3 m_camPos;
    float m_pitch;
    float m_yaw;


    // Light
    Light m_Light;

    // Skybox 
    std::unique_ptr<DirectX::GeometricPrimitive> m_sky;
    std::unique_ptr<DX::SkyboxEffect> m_effect; 
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyInputLayout;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubemap;

    // Audio - define variables if flag is declared 
#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine> m_audEngine;
    std::unique_ptr<DirectX::SoundEffect> m_ambient;
    std::unique_ptr<DirectX::SoundEffectInstance> m_ambientInstance;
    std::unique_ptr<SoundEffectInstance> m_audLoop;
   
    uint32_t m_audioEvent;
    float m_audioTimerAcc;
    float volume;
    bool m_retryAudio;
#endif

    //Shaders
    Shader m_BasicLightingShader;

    // Geometric primitive shapes/Models 
    std::unique_ptr<DirectX::GeometricPrimitive> m_room;
    std::unique_ptr<DirectX::GeometricPrimitive> m_sphere;
    ModelClass m_prism;

    ModelClass m_log;
    ModelClass m_groundModel;
    ModelClass m_groundLarge;
    ModelClass m_platform;
    ModelClass m_treeSimple;
    ModelClass m_treeSimpleTrunk;
    ModelClass m_treeFat;
    ModelClass m_treeFatTrunk;
    ModelClass m_mushroomGroup;
    ModelClass m_mushroom;
    ModelClass m_canoe;
    ModelClass m_canoePaddle;
    ModelClass m_tent;
    ModelClass m_stump;
    ModelClass m_campfireLogs;
    ModelClass m_crop;


    // Textures
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_grassTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_rockTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_treeBarkTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_treeLeavesTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_mushroomTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_woodGrainTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bambooTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pumpkinTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tentTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_waterTex;
};