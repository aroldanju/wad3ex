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

#ifndef WADEX_IMAGE_H
#define WADEX_IMAGE_H

#include <SDL2/SDL.h>
#include <string>
#include <iostream>

class Image {
public:
    Image(unsigned int width, unsigned int height);
    virtual ~Image();

    bool isReady() const { return m_surface != NULL; }

    void putPixel(unsigned int index, unsigned char red, unsigned char green, unsigned char blue);

    bool saveToBmp(const std::string& filename);

private:
    SDL_Surface *m_surface;
};

#endif //WADEX_IMAGE_H
