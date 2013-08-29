// Copyright (C) 2012 Alexandre Janniaux
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

// Un grand merci à Laurent Gomila pour la SFML qui m'aura bien aidé à réaliser cette implémentation

#define OEMRESOURCE

#include <Nazara/Core/ConditionVariable.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Core/Mutex.hpp>
#include <Nazara/Core/Thread.hpp>
#include <Nazara/Utility/Config.hpp>
#include <Nazara/Utility/Cursor.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Nazara/Utility/Icon.hpp>
#include <cstdio>
#include <Nazara/Utility/Debug.hpp>
#include <linux/input.h>
#include <Nazara/Utility/Linux/WindowImpl.hpp>
#include <Nazara/Utility/Linux/XDisplay.hpp>
#include <Nazara/Utility/Linux/CursorImpl.hpp>


// N'est pas définit avec MinGW apparemment
#ifndef MAPVK_VK_TO_VSC
    #define MAPVK_VK_TO_VSC 0
#endif

//#undef IsMinimized // Conflit avec la méthode du même nom

namespace
{
	const wchar_t* className = L"Nazara Window";
	NzWindowImpl* fullscreenWindow = nullptr;
}

NzWindowImpl::NzWindowImpl(NzWindow* parent) :
m_cursor(XCB_NONE),
m_handle(XCB_NONE),
m_maxSize(-1),
m_minSize(-1),
m_parent(parent),
m_keyRepeat(true),
m_mouseInside(false),
m_smoothScrolling(false),
m_scrolling(0)
{
    m_connection = nzOpenXCBConnection();
}

NzWindowImpl::~NzWindowImpl() {
    nzCloseXCBConnection(m_connection);
}

bool NzWindowImpl::Create(NzVideoMode mode, const NzString& title, nzUInt32 style)
{
    int screenNb = -1;
	auto connection = nzOpenXCBConnection(&screenNb);
    bool fullscreen = (style & nzWindowStyle_Fullscreen) != 0;
	unsigned int x, y;
	unsigned int width = mode.width;
	unsigned int height = mode.height;
	if (fullscreen)
	{
        /*xcb_change_property(
            m_connection, 
            XCB_PROP_MODE_REPLACE, 
            m_handle, 
            EWMH._NET_WM_STATE, 
            XCB_ATOM_ATOM, 
            32, 
            1, 
            &(EWMH._NET_WM_STATE_FULLSCREEN)
        );*/

		/*if (ChangeDisplaySettings(&win32Mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// Situation extrêmement rare grâce à NzVideoMode::IsValid appelé par NzWindow
			NazaraError("Failed to change display settings for fullscreen, this video mode is not supported by your computer");
			fullscreen = false;
		}*/
	}

	// Testé une seconde fois car sa valeur peut changer
	if (fullscreen)
	{
		x = 0;
		y = 0;

    

		fullscreenWindow = this;
	}
	else
	{
		/*win32Style = WS_VISIBLE;
		if (style & nzWindowStyle_Titlebar)
		{
			win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
			if (style & nzWindowStyle_Closable)
				win32Style |= WS_SYSMENU;

			if (style & nzWindowStyle_Resizable)
				win32Style |= WS_MAXIMIZEBOX | WS_SIZEBOX;
		}
		else
			win32Style |= WS_POPUP;

		win32StyleEx = 0; */

/*		RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
		AdjustWindowRect(&rect, win32Style, false);
		width = rect.right-rect.left;
		height = rect.bottom-rect.top;

		x = (GetSystemMetrics(SM_CXSCREEN) - width)/2;
		y = (GetSystemMetrics(SM_CYSCREEN) - height)/2; */
	}



	#if NAZARA_UTILITY_THREADED_WINDOW

	NzMutex mutex;
	NzConditionVariable condition;
	m_threadActive = true;

	// On attend que la fenêtre soit créée
	mutex.Lock();
	//m_thread = new NzThread(WindowThread, &m_handle, win32StyleEx, wtitle, win32Style, x, y, width, height, this, &mutex, &condition);
	condition.Wait(&mutex);
	mutex.Unlock();
	#else

    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for(int i = 0; i<screenNb; ++i)
    {
        xcb_screen_next(&iter);
    }    
    auto screen = iter.data;

    uint32_t mask;
    uint32_t values[2];
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = screen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE         | XCB_EVENT_MASK_POINTER_MOTION                     
                | XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE
                | XCB_EVENT_MASK_BUTTON_PRESS   | XCB_EVENT_MASK_BUTTON_RELEASE
                | XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW;

	m_handle = xcb_generate_id(m_connection);
	xcb_create_window(m_connection      // X11 connection
        ,XCB_COPY_FROM_PARENT           // depth (same as root)
        ,m_handle                       // window ID
        ,screen->root                   // parent window
        ,x, y                           // x, y
        ,width, height                  // width, height
        ,1                              // border width
        ,XCB_WINDOW_CLASS_INPUT_OUTPUT  // class
        ,screen->root_visual            // visual
        ,mask, values                   // masks
    );
    this->SetTitle(title);
    xcb_flush(m_connection);
	#endif


	if (fullscreen)
	{
        const uint32_t value = XCB_STACK_MODE_ABOVE;
        xcb_configure_window(
            m_connection,
            m_handle,
            XCB_CONFIG_WINDOW_STACK_MODE,
            &value
        );

        xcb_map_window(m_connection, m_handle);
	}

	m_eventListener = true;
	m_ownsWindow = true;
	#if !NAZARA_UTILITY_THREADED_WINDOW
	m_sizemove = false;
	#endif

	return m_handle != XCB_NONE;
}

