#include "pch.h"
#include "Input.h"

Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialise(HWND window)
{
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(window);
	m_quitApp = false;

	m_GameInput.forward = false;
	m_GameInput.back = false;
	m_GameInput.right = false;
	m_GameInput.left = false;
	m_GameInput.rotRight = false;
	m_GameInput.rotLeft = false;
	m_GameInput.rotUp = false;
	m_GameInput.rotDown = false;
	m_GameInput.mouseX = 0.0f;
	m_GameInput.mouseY = 0.0f;
}

void Input::Update()
{
	auto kb = m_keyboard->GetState();	//updates the basic keyboard state
	m_KeyboardTracker.Update(kb);		//updates the more feature filled state. Press / release etc. 
	auto mouse = m_mouse->GetState();   //updates the basic mouse state
	m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 

	if (kb.Escape)// check has escape been pressed.  if so, quit out. 
	{
		m_quitApp = true;
	}

	//A key
	if (kb.A)	m_GameInput.left = true;
	else		m_GameInput.left = false;

	//D key
	if (kb.D)	m_GameInput.right = true;
	else		m_GameInput.right = false;

	//W key
	if (kb.W)	m_GameInput.forward = true;
	else		m_GameInput.forward = false;

	//S key
	if (kb.S)	m_GameInput.back = true;
	else		m_GameInput.back = false;

	if (kb.Space)	m_GameInput.up = true;
	else			m_GameInput.up = false;

	if (kb.LeftControl)	m_GameInput.down = true;
	else				m_GameInput.down = false;

	if (kb.R)			m_GameInput.reset = true;
	else				m_GameInput.reset = false;

	if (m_MouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
		m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	}
	else if (m_MouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED) {
		m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	}

	if (mouse.positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		m_GameInput.mouseX = mouse.x;
		m_GameInput.mouseY = mouse.y;

		if (mouse.x > 0) {
			m_GameInput.rotRight = true;
			m_GameInput.rotLeft = false;
		}
		else {
			m_GameInput.rotLeft = true;
			m_GameInput.rotRight = false;
		}

		if (mouse.y > 0) {
			m_GameInput.rotUp = false;
			m_GameInput.rotDown = true;
		}
		else {
			m_GameInput.rotDown = false;
			m_GameInput.rotUp = true;
		}

	}
}

bool Input::Quit()
{
	return m_quitApp;
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}

//float Input::GetMousePosX()
//{
//	return mouse.x;
//}
//
//float Input::GetMousePosY()
//{
//	return mouse.y;
//}
