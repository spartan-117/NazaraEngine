// Copyright (C) 2013 Alexandre Janniaux
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_ICONIMPL_HPP
#define NAZARA_ICONIMPL_HPP

class xcb_image_t;
class NzImage;

class NzIconImpl
{
	public:
		bool Create(const NzImage& image);
		void Destroy();

		xcb_image_t* GetIcon();

	private:
		xcb_image_t* m_icon = nullptr;
};

#endif // NAZARA_ICONIMPL_HPP