bool NzWindowImpl::Create(NzWindowHandle handle)
{
	if (!handle)
	{
		NazaraError("Invalid handle");
		return false;
	}

	m_handle = static_cast<xcb_window_t>(handle);
	m_eventListener = false;
	m_ownsWindow = false;
	#if !NAZARA_UTILITY_THREADED_WINDOW
	m_sizemove = false;
	#endif

	return true;
}

void NzWindowImpl::Destroy()
{
	if (m_ownsWindow)
	{
		#if NAZARA_UTILITY_THREADED_WINDOW
		if (m_thread)
		{
			m_threadActive = false;
			//PostMessageW(m_handle, WM_NULL, 0, 0); // Pour réveiller le thread

			m_thread->Join();
			delete m_thread;
		}
		#else
		if (m_handle)
            xcb_destroy_window(m_connection,m_handle);
		#endif
	}
	else
		SetEventListener(false);
}

void NzWindowImpl::EnableKeyRepeat(bool enable)
{
	m_keyRepeat = enable;
}

void NzWindowImpl::EnableSmoothScrolling(bool enable)
{
	m_smoothScrolling = enable;
}

NzWindowHandle NzWindowImpl::GetHandle() const
{
	return m_handle;
}

unsigned int NzWindowImpl::GetHeight() const
{
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
            m_connection,
            xcb_get_geometry(m_connection, m_handle),
            NULL
        );
    unsigned int height = geom->height;
    delete geom;
	return height;
}

NzVector2i NzWindowImpl::GetPosition() const
{
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
            m_connection,
            xcb_get_geometry(m_connection, m_handle),
            NULL
        );
    NzVector2i position (geom->x, geom->y);
    delete geom;
	return position;
}

NzVector2ui NzWindowImpl::GetSize() const
{
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
            m_connection,
            xcb_get_geometry(m_connection, m_handle),
            NULL
        );
    NzVector2ui size (geom->width, geom->height);
    delete geom;
	return size;
}

NzString NzWindowImpl::GetTitle() const
{
}

unsigned int NzWindowImpl::GetWidth() const
{
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
            m_connection,
            xcb_get_geometry(m_connection, m_handle),
            NULL
        );
    unsigned int width = geom->width;
    delete geom;
	return width;
}

bool NzWindowImpl::HasFocus() const
{
	//return GetForegroundWindow() == m_handle;
}

void NzWindowImpl::IgnoreNextMouseEvent(int mouseX, int mouseY)
{
	// Petite astuce ...
	m_mousePos.x = mouseX;
	m_mousePos.y = mouseY;
}

