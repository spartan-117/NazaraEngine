// Copyright (C) 2013 Jérôme Leclercq
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/VideoModeImpl.hpp>
#include <Nazara/Utility/VideoMode.hpp>
#include <Nazara/Utility/Linux/XDisplay.hpp>
#include <algorithm>
#include <Nazara/Utility/Debug.hpp>
#include <xcb/xcb_util.h>

NzVideoMode NzVideoModeImpl::GetDesktopMode()
{
    int screenNb;
    auto connection = nzOpenXCBConnection(&screenNb);
    auto screen = xcb_aux_get_screen(connection, screenNb);

	return NzVideoMode(
            screen->width_in_pixels, 
            screen->height_in_pixels, 
            static_cast<nzUInt8>(xcb_aux_get_depth(connection, screen))
    );
}

void NzVideoModeImpl::GetFullscreenModes(std::vector<NzVideoMode>& modes)
{
    // TODO: use XRandR XCB extension ? 
    // Ref: awesome-wm source code
}
