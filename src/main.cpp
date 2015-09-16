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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "Image.h"

//#define __WADEXDEBUG__

using namespace std;

#define WADEX_TOOTEXTURES 1000

enum Texture_t {
    TEXTURE_REGULAR = 0x43,
    TEXTURE_FONT = 0x46,
    TEXTURE_DECAL = 0x40,
    TEXTURE_CACHED = 0x42
};

#define WADEX_HEADERSIZE    12
typedef struct {
    char sz_magic[4];
    unsigned int textures;
    unsigned int lumps_offset;
} wad_header;

#define WADEX_LUMPSIZE     32
typedef struct {
    unsigned int texture_offset;
    unsigned int size;
    unsigned int length;
    char type;
    char compression;
    char padding[2];
    char name[16];
} wad_lump;

typedef struct {
    char sz_name[16];               // nombre
    unsigned int width, height;     // ancho y alto
    unsigned int offset_image;      // Desplazamiento para la imagen
    unsigned int offset_mipmap_1;   // Desplazamiento para mipmap x2
    unsigned int offset_mipmap_2;   // Desplazamiento para mipmap x4
    unsigned int offset_mipmap_3;   // Desplazamiento para mipmap x8
    unsigned char *data;            // width * height
    unsigned char *data_mipmap_1;   // width/2 * height/2
    unsigned char *data_mipmap_2;   // width/4 * height/4
    unsigned char *data_mipmap_3;   // width/8 * height/8
    short colors;                   // colores
    unsigned char palette[256][3];  // paleta
    char padding[2];                // padding?

} wad_texture;

Image *createImage(int width, int height, unsigned char palette[][3], unsigned char* data)   {
    Image *image = new Image(width, height);
    if ( image->isReady() ) {
        for (unsigned int j = 0; j < width * height; j++) {
            image->putPixel(j,
                            palette[data[j]][0],
                            palette[data[j]][1],
                            palette[data[j]][2]
            );
        }
    }
    else {
        delete image;
        return NULL;
    }

    return image;
}

void showLicense() {
    std::cout << "wad3ex Copyright (C) 2015 A. Roldan\n"
         "This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
         "This is free software, and you are welcome to redistribute it\n"
         "under certain conditions; type `show c' for details." << std::endl << std::endl;
}