bool NzWindowImpl::IsMinimized() const
{
	//return IsIconic(m_handle);
}

bool NzWindowImpl::IsVisible() const
{
	//return IsWindowVisible(m_handle);
}

void NzWindowImpl::ProcessEvents(bool block)
{
	if (m_ownsWindow)
	{
        xcb_generic_event_t* event = nullptr;
		if (block)
            event = xcb_wait_for_event(m_connection);
        else
            event = xcb_poll_for_event(m_connection);

        HandleEvent(event);
        free(event);
	}
}

void NzWindowImpl::SetCursor(nzWindowCursor cursor)
{
	switch (cursor)
	{
		case nzWindowCursor_Crosshair:
			//m_cursor = ;
			break;

		case nzWindowCursor_Default:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_Hand:
		case nzWindowCursor_Pointer:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_HAND, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_Help:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_HELP, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_Move:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_SIZEALL, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_None:
			//m_cursor = nullptr;
			break;

		case nzWindowCursor_Progress:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_APPSTARTING, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_ResizeN:
		case nzWindowCursor_ResizeS:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_SIZENS, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_ResizeNW:
		case nzWindowCursor_ResizeSE:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_SIZENWSE, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_ResizeNE:
		case nzWindowCursor_ResizeSW:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_SIZENESW, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_ResizeE:
		case nzWindowCursor_ResizeW:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_SIZEWE, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_Text:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_IBEAM, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;

		case nzWindowCursor_Wait:
			//m_cursor = reinterpret_cast<HCURSOR>(LoadImage(nullptr, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_SHARED));
			break;
	}

	// Pas besoin de libérer le curseur par la suite s'il est partagé
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms648045(v=vs.85).aspx
	// ::SetCursor(m_cursor); TODO
}

void NzWindowImpl::SetCursor(const NzCursor& cursor)
{
	m_cursor = cursor.m_impl->GetCursor();

	//::SetCursor(m_cursor); TODO
}

void NzWindowImpl::SetEventListener(bool listener)
{
	if (m_ownsWindow)
		m_eventListener = listener;
	else if (listener != m_eventListener)
	{
		if (listener)
		{
			//SetWindowLongPtr(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			//m_callback = SetWindowLongPtr(m_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MessageHandler));
			m_eventListener = true;
		}
		else if (m_eventListener)
		{
			//SetWindowLongPtr(m_handle, GWLP_WNDPROC, m_callback);
			m_eventListener = false;
		}
	}
}

void NzWindowImpl::SetFocus()
{
	//SetForegroundWindow(m_handle);
}

void NzWindowImpl::SetIcon(const NzIcon& icon)
{
    /*
	HICON iconHandle = icon.m_impl->GetIcon();

	SendMessage(m_handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(iconHandle));
	SendMessage(m_handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(iconHandle));
    */
}

void NzWindowImpl::SetMaximumSize(int width, int height)
{
    /*
	RECT rect = {0, 0, width, height};
	AdjustWindowRect(&rect, GetWindowLongPtr(m_handle, GWL_STYLE), false);

	if (width != -1)
		m_maxSize.x = rect.right-rect.left;
	else
		m_maxSize.x = -1;

	if (height != -1)
		m_maxSize.y = rect.bottom-rect.top;
	else
		m_maxSize.y = -1;
        */
}

void NzWindowImpl::SetMinimumSize(int width, int height)
{
/*	RECT rect = {0, 0, width, height};
	AdjustWindowRect(&rect, GetWindowLongPtr(m_handle, GWL_STYLE), false);

	if (width != -1)
		m_minSize.x = rect.right-rect.left;
	else
		m_minSize.x = -1;

	if (height != -1)
		m_minSize.y = rect.bottom-rect.top;
	else
		m_minSize.y = -1;
        */
}

void NzWindowImpl::SetPosition(int x, int y)
{
    uint32_t pos[] = { 
        static_cast<uint32_t>(x), 
        static_cast<uint32_t>(y) 
    };

    xcb_configure_window (
        m_connection,
        m_handle,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
        pos
    );
}

