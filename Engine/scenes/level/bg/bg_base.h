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

#ifndef BG_BASE_H
#define BG_BASE_H

#include <data_configs/obj_bg.h>
#include <common_features/rectf.h>

class LevelScene;

/*!
 * \brief Generic level background processor
 */
class LevelBackgroundBase
{
public:
    /*!
     * \brief Captain Obvious sayd: It's descructor!
     */
    virtual ~LevelBackgroundBase();

    /*!
     * \brief Initialize background with given setup
     * \param bg Background config
     */
    virtual void init(const obj_BG &bg) = 0;

    /*!
     * \brief Give the level scene pointer to the background processor (to allow in-scene rendering)
     * \param scene Pointer to level scene
     */
    virtual void setScene(LevelScene *scene) = 0;

    /*!
     * \brief Process internal animation (scrolling, etc.) if available
     * \param frameDelay (Delay of one frame in milliseconds)
     */
    virtual void process(double frameDelay) = 0;

    /*!
     * \brief Draw background on the screen
     * \param box Current section box
     * \param x X position on the screen
     * \param y Y position on the screen
     * \param w Width of screen
     * \param h Heigh of screen
     */
    virtual void renderBackground(const PGE_RectF &box, double x, double y, double w, double h) = 0;

    /*!
     * \brief Draw in-scene background on the screen
     * \param box Current section box
     * \param x X position on the screen
     * \param y Y position on the screen
     * \param w Width of screen
     * \param h Heigh of screen
     *
     * Instead of directly draw, this function puts prepared to draw image pieces to the special queue.
     * You must call it once per PHYSICAL step and don't call it in render() function or you have a risk
     * to overflow memory while game pause is turned on!
     */
    virtual void renderInScene(const PGE_RectF &box, double x, double y, double w, double h) = 0;

    /*!
     * \brief Draw foreground on the screen
     * \param box Current section box
     * \param x X position on the screen
     * \param y Y position on the screen
     * \param w Width of screen
     * \param h Heigh of screen
     */
    virtual void renderForeground(const PGE_RectF &box, double x, double y, double w, double h) = 0;
};

#endif // BG_BASE_H
