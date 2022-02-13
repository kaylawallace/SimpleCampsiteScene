//
// Game.cpp
//
// Adapted from https://github.com/microsoft/DirectXTK documentation / tutorials 

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

// Scene bounds (used to restrict camera movement)
namespace
{
    const XMVECTORF32 INIT_POS = { 2.f, -10.f, -1.5f, 0.f };
    const XMVECTORF32 SCENE_BOUNDS = { 20.f, 20.f, 20.f, 0.f };
    constexpr float ROT_SPEED = 0.01f;
    constexpr float MOV_SPEED = 0.05f;
}

// Constructor 
Game::Game() noexcept(false) :
    m_pitch(0),
    m_yaw(0),
    m_camPos(INIT_POS)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

// Only runs if DXTK_AUDIO flag is defined in pch.h
#ifdef DXTK_AUDIO
    m_retryAudio = false;
#endif
}

// Deconstructor 
// Suspends the audio engine and resets the audio loop 
Game::~Game()
{
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
    m_audLoop.reset();
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // Initialise mouse, keyboard and gamepad for input
    m_gamePad = std::make_unique<GamePad>();
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    // Set up light 
    m_Light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
    m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light.setPosition(-20.0f, 10.0f, -15.0f);
    m_Light.setDirection(-1.0f, -1.0f, 1.0f);

    // Set up audio
#ifdef DXTK_AUDIO
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;

#ifdef _DEBUG
    eflags |= AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    // Set the audio volume value 
    volume = .7f;

   // Load in the .wav file and create a sound effect instance to play the audio in the scene 
    m_ambient = std::make_unique<SoundEffect>(m_audEngine.get(), L"Audio/musicmono_adpcm.wav");
    m_ambientInstance = m_ambient->CreateInstance();

    // Set volume of instance to volume set above and play the audio
    m_ambientInstance->SetVolume(volume);
    m_ambientInstance->Play(true);
#endif 

}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();

#ifdef DXTK_AUDIO
    if (m_retryAudio)
    {
        m_retryAudio = false;
        if (m_audEngine->Reset())
        {
            if (m_ambientInstance)
            {
                m_ambientInstance->Play();
            }
        }
    }
    // Only update audio engine once per frame
    else if (m_audEngine->Update())
    {
        if (m_audEngine->IsCriticalError()) {
            m_retryAudio = true;
        }
    }
#endif
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float delta = float(timer.GetElapsedSeconds());
    auto time = static_cast<float>(timer.GetTotalSeconds());

// Region holding all mouse input info
#pragma region MouseInput
    auto mouse = m_mouse->GetState();
    m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 

    // If the left mouse button is pressed, change the mouse state to relative (allowing camera control)
    if (m_MouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
        m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
    }
    // Else if left mouse button is released, change the state back to absolute 
    // (revoking control so user can use their mouse as normal on their screen)
    else if (m_MouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED) {
        m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    }

    // Allow mouse control of the camera when the mouse mode is relative 
    if (mouse.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f) * ROT_SPEED;
        m_pitch -= delta.y;
        m_yaw -= delta.x;
    }


#pragma endregion 

// Region holding all keyboard input info 
#pragma region KeyboardInput
    auto kb = m_keyboard->GetState();
    
    // Exit game on 'Esc' press 
    if (kb.Escape)
    {
        ExitGame();
    }
    
    // Reset camera position and rotation on 'R' press 
    if (kb.R)
    {
        m_camPos = INIT_POS.v;
        m_pitch = m_yaw = 0;
    }

    // Initialise move vector for camera movement
    Vector3 move = Vector3::Zero;

    // Move up
    if (kb.Space)
        move.y += 1.f;
    // Move down 
    if (kb.LeftControl)
        move.y -= 1.f;
    // Move left
    if ( kb.A)
        move.x += 1.f;
    // Move right
    if (kb.Right || kb.D)
        move.x -= 1.f;
    // Move forwards 
    if (kb.W)
        move.z += 1.f;
    // Move backwards
    if (kb.S)
        move.z -= 1.f;
#pragma endregion

// Region holding gamepad input info 
#pragma region GamePad
    auto pad = m_gamePad->GetState(0);

    // Only run if a gamepad controller is connected
    if (pad.IsConnected())
    {
        // Quit the application 
        if (pad.IsViewPressed())
        {
            ExitGame();
        }

        // Reset the camera rotation 
        if (pad.IsLeftStickPressed())
        {
            m_yaw = m_pitch = 0.f;
        }
        // Rotate the camera based on the position of the left analog stick
        else
        {
            constexpr float ROT_SPEED = 0.1f;
            m_yaw += -pad.thumbSticks.leftX * ROT_SPEED;
            m_pitch += pad.thumbSticks.leftY * ROT_SPEED;
        }
    }

