// Copyright (C) 2012 Alexandre Janniaux
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

// Interface inspirée de la SFML par Laurent Gomila

#pragma once

#ifndef NAZARA_XDISPLAY_HPP
#define NAZARA_XDISPLAY_HPP

#include <Nazara/Core/NonCopyable.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Utility/Config.hpp>
#include <Nazara/Utility/Keyboard.hpp>
#include <Nazara/Utility/Mouse.hpp>
#include <Nazara/Utility/VideoMode.hpp>
#include <Nazara/Utility/Window.hpp>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

#if NAZARA_UTILITY_THREADED_WINDOW
class NzConditionVariable;
class NzMutex;
class NzThread;
#endif

Display* nzOpenDisplay(int* screenNb=nullptr);
xcb_connection_t* nzOpenXCBConnection(int* screenNb=nullptr);

void nzCloseDisplay(Display* display);
void nzCloseXCBConnection(xcb_connection_t* connection);

#endif // NAZARA_XDISPLAY_HPP
