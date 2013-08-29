// Copyright (C) 2013 Alexandre Janniaux
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/Linux/CursorImpl.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Nazara/Utility/Debug.hpp>

bool NzCursorImpl::Create(const NzImage& cursor, int hotSpotX, int hotSpotY)
{
    NazaraError("[POSIX] NzCursorImpl::Create is not implemented");
    return false;

    NzImage windowsCursor(cursor);
    if (!windowsCursor.Convert(nzPixelFormat_BGRA8))
    {
        NazaraError("Failed to convert cursor to BGRA8");
        return false;
    } 
}

void NzCursorImpl::Destroy()
{
    NazaraError("[POSIX] NzCursorImpl::Destroy is not implemented");
}

xcb_cursor_t NzCursorImpl::GetCursor()
{
    return m_cursor;
}