void NzWindowImpl::SetSize(unsigned int width, unsigned int height)
{
    uint32_t size[] = { width, height };
    xcb_configure_window (
        m_connection,
        m_handle,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        size
    );
}

void NzWindowImpl::SetStayOnTop(bool stayOnTop)
{
	//if (stayOnTop)
    // TODO
	//else
}

void NzWindowImpl::SetTitle(const NzString& title)
{
    // http://cgit.freedesktop.org/xcb/util/commit/?id=0e2f18cb5ea60319577ce39ad5c1c544c0790d5e
	const char* ctitle = NzString(title).GetBuffer();
    // We need to define both name and icon name TODO: check this.
    xcb_change_property (
        m_connection,
        XCB_PROP_MODE_REPLACE,
        m_handle,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(ctitle),
        ctitle
    );
    
    xcb_change_property (
        m_connection,
        XCB_PROP_MODE_REPLACE,
        m_handle,
        XCB_ATOM_WM_ICON_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(ctitle),
        ctitle
    );
	delete[] ctitle;
}

void NzWindowImpl::SetVisible(bool visible)
{
    visible ? xcb_map_window(m_connection, m_handle) : xcb_unmap_window(m_connection, m_handle) ;
}


bool NzWindowImpl::HandleEvent(xcb_generic_event_t* evt)
{
	// Inutile de récupérer des évènements ne venant pas de notre fenêtre
	//if (m_handle != evt->event)
		//return false;

	if (m_eventListener)
	{
        // http://xcb.freedesktop.org/tutorial/events/
		switch (evt->response_type & ~0x80)
		{

			case XCB_DESTROY_NOTIFY:
			{
				NzEvent event;
				event.type = nzEventType_Quit;
				m_parent->PushEvent(event);

				return true; // Afin que Windows ne ferme pas la fenêtre automatiquement
			}

            case XCB_ENTER_NOTIFY:
            {
                //auto e = static_cast<xcb_enter_notify_event_t*>(evt);
                m_mouseInside = true;
				NzEvent event;
				//event.type = nzEventType_MouseEnter;
				m_parent->PushEvent(event); 
                break;
// TODO
                break;
            }

            case XCB_LEAVE_NOTIFY:
            {
                // TODO
   				//NzEvent event;
				//event.type = nzEventType_LostFocus; // ??
				//m_parent->PushEvent(event);

                m_mouseInside = false;
				NzEvent event;
				event.type = nzEventType_MouseLeft;
				m_parent->PushEvent(event); 
                break;
            }

			case XCB_KEY_PRESS:
			{
				// http://msdn.microsoft.com/en-us/library/ms646267(VS.85).aspx
				//bool repeated = ((HIWORD(lParam) & KF_REPEAT) != 0); TODO : CHECK
				//if (m_keyRepeat || !repeated)
				{
                    auto e = (xcb_key_press_event_t*)evt;
					NzEvent event;
					event.type = nzEventType_KeyPressed;
					event.key.code = ConvertKeycode(e->detail);
/*					event.key.alt = ;
					event.key.control = 
					event.key.repeated = repeated;
					event.key.shift = 
					event.key.system = */
					m_parent->PushEvent(event);
				}

				break;
			}

            case XCB_KEY_RELEASE:
			{
                auto e = (xcb_key_release_event_t*)evt;
				NzEvent event;
				event.type = nzEventType_KeyReleased;
				event.key.code = ConvertKeycode(e->detail);
/*				event.key.alt = ; // TODO
				event.key.control =;
				event.key.shift = ;
				event.key.system = ;*/
				m_parent->PushEvent(event);

				break;
			}

			case XCB_BUTTON_PRESS:
			{
                auto e = (xcb_button_press_event_t*)evt;

                if (e->detail == 4 || e->detail == 5) // Mouse wheel event
                {
                    NzEvent event;
                    //event.type = nzEventType_MouseWheel;
                    //event.mouseWheel.delta = ; // TODO
                    //event.mouseWheel.x = e->event_x;
                    //event.mouseWheel.y = e->event_y;
                    m_parent->PushEvent(event);
                }
                else
                {
    				NzEvent event; // TODO
	    			event.type = nzEventType_MouseButtonPressed;
                    switch(e->detail) { // TODO
                        case 1:
                        {
    		    		    event.mouseButton.button = NzMouse::Left;
                            break;
                        }
                    }
			    	event.mouseButton.x = e->event_x; // Window relative position
				    event.mouseButton.y = e->event_y;
    				m_parent->PushEvent(event);
                }
				break;
			}

			case XCB_BUTTON_RELEASE:
			{
                auto e = (xcb_button_release_event_t*)evt;
				NzEvent event;
				event.type = nzEventType_MouseButtonReleased;
                switch(e->detail) {
                    /*case XCB_BUTTON_MASK_1:
          				event.mouseButton.button = NzMouse::Left;
                        break;
                    case XCB_BUTTON_MASK_2:
                        event.mouseButton.button = NzMouse::Middle;
                        break;
                    case XCB_BUTTON_MASK_3:
                        event.mouseButton.button = NzMouse::Right;
                        break;*/
                    default:
                        break;
                }
				event.mouseButton.x = e->event_x;
				event.mouseButton.y = e->event_y;
				m_parent->PushEvent(event);

				break;
			}

			case XCB_MOTION_NOTIFY: // TODO
			{
                auto e = (xcb_motion_notify_event_t*)evt;
				int currentX = e->event_x;
				int currentY = e->event_y;

				if (!m_mouseInside)
				{
					m_mouseInside = true;
					NzEvent event;

					event.type = nzEventType_MouseEntered;
					m_parent->PushEvent(event);

					event.type = nzEventType_MouseMoved;

					// Le delta sera 0
					event.mouseMove.deltaX = 0;
					event.mouseMove.deltaY = 0;

					event.mouseMove.x = currentX;
					event.mouseMove.y = currentY;

					m_mousePos.x = currentX;
					m_mousePos.y = currentY;

					m_parent->PushEvent(event);
					break;
				}
				else if (m_mousePos.x == currentX && m_mousePos.y == currentY)
					break;

				NzEvent event;
				event.type = nzEventType_MouseMoved;
				event.mouseMove.deltaX = currentX - m_mousePos.x;
				event.mouseMove.deltaY = currentY - m_mousePos.y;
				event.mouseMove.x = currentX;
				event.mouseMove.y = currentY;

				m_mousePos.x = currentX;
				m_mousePos.y = currentY;

				m_parent->PushEvent(event);
				break;
			}

            case XCB_CONFIGURE_REQUEST: // TODO
            {
                break;
            }

            case XCB_CONFIGURE_NOTIFY: // TODO
            {
                break;
            }

            case XCB_FOCUS_IN: // TODO
            {
                break;
            }

            case XCB_CLIENT_MESSAGE: // TODO
            {
                break;
            }

            case XCB_MAPPING_NOTIFY: // TODO
            {
                break;
            }

            case XCB_MAP_REQUEST: // TODO
            {
                break;
            }

            case XCB_PROPERTY_NOTIFY: // TODO
            {
                break;
            }

            case XCB_REPARENT_NOTIFY: // TODO
            {
                break;
            }

            case XCB_UNMAP_NOTIFY: // TODO
            {
                break;
            }

			default:
				break;
		}
	}

	return false;
}

