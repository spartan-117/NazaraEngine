// Copyright (C) 2013 Alexandre Janniaux
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/Linux/IconImpl.hpp>
#include <Nazara/Utility/Image.hpp>
#include <xcb/xcb.h>
#include <Nazara/Utility/Debug.hpp>

bool NzIconImpl::Create(const NzImage& icon)
{
	NzImage windowsIcon(icon); // Vive le COW
	if (!windowsIcon.Convert(nzPixelFormat_BGRA8))
	{
		NazaraError("Failed to convert icon to BGRA8");
		return false;
	}

    if(m_icon)
    {
        Destroy();
        m_icon = nullptr;
    }

    /*
     * xcb_image_t* xcb_image_create ( uint16_t  width,
     * uint                 16_t  height,
     * xcb_image_format_t  format,
     * uint8_t  xpad,
     * uint8                                    _t  depth,
     * uint8_t  bpp,
     * uint8_t  unit,
     * xcb_image_order_t  by                                        te_order,
     * xcb_image_order_t  bit_order,
     * void *  base,
     * uint32_t                                  bytes,
     * uint8_t *  data 
     * )                            
     */
    // http://xcb.freedesktop.org/XcbUtil/api/group__xcb____image__t.html#g1db19b77fbeb38552d41bb3c623d5343
    // http://opensource.apple.com/source/X11libs/X11libs-40/xcb-util/xcb-util-0.3.3/image/xcb_image.h
    // http://opensource.apple.com/source/X11libs/X11libs-40/xcb-util/xcb-util-0.3.3/image/test_swap.c
    /*m_icon = xcb_image_create(
                windowsIcon.GetWidth(),     // width
                windowsIcon.GetHeight(),    // height
                XCB_IMAGE_FORMAT_XY_BITMAP, // format
                32,                         // xpad (scanline pad of the image   
                32,                         // depth
                32,                         // bpp
                32,                         // unit in bits
                XCB_IMAGE_ORDER_LSB_FIRST,  // byte_order
                XCB_IMAGE_ORDER_LSB_FIRST,  // bit_order
                NULL,                       // base address of malloced image data
                0,                          // bytes (size in bhytes of the storage pointed to by base
                windowsIcon.GetConstPixels()// data
    );*/

	if (!m_icon)
	{
		NazaraError("Failed to create icon: " + NzGetLastSystemError());
		return false;
	}

	return true;
}

void NzIconImpl::Destroy()
{
	//xcb_image_destroy(m_icon);
}

xcb_image_t* NzIconImpl::GetIcon()
{
	return m_icon;
}
