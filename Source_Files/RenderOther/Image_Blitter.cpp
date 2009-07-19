/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
	Image blitter
	written by Jeremiah Morris, 2009
*/

#include "Image_Blitter.h"
#include "Images.h"

Image_Blitter::Image_Blitter() : m_surface(NULL), m_scaled_surface(NULL), tint_color_r(1.0), tint_color_g(1.0), tint_color_b(1.0), tint_color_a(1.0), rotation(0.0)
{
	m_src.x = m_src.y = m_src.w = m_src.h = 0;
	m_scaled_src.x = m_scaled_src.y = m_scaled_src.w = m_scaled_src.h = 0;
	crop_rect.x = crop_rect.y = crop_rect.w = crop_rect.h = 0;
}

bool Image_Blitter::Load(const ImageDescriptor& image)
{
#ifdef ALEPHONE_LITTLE_ENDIAN
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(const_cast<uint32 *>(image.GetBuffer()), image.GetWidth(), image.GetHeight(), 32, image.GetWidth() * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(const_cast<uint32 *>(image.GetBuffer()), image.GetWidth(), image.GetHeight(), 32, image.GetWidth() * 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
	if (!s)
		return false;
	bool ret = Load(*s);
	SDL_FreeSurface(s);
	return ret;
}

bool Image_Blitter::Load(const SDL_Surface& s)
{
	SDL_Rect sr = { 0, 0, s.w, s.h };
	return Load(s, sr);
}

bool Image_Blitter::Load(const SDL_Surface& s, const SDL_Rect& src)
{
	Unload();
	m_src.x = 0;
	m_src.y = 0;
	m_src.w = src.w;
	m_src.h = src.h;
	m_scaled_src.x = 0;
	m_scaled_src.y = 0;
	m_scaled_src.w = m_src.w;
	m_scaled_src.h = m_src.h;
	crop_rect.x = 0;
	crop_rect.y = 0;
	crop_rect.w = m_src.w;
	crop_rect.h = m_src.h;
	
#ifdef ALEPHONE_LITTLE_ENDIAN
	m_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, m_src.w, m_src.h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#else
	m_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, m_src.w, m_src.h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
	if (!m_surface)
		return false;
	SDL_SetAlpha(m_surface, SDL_SRCALPHA, 0);
	
	// when blitting surface, make sure we copy rather than blend the alpha
	uint8 src_alpha = s.format->alpha;
	uint32 src_flags = s.flags;
	if (src_flags & SDL_SRCALPHA)
		SDL_SetAlpha(const_cast<SDL_Surface *>(&s), src_flags & ~SDL_SRCALPHA, 0);
	
	SDL_Rect sr = src;
	int ret = SDL_BlitSurface(const_cast<SDL_Surface *>(&s), &sr, m_surface, NULL);
	
	if (src_flags & SDL_SRCALPHA)
		SDL_SetAlpha(const_cast<SDL_Surface *>(&s), src_flags, src_alpha);
	
	return (ret == 0);
}

void Image_Blitter::Unload()
{
	SDL_FreeSurface(m_surface);
	m_surface = NULL;
	SDL_FreeSurface(m_scaled_surface);
	m_scaled_surface = NULL;
	m_src.w = 0;
	m_scaled_src.w = 0;
	m_src.h = 0;
	m_scaled_src.h = 0;
}

bool Image_Blitter::Loaded()
{
	return (m_surface != NULL);
}

void Image_Blitter::Rescale(int width, int height)
{	
	if (width != m_scaled_src.w)
	{
		crop_rect.x = crop_rect.x * width / m_scaled_src.w;
		crop_rect.w = crop_rect.w * width / m_scaled_src.w;
		m_scaled_src.w = width;
	}
	if (height != m_scaled_src.h)
	{
		crop_rect.y = crop_rect.y * height / m_scaled_src.h;
		crop_rect.h = crop_rect.h * height / m_scaled_src.h;
		m_scaled_src.h = height;
	}
}
	
int Image_Blitter::Width()
{
	return m_scaled_src.w;
}

int Image_Blitter::Height()
{
	return m_scaled_src.h;
}

int Image_Blitter::UnscaledWidth()
{
	return m_src.w;
}

int Image_Blitter::UnscaledHeight()
{
	return m_src.h;
}

void Image_Blitter::Draw(SDL_Surface *dst_surface, SDL_Rect& dst, SDL_Rect& src)
{
	if (!Loaded())
		return;
	if (!dst_surface)
		return;
	
	SDL_Surface *src_surface = m_surface;
	
	// rescale surface if necessary
	if (m_scaled_src.w != m_src.w || m_scaled_src.h != m_src.h)
	{
		if (!m_scaled_surface ||
				m_scaled_surface->w != m_scaled_src.w ||
				m_scaled_surface->h != m_scaled_src.h)
		{
			SDL_FreeSurface(m_scaled_surface);
			m_scaled_surface = rescale_surface(m_surface, m_scaled_src.w, m_scaled_src.h);
		}
		src_surface = m_scaled_surface;
	}
	
	if (!src_surface)
		return;
  
	SDL_BlitSurface(src_surface, &src, dst_surface, &dst);
}

Image_Blitter::~Image_Blitter()
{
	Unload();
}