int main(int argc, char** argv) {

    std::string textureName, texturesPath, wadPath, startPattern, mipmapsPath;
    wad_header wadHeader;
    wad_texture wadTexture;
    wad_lump wadLump;
    int textures, start, end;
    bool generateMipmaps, verbose;
    char selection;

    // valores por defecto
    generateMipmaps = false;
    texturesPath = "./";
    start = 0;
    end = -1;
    startPattern = "";
    verbose = false;
    textures = 0;
    mipmapsPath = "./";

    showLicense();

    if ( argc < 2 ) {
        std::cout << "usage: " << argv[0] << " wadfile [options]" << std::endl;
        std::cout << "Options" << std::endl;
        std::cout << "\t -s, --start <N> .......... Extract from texture #N" << std::endl;
        std::cout << "\t -e, --end <N> ............ Extract to texture #N" << std::endl;
        std::cout << "\t -o, --output <path> ...... Textures output path to <path>" << std::endl;
        std::cout << "\t -v, --verbose ............ Prints to console the status" << std::endl;
        std::cout << "\t -p, --pattern <pattern> .. Texture pattern in name" << std::endl;
        std::cout << "\t -m, --mipmaps <path> ..... Extract the texture mipmaps to <path>" << std::endl;
        return 1;
    }

    if ( argc == 2 ) {
        wadPath = argv[1];
    }
    else {
        wadPath = argv[1];

        for ( int i = 2; i < argc; i++ ) {
            std::string option = argv[i];
            if ( option == "--start" || option == "-s" ) {
                if ( argc > i + 1 ) {
                    start = atoi(argv[i + 1]);
                    i++;
                }
                else {
                    std::cout << " Error: Option --start require a parameter" << std::endl;
                    return 1;
                }
            }
            else if ( option == "--end" || option == "-e" ) {
                if ( argc > i + 1 ) {
                    end = atoi(argv[i + 1]);
                    i++;
                }
                else {
                    std::cout << " Error: Option --end require a parameter" << std::endl;
                    return 1;
                }
            }
            else if ( option == "--output" || option == "-o" ) {
                if ( argc > i + 1 ) {
                    texturesPath = argv[i + 1];
                    i++;
                }
                else {
                    std::cout << " Error: Option --output require a parameter" << std::endl;
                    return 1;
                }
            }
            else if ( option == "--pattern" || option == "-p" ) {
                if ( argc > i + 1 ) {
                    startPattern = argv[i + 1];
                    i++;
                }
                else {
                    std::cout << " Error: Option --pattern require a parameter" << std::endl;
                    return 1;
                }
            }
            else if ( option == "--verbose" || option == "-v" ) {
                verbose = true;
            }
            else if ( option == "--mipmaps" || option == "-m" ) {
                if ( argc > i + 1 ) {
                    mipmapsPath = argv[i + 1];
                    generateMipmaps = true;
                    i++;
                }
                else {
                    std::cout << " Error: Option --mipmaps require a parameter" << std::endl;
                    return 1;
                }
            }
        }
    }

    std::ifstream tempFile(wadPath.c_str(), std::ios_base::in | std::ios_base::binary);
    if ( !tempFile.is_open() ) {
        std::cout << " Error: could not open '" << wadPath << "' WAD file" << std::endl << std::endl;
        return 1;
    }
    tempFile.close();

    if ( end != -1 && start > end ) {
        std::swap(start, end);
    }

#ifdef __WADEXDEBUG__
    std::cout << " use range = " << useRange << std::endl;
    std::cout << " range = " << range << std::endl;
    std::cout << " start = " << start << std::endl;
    std::cout << " end = " << end << std::endl;
    std::cout << " wadfile = " << wadPath << std::endl;
    std::cout << " mipmaps = " << generateMipmaps << std::endl;
    std::cout << " output = " << texturesPath << std::endl;
    std::cout << " pattern = " << startPattern << std::endl;
#endif  // __WADEXDEBUG__

    std::ifstream wadFile(wadPath.c_str(), std::ios_base::in | std::ios_base::binary);

    if ( wadFile.is_open() ) {
        wadFile.read((char*)&wadHeader.sz_magic, 4);

        // WAD3?
        if ( wadHeader.sz_magic[0] != 'W' || wadHeader.sz_magic[1] != 'A' || wadHeader.sz_magic[2] != 'D' || wadHeader.sz_magic[3] != '3' ) {
            std::cout << " Error: WAD file has an incorrect format" << std::endl;
            wadFile.close();
            exit(1);
        }

        wadFile.read((char*)&wadHeader.textures, 4);
        wadFile.read((char*)&wadHeader.lumps_offset, 4);

        // limite de texturas
        if ( wadHeader.textures < (end - start) ) {
            end = wadHeader.textures;
        }

        if ( verbose ) {
            std::cout << " Info: " << wadHeader.textures << " textures in package" << std::endl << std::endl;
        }

        if ( end == -1 ) { end = wadHeader.textures; }

        // Si son demasiadas
        if ( (end - start) >= WADEX_TOOTEXTURES ) {
            std::cout << " There are " << (end - start) << " textures, it could take several minutes. Do you want continue? [Y/n]: ";
            std::cin >> selection;
            if ( selection != 'y' && selection != 'Y' ) {
                std::cout << " Info: Aborted by user" << std::endl;
                wadFile.close();
                exit(1);
            }
        }

        for ( int i = start; i < end; i++ ) {

            // pasa al offset de la lista de lumps
            wadFile.seekg(wadHeader.lumps_offset + (i * WADEX_LUMPSIZE));

            wadFile.read((char *) &wadLump, WADEX_LUMPSIZE);

#ifdef __WADEXDEBUG__
            std::cout << "[LUMP]" << std::endl;
            std::cout << "\ttexture_offset = " << wadLump.texture_offset << std::endl;
            std::cout << "\tsize = " << wadLump.size << std::endl;
            std::cout << "\tlength = " << wadLump.length << std::endl;
            std::cout << "\ttype = 0x" << std::hex << (int) wadLump.type << std::dec << std::endl;
            std::cout << "\tcompression = " << wadLump.compression << std::endl;
            std::cout << "\tpadding = " << wadLump.padding << std::endl;
            std::cout << "\tname = " << wadLump.name << std::endl;
#endif  // __WADEXDEBUG__

            if ( wadLump.type == TEXTURE_FONT ) {
                std::cout << " Warning: skipping font texture" << std::endl;
                continue;
            }

            // lleva el cursor de fichero al offset de la textura según el lump
            wadFile.seekg(wadLump.texture_offset);

            if (wadLump.type == TEXTURE_REGULAR || wadLump.type == TEXTURE_DECAL) {
                wadFile.read((char *) &wadTexture.sz_name, 16);
            }

            // comprueba el patrón de inicio
            if ( startPattern != "" ) {
                std::string name = wadTexture.sz_name;
                if ( startPattern.length() < name.length() ) {
                    name = name.substr(0, startPattern.length());
                }
                if (name != startPattern) {
                    continue;
                }
            }

            // lee el ancho y el alto
            wadFile.read((char *) &wadTexture.width, 4);
            wadFile.read((char *) &wadTexture.height, 4);

            // datos de textura regular
            if (wadLump.type == TEXTURE_REGULAR || wadLump.type == TEXTURE_DECAL) {
                wadFile.read((char *) &wadTexture.offset_image, 4);
                wadFile.read((char *) &wadTexture.offset_mipmap_1, 4);
                wadFile.read((char *) &wadTexture.offset_mipmap_2, 4);
                wadFile.read((char *) &wadTexture.offset_mipmap_3, 4);
            }

            // Datos de color
            wadTexture.data = new unsigned char[wadTexture.width * wadTexture.height];
            wadFile.read((char *) wadTexture.data, wadTexture.width * wadTexture.height);

            // mipmaps
            if (wadLump.type == TEXTURE_REGULAR || wadLump.type == TEXTURE_DECAL) {
                wadTexture.data_mipmap_1 = new unsigned char[(wadTexture.width / 2) * (wadTexture.height / 2)];
                wadTexture.data_mipmap_2 = new unsigned char[(wadTexture.width / 4) * (wadTexture.height / 4)];
                wadTexture.data_mipmap_3 = new unsigned char[(wadTexture.width / 8) * (wadTexture.height / 8)];
                wadFile.read((char *) wadTexture.data_mipmap_1, (wadTexture.width / 2) * (wadTexture.height / 2));
                wadFile.read((char *) wadTexture.data_mipmap_2, (wadTexture.width / 4) * (wadTexture.height / 4));
                wadFile.read((char *) wadTexture.data_mipmap_3, (wadTexture.width / 8) * (wadTexture.height / 8));
            }

            // paleta
            wadFile.read((char *) &wadTexture.colors, 2);
            if (wadLump.type == TEXTURE_REGULAR || wadLump.type == TEXTURE_CACHED) {
                for (unsigned int j = 0; j < wadTexture.colors; j++) {
                    wadFile.read((char *) &wadTexture.palette[j][0], 1);
                    wadFile.read((char *) &wadTexture.palette[j][1], 1);
                    wadFile.read((char *) &wadTexture.palette[j][2], 1);
                }
            }

            // padding
            wadFile.read(wadTexture.padding, 2);

            Image *image;

            // mipmaps
            if ( generateMipmaps && (wadLump.type == TEXTURE_REGULAR || wadLump.type == TEXTURE_DECAL) ) {
                textureName = mipmapsPath;
                textureName += wadTexture.sz_name;
                textureName += ".x2.bmp";
                image = createImage((wadTexture.width / 2), (wadTexture.height / 2), wadTexture.palette, wadTexture.data_mipmap_1);
                if (image) {
                    image->saveToBmp(textureName);
                    delete image;
                }
                else {
                    if ( verbose ) {
                        std::cout << "[Fail] Texture mipmap x2 '" << mipmapsPath << wadTexture.sz_name << "'" << std::endl;
                    }
                }

                textureName = mipmapsPath;
                textureName += wadTexture.sz_name;
                textureName += ".x4.bmp";
                image = createImage((wadTexture.width / 4), (wadTexture.height / 4), wadTexture.palette, wadTexture.data_mipmap_2);
                if (image) {
                    image->saveToBmp(textureName);
                    delete image;
                }
                else {
                    if ( verbose ) {
                        std::cout << "[Fail] Texture mipmap x4 '" << mipmapsPath << wadTexture.sz_name << "'" << std::endl;
                    }
                }

                textureName = mipmapsPath;
                textureName += wadTexture.sz_name;
                textureName += ".x8.bmp";
                image = createImage((wadTexture.width / 8), (wadTexture.height / 8), wadTexture.palette, wadTexture.data_mipmap_3);
                if (image) {
                    image->saveToBmp(textureName);
                    delete image;
                }
                else {
                    if ( verbose ) {
                        std::cout << "[Fail] Texture mipmap x8 '" << mipmapsPath << wadTexture.sz_name << "'" << std::endl;
                    }
                }
            }

            textureName = texturesPath;
            textureName += wadTexture.sz_name;
            textureName += ".bmp";
            image = createImage(wadTexture.width, wadTexture.height, wadTexture.palette, wadTexture.data);
            if ( image ) {
                image->saveToBmp(textureName);
                delete image;
                if ( verbose ) {
                    std::cout << "[Success] Texture '" << wadTexture.sz_name << "' " << wadTexture.width << "x" << wadTexture.height << std::endl;
                }

                textures++;
            }
            else {
                if ( verbose ) {
                    std::cout << "[Fail] Texture '" << wadTexture.sz_name << "'" << std::endl;
                }
            }

#ifdef __WADEXDEBUG__
            std::cout << "[TEXTURE]" << std::endl;
            std::cout << "\tName = " << wadTexture.sz_name << std::endl;
            std::cout << "\tWidth = " << wadTexture.width << std::endl;
            std::cout << "\tHeight = " << wadTexture.height << std::endl;
            std::cout << "\tImage offset = " << wadTexture.offset_image << std::endl;
            std::cout << "\tMipmap x2 offset = " << wadTexture.offset_mipmap_1 << std::endl;
            std::cout << "\tMipmap x4 offset = " << wadTexture.offset_mipmap_2 << std::endl;
            std::cout << "\tMipmap x8 offset = " << wadTexture.offset_mipmap_3 << std::endl;
#endif  // __WADEXDEBUG__

            delete wadTexture.data;

            if ( wadLump.type == TEXTURE_REGULAR || wadLump.type == TEXTURE_DECAL ) {
                delete wadTexture.data_mipmap_1;
                delete wadTexture.data_mipmap_2;
                delete wadTexture.data_mipmap_3;
            }
        }

        wadFile.close();
    }

    if ( verbose ) {
        std::cout << std::endl << " " << textures << " textures created" << std::endl << std::endl;
    }

    return 0;
}