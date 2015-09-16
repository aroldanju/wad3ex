/*
 *  wad3ex - WAD3 texture extractor
 *  Copyright (C) 2015  A. Roldan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Image.h"

Image::Image(unsigned int width, unsigned int height) {
    m_surface = NULL;

    m_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
    if (!m_surface) {
        std::cout << "[ Error - Image::Image ]: No se pudo crear la superficie!" << std::endl;
        return;
    }
}


Image::~Image() {
    if ( m_surface ) {
        delete m_surface;
    }
}

void Image::putPixel(unsigned int index, unsigned char red, unsigned char green, unsigned char blue)   {
    Uint32* pixel = (Uint32*)m_surface->pixels;
    Uint32 color = SDL_MapRGB(m_surface->format, red, green, blue);
    pixel += index;
    *pixel = color;
}

bool Image::saveToBmp(const std::string& filename){
    if ( !m_surface ) {
        return false;
    }

    SDL_SaveBMP(m_surface, filename.c_str());

    return true;
}