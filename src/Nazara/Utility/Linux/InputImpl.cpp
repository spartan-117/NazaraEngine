// Copyright (C) 2013 Alexandre Janniaux
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/Linux/InputImpl.hpp>
#include <Nazara/Utility/Linux/XDisplay.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Utility/Window.hpp>
#include <xcb/xcb.h>

#define XK_LATIN1
#define XK_MISCELLANY
#define XK_3270

#include <X11/keysymdef.h>
#include <Nazara/Utility/Debug.hpp>

namespace
{
	int vKeys[NzKeyboard::Count] = {
		// Lettres
		XK_A, // Key::A
		XK_B, // Key::B
		XK_C, // Key::C
		XK_D, // Key::D
		XK_E, // Key::E
		XK_F, // Key::F
		XK_G, // Key::G
		XK_H, // Key::H
		XK_I, // Key::I
		XK_J, // Key::J
		XK_K, // Key::K
		XK_L, // Key::L
		XK_M, // Key::M
		XK_N, // Key::N
		XK_O, // Key::O
		XK_P, // Key::P
		XK_Q, // Key::Q
		XK_R, // Key::R
		XK_S, // Key::S
		XK_T, // Key::T
		XK_U, // Key::U
		XK_V, // Key::V
		XK_W, // Key::W
		XK_X, // Key::X
		XK_Y, // Key::Y
		XK_Z, // Key::Z

		// Touches de fonction
		XK_F1,	// Key::F1
		XK_F2,	// Key::F2
		XK_F3,	// Key::F3
		XK_F4,	// Key::F4
		XK_F5,	// Key::F5
		XK_F6,	// Key::F6
		XK_F7,	// Key::F7
		XK_F8,	// Key::F8
		XK_F9,	// Key::F9
		XK_F10,	// Key::F10
		XK_F11,	// Key::F11
		XK_F12,	// Key::F12
		XK_F13,	// Key::F13
		XK_F14,	// Key::F14
		XK_F15,	// Key::F15

		// Flèches directionnelles
		XK_Down,  // Key::Down
		XK_Left,  // Key::Left
		XK_Right, // Key::Right
		XK_Up,	  // Key::Up

		// Pavé numérique
		XK_KP_Add,		 // Key::Add
		XK_KP_Divide,	 // Key::Divide
		XK_KP_Multiply, // Key::Multiply
		XK_KP_0,	 // Key::Numpad0
		XK_KP_1,	 // Key::Numpad1
		XK_KP_2,	 // Key::Numpad2
		XK_KP_3,	 // Key::Numpad3
		XK_KP_4,	 // Key::Numpad4
		XK_KP_5,	 // Key::Numpad5
		XK_KP_6,	 // Key::Numpad6
		XK_KP_7,	 // Key::Numpad7
		XK_KP_8,	 // Key::Numpad8
		XK_KP_9,	 // Key::Numpad9
		XK_KP_Subtract, // Key::Subtract

		// Divers
		XK_backslash,	   // Key::Backslash
		XK_BackSpace,	   // Key::Backspace
		XK_Clear,	   // Key::Clear
		XK_comma,  // Key::Comma,
		XK_minus,  // Key::Dash
		XK_Delete,	   // Key::Delete
		XK_End,		   // Key::End
		XK_equal,   // Key::Equal
		XK_Escape,	   // Key::Escape
		XK_Home,	   // Key::Home
		XK_Insert,	   // Key::Insert
		XK_Alt_L,	   // Key::LAlt
		XK_bracketleft,	   // Key::LBracket
		XK_Control_L,   // Key::LControl
		XK_Shift_L,	   // Key::LShift
		XK_Super_L,	   // Key::LSystem
		XK_0,		   // Key::Num0
		XK_1,		   // Key::Num1
		XK_2,		   // Key::Num2
		XK_3,		   // Key::Num3
		XK_4,		   // Key::Num4
		XK_5,		   // Key::Num5
		XK_6,		   // Key::Num6
		XK_7,		   // Key::Num7
		XK_8,		   // Key::Num8
		XK_9,		   // Key::Num9
		XK_Page_Down,	   // Key::PageDown
		XK_Page_Up,	   // Key::PageUp
		XK_Pause,	   // Key::Pause
		XK_period, // Key::Period,
		XK_Print,	   // Key::Print
		XK_3270_PrintScreen,   // Key::PrintScreen TODO: CHECK
		XK_quotedbl,	   // Key::Quote
		XK_Alt_R,	   // Key::RAlt
		XK_bracketright,	   // Key::RBracket
		XK_Control_R,   // Key::RControl
		XK_Return,	   // Key::Return
		XK_Shift_R,	   // Key::RShift
		XK_Super_R,	   // Key::RSystem
		XK_semicolon,	   // Key::Semicolon
		XK_slash,	   // Key::Slash
		XK_space,	   // Key::Space
		XK_Tab,		   // Key::Tab
		XK_asciitilde,	   // Key::Tilde TODO CHECK
/*
		// Touches navigateur
		XK_BROWSER_BACK,	  // Key::Browser_Back
		XK_BROWSER_FAVORITES, // Key::Browser_Favorites
		XK_BROWSER_FORWARD,	  // Key::Browser_Forward
		XK_BROWSER_HOME,	  // Key::Browser_Home
		XK_BROWSER_REFRESH,	  // Key::Browser_Refresh
		XK_BROWSER_SEARCH,	  // Key::Browser_Search
		XK_BROWSER_STOP,	  // Key::Browser_Stop

		// Touches de contrôle
		XK_MEDIA_NEXT_TRACK, // Key::Media_Next,
		XK_MEDIA_PLAY_PAUSE, // Key::Media_PlayPause,
		XK_MEDIA_PREV_TRACK, // Key::Media_Previous,
		XK_MEDIA_STOP,		 // Key::Media_Stop,

		// Touches de contrôle du volume
		XK_VOLUME_DOWN,	// Key::Volume_Down
		XK_VOLUME_MUTE,	// Key::Volume_Mute
		XK_VOLUME_UP//,	// Key::Volume_Up
*/
		// Touches à verrouillage
		XK_Caps_Lock,	// Key::CapsLock
		XK_Num_Lock,	// Key::NumLock
		XK_Scroll_Lock	// Key::ScrollLock
        
	};
}