bool NzWindowImpl::Initialize()
{
}

void NzWindowImpl::Uninitialize()
{
}

NzKeyboard::Key NzWindowImpl::ConvertKeycode(xcb_keycode_t key) const
{
	switch (key) // Take care, there is an offset of 8 
	{
		case KEY_RIGHTCTRL+8:	            return NzKeyboard::RControl;
        case KEY_LEFTCTRL+8:              return NzKeyboard::LControl;
		case KEY_LEFTALT+8:		        return NzKeyboard::LAlt;
        case KEY_RIGHTALT+8:              return NzKeyboard::RAlt;
        case KEY_LEFTSHIFT+8:             return NzKeyboard::LShift;
        case KEY_RIGHTSHIFT+8:            return NzKeyboard::RShift;
		case KEY_0+8:				   return NzKeyboard::Num0;
		case KEY_1+8:				   return NzKeyboard::Num1;
		case KEY_2+8:				   return NzKeyboard::Num2;
		case KEY_3+8:				   return NzKeyboard::Num3;
		case KEY_4+8:				   return NzKeyboard::Num4;
		case KEY_5+8:				   return NzKeyboard::Num5;
		case KEY_6+8:				   return NzKeyboard::Num6;
		case KEY_7+8:				   return NzKeyboard::Num7;
		case KEY_8+8:				   return NzKeyboard::Num8;
		case KEY_9+8:				   return NzKeyboard::Num9;
		case KEY_A+8:				   return NzKeyboard::A;
		case KEY_B+8:				   return NzKeyboard::B;
		case KEY_C+8:				   return NzKeyboard::C;
		case KEY_D+8:				   return NzKeyboard::D;
		case KEY_E+8:				   return NzKeyboard::E;
		case KEY_F+8:				   return NzKeyboard::F;
		case KEY_G+8:				   return NzKeyboard::G;
		case KEY_H+8:				   return NzKeyboard::H;
		case KEY_I+8:				   return NzKeyboard::I;
		case KEY_J+8:				   return NzKeyboard::J;
		case KEY_K+8:				   return NzKeyboard::K;
		case KEY_L+8:				   return NzKeyboard::L;
		case KEY_M+8:				   return NzKeyboard::M;
		case KEY_N+8:				   return NzKeyboard::N;
		case KEY_O+8:				   return NzKeyboard::O;
		case KEY_P+8:				   return NzKeyboard::P;
		case KEY_Q+8:				   return NzKeyboard::Q;
		case KEY_R+8:				   return NzKeyboard::R;
		case KEY_S+8:				   return NzKeyboard::S;
		case KEY_T+8:				   return NzKeyboard::T;
		case KEY_U+8:				   return NzKeyboard::U;
		case KEY_V+8:				   return NzKeyboard::V;
		case KEY_W+8:				   return NzKeyboard::W;
		case KEY_X+8:				   return NzKeyboard::X;
		case KEY_Y+8:				   return NzKeyboard::Y;
		case KEY_Z+8:				   return NzKeyboard::Z;
/*		case VK_ADD:			   return NzKeyboard::Add; // TODO
		case KEY_BACKSPACE:			   return NzKeyboard::Backspace;
		case VK_BROWSER_BACK:	   return NzKeyboard::Browser_Back;
		case VK_BROWSER_FAVORITES: return NzKeyboard::Browser_Favorites;
		case VK_BROWSER_FORWARD:   return NzKeyboard::Browser_Forward;
		case VK_BROWSER_HOME:	   return NzKeyboard::Browser_Home;
		case VK_BROWSER_REFRESH:   return NzKeyboard::Browser_Refresh;
		case VK_BROWSER_SEARCH:	   return NzKeyboard::Browser_Search;
		case VK_BROWSER_STOP:	   return NzKeyboard::Browser_Stop;
		case VK_CAPITAL:		   return NzKeyboard::CapsLock;
		case VK_CLEAR:			   return NzKeyboard::Clear;
		case VK_DELETE:			   return NzKeyboard::Delete;
		case KEY_:			   return NzKeyboard::Divide;*/
		case KEY_DOWN+8:			   return NzKeyboard::Down;
		case KEY_END+8:			   return NzKeyboard::End;
		case KEY_ESC+8:			   return NzKeyboard::Escape;
		case KEY_F1+8:				   return NzKeyboard::F1;
		case KEY_F2+8:				   return NzKeyboard::F2;
		case KEY_F3+8:				   return NzKeyboard::F3;
		case KEY_F4+8:				   return NzKeyboard::F4;
		case KEY_F5+8:				   return NzKeyboard::F5;
		case KEY_F6+8:				   return NzKeyboard::F6;
		case KEY_F7+8:				   return NzKeyboard::F7;
		case KEY_F8+8:				   return NzKeyboard::F8;
		case KEY_F9+8:				   return NzKeyboard::F9;
		case KEY_F10+8:			   return NzKeyboard::F10;
		case KEY_F11+8:			   return NzKeyboard::F11;
		case KEY_F12+8:			   return NzKeyboard::F12;
		case KEY_F13+8:			   return NzKeyboard::F13;
		case KEY_F14+8:			   return NzKeyboard::F14;
		case KEY_F15+8:			   return NzKeyboard::F15;
		case KEY_HOME+8:			   return NzKeyboard::Home;
		case KEY_INSERT+8:			   return NzKeyboard::Insert;
		case KEY_LEFT+8:			   return NzKeyboard::Left;
		case KEY_LEFTMETA+8:			   return NzKeyboard::LSystem;
    	case KEY_NEXTSONG+8:  return NzKeyboard::Media_Next;
		case KEY_PAUSECD+8:  return NzKeyboard::Media_Play;
		case KEY_PREVIOUSSONG+8:  return NzKeyboard::Media_Previous;
		case KEY_STOPCD+8:		   return NzKeyboard::Media_Stop;
//		case VK_MULTIPLY:		   return NzKeyboard::Multiply; // TODO
		case KEY_PAGEDOWN+8:			   return NzKeyboard::PageDown;
		case KEY_NUMERIC_0:		   return NzKeyboard::Numpad0;
		case KEY_NUMERIC_1:		   return NzKeyboard::Numpad1;
		case KEY_NUMERIC_2:		   return NzKeyboard::Numpad2;
		case KEY_NUMERIC_3:		   return NzKeyboard::Numpad3;
		case KEY_NUMERIC_4:		   return NzKeyboard::Numpad4;
		case KEY_NUMERIC_5:		   return NzKeyboard::Numpad5;
		case KEY_NUMERIC_6:		   return NzKeyboard::Numpad6;
		case KEY_NUMERIC_7:		   return NzKeyboard::Numpad7;
		case KEY_NUMERIC_8:		   return NzKeyboard::Numpad8;
		case KEY_NUMERIC_9:		   return NzKeyboard::Numpad9;
        case KEY_NUMLOCK+8:		   return NzKeyboard::NumLock;
		case KEY_SEMICOLON+8:			   return NzKeyboard::Semicolon;
		case KEY_SLASH+8:			   return NzKeyboard::Slash;
//		case KEY_TILDE+8:			   return NzKeyboard::Tilde;
//		case KEY_:			   return NzKeyboard::LBracket; //TODO
		case KEY_BACKSLASH+8:			   return NzKeyboard::Backslash;
//		case VK_OEM_6:			   return NzKeyboard::RBracket;
//		case VK_OEM_7:			   return NzKeyboard::Quote;
		case KEY_COMMA+8:		   return NzKeyboard::Comma;
//		case VK_OEM_MINUS:		   return NzKeyboard::Dash;
//		case VK_OEM_PERIOD:		   return NzKeyboard::Period;
//		case VK_OEM_PLUS:		   return NzKeyboard::Equal;
		case KEY_RIGHT+8:			   return NzKeyboard::Right;
		case KEY_PAGEUP+8:			   return NzKeyboard::PageUp;
		case KEY_PAUSE+8:			   return NzKeyboard::Pause;
		case KEY_PRINT+8:			   return NzKeyboard::Print;
//		case VK_SCROLL:			   return NzKeyboard::ScrollLock;
//		case VK_SNAPSHOT:		   return NzKeyboard::PrintScreen;
//		case VK_SUBTRACT:		   return NzKeyboard::Subtract;// TODO
		case KEY_ENTER+8:			   return NzKeyboard::Return;
		case KEY_RIGHTMETA+8:			   return NzKeyboard::RSystem;
		case KEY_SPACE+8:			   return NzKeyboard::Space;
		case KEY_TAB+8:	                return NzKeyboard::Tab;
		case KEY_UP+8: 	    		    return NzKeyboard::Up;
//		case KEY_VOLUME_DOWN+8:	        return NzKeyboard::Volume_Down;
		case KEY_MUTE+8:                return NzKeyboard::Volume_Mute;
//		case KEY_VOLUME_UP+8:           return NzKeyboard::Volume_Up;

		default:
			return NzKeyboard::Key(NzKeyboard::Undefined);
	}
}