#pragma endregion

// Region containing camera movement/rotation calculations 
#pragma region CameraMovement
    
    // Create a quaternion for camera rotation
    // Uses yaw and pitch (x- and y-axis rotation), but not roll (z-axis rotation) as it is not very common in games / is an unsettling effect
    Quaternion q = Quaternion::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.f);

    // Transform the move vector by the current position and rotation
    move = Vector3::Transform(move, q);

    // Add speed to the camera movement
    move *= MOV_SPEED;

    // Move the camera 
    m_camPos += move;

    // Limit the position of the camera within the SCENE_BOUNDS 
    Vector3 halfBounds = (Vector3(SCENE_BOUNDS.v) / Vector3(2.f))
        - Vector3(0.1f, 0.1f, 0.1f);

    m_camPos = Vector3::Min(m_camPos, halfBounds);
    m_camPos = Vector3::Max(m_camPos, -halfBounds);

    // Limit pitch rotation to straight up/straight down
    constexpr float limit = XM_PIDIV2 - 0.01f;
    m_pitch = __max(-limit, m_pitch);
    m_pitch = __min(+limit, m_pitch);

    // Limit yaw rotation by wrapping  
    if (m_yaw > XM_PI)
    {
        m_yaw -= XM_2PI;
    }
    else if (m_yaw < -XM_PI)
    {
        m_yaw += XM_2PI;
    }

    // Camera rotation calculations 
    // sinf/cosf - calculate the sine of a float 
    float y = sinf(m_pitch);
    float r = cosf(m_pitch);
    float z = r * cosf(m_yaw);
    float x = r * sinf(m_yaw);

    // Change the cameras lookAt vector based on the rotation calculations
    XMVECTOR lookAt = m_camPos + Vector3(x, y, z);

    // XMMatrixLookAtRH: Builds camera 'view' matrix for a right hand coord system 
    m_view = XMMatrixLookAtRH(m_camPos, lookAt, Vector3::Up);
#pragma endregion

}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // Start Render event 
    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    // Draw Text to the screen
    m_deviceResources->PIXBeginEvent(L"Draw sprite");
    m_sprites->Begin();
        m_font->DrawString(m_sprites.get(), L"CMP502: Assignment 2", XMFLOAT2(10, 10), Colors::Yellow);
    m_sprites->End();
    m_deviceResources->PIXEndEvent();

#pragma region ModelRendering

    // 
    // Initialise transformation matrices
    //
    Matrix translate;
    Matrix scale; 
    Matrix rotate;

    // Draw skybox before all other models 
    m_effect->SetView(m_view);
    m_sky->Draw(m_effect.get(), m_skyInputLayout.Get());

    // Set rendering states after skybox rendering 
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullClockwise());

    // Turn the basic lighting shader on
    m_BasicLightingShader.EnableShader(context);
    
    //
    // Model Rendering
    //