NzString NzEventImpl::GetKeyName(NzKeyboard::Key key)
{
    wchar_t keyName[20]; // Je ne pense pas que ça dépassera 20 caractères

	return NzString::Unicode(keyName);
}

NzVector2i NzEventImpl::GetMousePosition()
{
	auto connection = nzOpenXCBConnection();

    auto cookie = xcb_query_pointer(connection, XCB_NONE);
    auto reply = xcb_query_pointer_reply(connection, cookie, NULL);    
    nzCloseXCBConnection(connection);

    return NzVector2i(reply->root_x, reply->root_y);

}

NzVector2i NzEventImpl::GetMousePosition(const NzWindow& relativeTo)
{
    auto connection = nzOpenXCBConnection();
    auto handle = static_cast<xcb_window_t>(relativeTo.GetHandle());

	if (handle)
	{
        auto cookie = xcb_query_pointer(connection, handle);
        auto reply = xcb_query_pointer_reply(connection, cookie, NULL);    
        nzCloseXCBConnection(connection);
		return NzVector2i(reply->win_x, reply->win_y);
	}
	else
	{
        nzCloseXCBConnection(connection);
		NazaraError("Window's handle is invalid");

		// Attention que (-1, -1) est une position tout à fait valide et ne doit pas être utilisée pour tester l'erreur
		return NzVector2i(-1, -1);
	}
}

bool NzEventImpl::IsKeyPressed(NzKeyboard::Key key)
{
	switch (key)
	{
		case NzKeyboard::CapsLock:
		case NzKeyboard::NumLock:
		case NzKeyboard::ScrollLock:

		default:
            break;
	}
    return false; // TODO
}

bool NzEventImpl::IsMouseButtonPressed(NzMouse::Button button)
{
	static int vButtons[NzMouse::Max+1] = {
		//XK_LBUTTON,	 // Button::Left
		//XK_MBUTTON,	 // Button::Middle
		//XK_RBUTTON,	 // Button::Right
		//XK_XBUTTON1, // Button::XButton1
		//XK_XBUTTON2	 // Button::XButton2
	};

	// Gestion de l'inversement des boutons de la souris
	switch (button)
	{
		case NzMouse::Left:
			break;

		case NzMouse::Right:
			break;

		default:
			break;
	}

    return false; //TODO
}

void NzEventImpl::SetMousePosition(int x, int y)
{
    int screenNum = -1;
    auto connection = nzOpenXCBConnection(&screenNum); // Increment reference counter
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for(int i = 0; i<screenNum; ++i)
    {
        xcb_screen_next(&iter);
    }
    
    // http://www.x.org/releases/current/doc/man/man3/xcb_warp_pointer.3.xhtml
    xcb_warp_pointer_checked(
        connection,
        None,
        iter.data->root,
        0, 0, 0, 0,
        x, y
    );
    xcb_flush(connection);
    nzCloseXCBConnection(connection);
}

void NzEventImpl::SetMousePosition(int x, int y, const NzWindow& relativeTo)
{
    auto handle = static_cast<xcb_window_t>(relativeTo.GetHandle());
	if (handle)
	{
        auto connection = nzOpenXCBConnection(); // Increment reference counter

        // http://www.x.org/releases/current/doc/man/man3/xcb_warp_pointer.3.xhtml
        xcb_warp_pointer_checked(
            connection,
            None,
            handle,
            0, 0, 0, 0,
            x, y
        );
        xcb_flush(connection);
        nzCloseXCBConnection(connection);
	}
	else
		NazaraError("Window's handle is invalid");
}
