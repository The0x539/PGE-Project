/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2017 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OBJ_BLOCK_H
#define OBJ_BLOCK_H

#include "../graphics/graphics.h"

#include <string>
#include <ConfigPackManager/level/config_block.h>

struct obj_block
{
    /*   OpenGL    */
    bool isInit;
    PGE_Texture *image;
    GLuint textureID;
    int32_t textureArrayId;
    int32_t animator_ID;
    /*   OpenGL    */

    BlockSetup setup;
};


#endif // OBJ_BLOCK_H