#pragma region ModelRendering

    // Ground model
    // Create transformations (translate, rotate, scale variables) and multiply with the world matrix to 
    // render models in required positions
    m_world = Matrix::Identity;
    translate = Matrix::CreateTranslation(0.f, -10.f, 0.f);
    scale = Matrix::CreateScale(2.f, 2.f, 2.f);
    m_world = m_world * translate;
    // Set the shader parameters before rendering to get the correct texture
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_grassTex.Get());
    // Render the model 
    m_groundModel.Render(context);

    // Rock platform model
    // No reset for m_world here so that platform can be transformed in relation to the ground model
    translate = Matrix::CreateTranslation(1.2f, 9.6f, 3.2f);
    scale = Matrix::CreateScale(2.f, 2.f, 2.f);
    rotate = Matrix::CreateRotationY(-.5f);
    m_world = m_world * rotate * scale * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_rockTex.Get());
    m_platform.Render(context);

    // Tent model
    m_world = SimpleMath::Matrix::Identity;
    translate = Matrix::CreateTranslation(1.2f, -10.25f, 3.3f);
    rotate = Matrix::CreateRotationY(1.2f);
    m_world = m_world * rotate * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_tentTex.Get());
    m_tent.Render(context);

    // SImple tree top model
    m_world = SimpleMath::Matrix::Identity;
    translate = Matrix::CreateTranslation(2.2f, -10.35f, 5.2f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeLeavesTex.Get());
    m_treeSimple.Render(context);

    // Simple tree trunk model
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeBarkTex.Get());
    m_treeSimpleTrunk.Render(context);

    // Mushroom model
    // No reset for m_world here so that mushroom can be placed in relation to the tree
    translate = Matrix::CreateTranslation(-0.2f, 0.f, -0.05f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_mushroomTex.Get());
    m_mushroom.Render(context);

    // Mushroom group model 
    // No reset for m_world here so that mushroom group can be placed in relation to the mushroom
    translate = Matrix::CreateTranslation(.4f, 0.f, -0.35f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_mushroomTex.Get());
    m_mushroomGroup.Render(context);

    // Tree stump model
    // No reset for m_world here so that stump can be placed in relation to the mushroom group
    translate = Matrix::CreateTranslation(-.7f, 0.f, 0.f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeBarkTex.Get());
    m_stump.Render(context);

    // Crop model
    m_world = Matrix::Identity;
    translate = Matrix::CreateTranslation(-.2f, -10.35f, 3.2f);
    scale = Matrix::CreateScale(.5f, .5f, .5f);
    m_world = m_world * scale * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_bambooTex.Get());
    m_crop.Render(context);

    // Crop model
    // m_world not reset to place each crop in relation to the last
    translate = Matrix::CreateTranslation(0.f, 0.f, -.25f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_bambooTex.Get());
    m_crop.Render(context);

    // Crop model
    translate = Matrix::CreateTranslation(0.f, 0.f, -.25f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_bambooTex.Get());
    m_crop.Render(context);
    
    // Crop model
    translate = Matrix::CreateTranslation(0.f, 0.f, -.25f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_bambooTex.Get());
    m_crop.Render(context);

    // Canoe model
    m_world = Matrix::Identity;
    translate = Matrix::CreateTranslation(0.65f, -10.35f, 1.3f);
    rotate = Matrix::CreateRotationY(.5f);
    m_world = m_world * rotate * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_woodGrainTex.Get());
    m_canoe.Render(context);

    // Canoe paddle
    // No reset for m_world here so that the canoe paddle can be placed in relation to the canoe
    translate = Matrix::CreateTranslation(0.4f, 0.f, 0.2f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_woodGrainTex.Get());
    m_canoePaddle.Render(context);

    // Mushroom group model 
    // No reset for m_world here so that mushroom group can be placed in relation to the canoe paddle
    translate = Matrix::CreateTranslation(1.3f, 0.f, 0.f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_mushroomTex.Get());
    m_mushroomGroup.Render(context);

    // Log model
    m_world = Matrix::Identity;
    translate = Matrix::CreateTranslation(2.6f, -10.35f, 2.4f);
    rotate = Matrix::CreateRotationY(.87f);
    m_world = m_world * rotate * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeBarkTex.Get());
    m_log.Render(context);

    // Campfire logs model
    // No reset for m_world here so that campfire logs can be placed in relation to the log model
    translate = Matrix::CreateTranslation(-.3f, 0.f, .4f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeBarkTex.Get());
    m_campfireLogs.Render(context);

    // Simple tree trunk model 
    // No reset for m_world here so that tree trunk can be placed in relation to the campfire logs
    translate = Matrix::CreateTranslation(1.25f, 0.f, 1.5f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeBarkTex.Get());
    m_treeSimpleTrunk.Render(context);

    // Simple tree top model
    // No reset for m_world here so that tree top can be placed in relation to the tree trunk
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeLeavesTex.Get());
    m_treeSimple.Render(context);

    // Fat tree top model 
    // No reset for m_world here so that tree top can be placed in relation to the last tree position
    translate = Matrix::CreateTranslation(-.5f, 0.f, -.2f);
    m_world = m_world * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeLeavesTex.Get());
    m_treeFat.Render(context);

    // Fat tree trunk model 
    // No reset for m_world here so that tree top can be placed in relation to the tree trunk
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_treeBarkTex.Get());
    m_treeFatTrunk.Render(context);

    // Custom geometry -- prism render 
    m_world = Matrix::Identity;
    translate = Matrix::CreateTranslation(3.5f, -10.35f, 2.2f);
    scale = Matrix::CreateScale(0.3f, 0.3f, 0.3f);
    m_world = m_world *  scale * translate;
    m_BasicLightingShader.SetShaderParameters(context, &m_world, &m_view, &m_proj, &m_Light, m_tentTex.Get());
    //m_prism.Render(context);   
#pragma endregion
   
    // End render event 
    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Game is becoming active window 
void Game::OnActivated()
{
    m_gamePad->Resume();
}

//Game is becoming background window
void Game::OnDeactivated()
{
    m_gamePad->Suspend();
}

// Game is being power-suspended (or minimized)
void Game::OnSuspending()
{
    // Suspend audio and game pad controls 
    m_gamePad->Suspend();
    m_audEngine->Suspend();
}

// Game is being power-resumed (or returning from minimize)
void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_audEngine->Resume();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();
    
    // Set DirectXTK objects
    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"Fonts/SegoeUI_18.spritefont");
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    // Load and set up shaders (vertex and pixel shader pairs)
    m_BasicLightingShader.InitStandard(device, L"light_vs.cso", L"light_ps.cso");

#pragma region InitializeModels
    // Initialize shapes and models 
    m_sky = GeometricPrimitive::CreateGeoSphere(context, 2.f, 3, false);
    m_room = GeometricPrimitive::CreateBox(context, XMFLOAT3(SCENE_BOUNDS[0], SCENE_BOUNDS[1], SCENE_BOUNDS[2]), false, true);
    m_prism.InitializePrism(device);
    m_sphere = GeometricPrimitive::CreateSphere(context);
    m_groundModel.InitializeModel(device, "Models/ground_block.obj");
    m_log.InitializeModel(device, "Models/log.obj");
    m_platform.InitializeModel(device, "Models/platform_grass.obj");
    m_tent.InitializeModel(device, "Models/tent_smallClosed.obj");
    m_treeSimple.InitializeModel(device, "Models/tree_simple_top.obj");
    m_treeSimpleTrunk.InitializeModel(device, "Models/tree_simple_trunk.obj");
    m_treeFat.InitializeModel(device, "Models/tree_dark_top.obj");
    m_treeFatTrunk.InitializeModel(device, "Models/tree_dark_trunk.obj");
    m_mushroomGroup.InitializeModel(device, "Models/mushroom_redGroup.obj");
    m_mushroom.InitializeModel(device, "Models/mushroom_tanTall.obj");
    m_canoe.InitializeModel(device, "Models/canoe.obj");
    m_canoePaddle.InitializeModel(device, "Models/canoe_paddle.obj");
    m_stump.InitializeModel(device, "Models/stump_round.obj");
    m_campfireLogs.InitializeModel(device, "Models/campfire_logs.obj");
    m_crop.InitializeModel(device, "Models/crop.obj");
#pragma endregion

    // Skybox effect and input layout 
    m_effect = std::make_unique<DX::SkyboxEffect>(device);
    m_sky->CreateInputLayout(m_effect.get(), m_skyInputLayout.ReleaseAndGetAddressOf());

#pragma region LoadTextures
    // Load in textures 
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/skybox3.dds", nullptr, m_cubemap.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/Grass_Base_Color.dds", nullptr, m_grassTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/Rock_Base_Color.dds", nullptr, m_rockTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/red-fabric.dds", nullptr, m_tentTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/Wood_Bark.dds", nullptr, m_treeBarkTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/Stylized_Leaves.dds", nullptr, m_treeLeavesTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/Mushroom_Top.dds", nullptr, m_mushroomTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/Wood_Grain.dds", nullptr, m_woodGrainTex.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Textures/bamboo_tex.dds", nullptr, m_bambooTex.ReleaseAndGetAddressOf()));
#pragma endregion

    // Set texture for skybox
    m_effect->SetTexture(m_cubemap.Get());

    // Set world to identity matrix
    m_world = Matrix::Identity;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f), Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(70.f), float(size.right) / float(size.bottom), 0.01f, 100.f);
    m_effect->SetProjection(m_proj);
}

void Game::OnDeviceLost()
{
   // Shape/model resets 
    m_room.reset();   
    m_sphere.reset();
    m_prism.Shutdown();
    m_groundModel.Shutdown();
    m_platform.Shutdown();
    m_tent.Shutdown();
    m_log.Shutdown();
    m_treeSimple.Shutdown();
    m_treeSimpleTrunk.Shutdown();
    m_treeFat.Shutdown();
    m_treeFatTrunk.Shutdown();
    m_mushroom.Shutdown();
    m_mushroomGroup.Shutdown();
    m_canoe.Shutdown();
    m_canoePaddle.Shutdown();
    m_campfireLogs.Shutdown();
    m_stump.Shutdown();
    m_crop.Shutdown();

    // Texture resets 
    m_cubemap.Reset();
    m_grassTex.Reset();
    m_rockTex.Reset();
    m_tentTex.Reset();
    m_treeBarkTex.Reset();
    m_treeLeavesTex.Reset();
    m_mushroomTex.Reset();
    m_bambooTex.Reset();
    m_woodGrainTex.Reset();

    // Skybox resets 
    m_sky.reset();
    m_effect.reset();
    m_skyInputLayout.Reset();
    m_cubemap.Reset();

    // DirectXTK object resets
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
    m_batch.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
