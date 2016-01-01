/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2015 Vitaly Novichkov <admin@wohlnet.ru>
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

/*
#define GLEW_STATIC
#define GLEW_NO_GLU
#include <GL/glew.h>
*/

#include "gl_renderer.h"
#include "window.h"
#include "../common_features/app_path.h"

#include <common_features/graphics_funcs.h>
#include <common_features/logger.h>
#include <gui/pge_msgbox.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h> // SDL 2 Library
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_thread.h>
//#ifdef __APPLE__
//    //#include <OpenGL/glu.h>
//#else
//    #ifndef ANDROID
//    //#include <GL/glu.h>
//    #endif
//#endif

#include "gl_debug.h"

#ifdef _WIN32
#define FREEIMAGE_LIB
#endif
#include <FreeImageLite.h>

#include <audio/pge_audio.h>

#include <QDir>
#include <QImage>
#include <QDateTime>
#include <QMessageBox>
#include <QtDebug>

bool GlRenderer::_isReady=false;
SDL_Thread *GlRenderer::thread = NULL;

int GlRenderer::window_w=800;
int GlRenderer::window_h=600;
float GlRenderer::scale_x=1.0f;
float GlRenderer::scale_y=1.0f;
float GlRenderer::offset_x=0.0f;
float GlRenderer::offset_y=0.0f;
float GlRenderer::viewport_x=0;
float GlRenderer::viewport_y=0;
float GlRenderer::viewport_scale_x=1.0f;
float GlRenderer::viewport_scale_y=1.0f;
float GlRenderer::viewport_w=800;
float GlRenderer::viewport_h=600;
float GlRenderer::viewport_w_half=400;
float GlRenderer::viewport_h_half=300;

float GlRenderer::color_level_red=1.0;
float GlRenderer::color_level_green=1.0;
float GlRenderer::color_level_blue=1.0;
float GlRenderer::color_level_alpha=1.0;

float GlRenderer::color_binded_texture[16] = { 1.0f, 1.0f, 1.0f, 1.0f,
                                               1.0f, 1.0f, 1.0f, 1.0f,
                                               1.0f, 1.0f, 1.0f, 1.0f,
                                               1.0f, 1.0f, 1.0f, 1.0f };

PGE_Texture GlRenderer::_dummyTexture;

bool GlRenderer::init()
{
    if(!PGE_Window::isReady())
        return false;

    /*
    glewExperimental = GL_TRUE; // Needed for a Core-mode OpenGL
    if( glewInit() != GLEW_OK )
    {
        GLERROR("Impossible to initialize GLEW");
        return false;
    }
    GLERRORCHECK();*/

    glViewport( 0.f, 0.f, PGE_Window::Width, PGE_Window::Height ); GLERRORCHECK();

    //Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f ); GLERRORCHECK();

    glDisable( GL_DEPTH_TEST ); GLERRORCHECK();
    glDepthFunc(GL_NEVER); GLERRORCHECK();// Ignore depth values (Z) to cause drawing bottom to top

    //glDisable( GL_LIGHTING ); GLERRORCHECK();
    glEnable(GL_BLEND); GLERRORCHECK();
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); GLERRORCHECK();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLERRORCHECK();

    //glEnable( GL_TEXTURE_2D ); // Need this to display a texture
    //glEnableVertexAttribArray(0); GLERRORCHECK();
    //glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    //glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    //Check for error
//    GLenum error = glGetError();
//    if( error != GL_NO_ERROR )
//    {
//        QMessageBox::critical(NULL, "OpenGL Error",
//            QString("Error initializing OpenGL!\n%1")
//            .arg( getGlErrorStr(error) ), QMessageBox::Ok);
//       //printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
//       return false;
//    }

    ScreenshotPath = AppPathManager::userAppDir()+"/screenshots/";
    _isReady=true;

    resetViewport();

    //Init dummy texture;
    initDummyTexture();

    return true;
}

bool GlRenderer::uninit()
{
    //glDisable(GL_TEXTURE_2D);
    glDeleteTextures( 1, &(_dummyTexture.texture) );
    return false;
}

void GlRenderer::initDummyTexture()
{
    //loadTextureP(_dummyTexture, "://images/_broken.png");
    //FIBITMAP* sourceImage;
    QImage image = GraphicsHelps::convertToGLFormat(QImage("://images/_broken.png")).mirrored(false, true);
    //sourceImage = GraphicsHelps::loadImageRC("://images/_broken.png");
    int w = image.width();//FreeImage_GetWidth(sourceImage);
    int h = image.height();//FreeImage_GetHeight(sourceImage);
    //FreeImage_FlipVertical(sourceImage);
    _dummyTexture.nOfColors = GL_RGBA;
    _dummyTexture.format = GL_BGRA;
    _dummyTexture.w = w;
    _dummyTexture.h = h;
    GLubyte* textura= (GLubyte*)image.bits();//FreeImage_GetBits(sourceImage);

    //glEnable(GL_TEXTURE_2D); GLERRORCHECK();
    glActiveTexture( GL_TEXTURE0 ); GLERRORCHECK();
    glGenTextures( 1, &(_dummyTexture.texture) ); GLERRORCHECK();
    glBindTexture( GL_TEXTURE_2D, _dummyTexture.texture ); GLERRORCHECK();
    glTexImage2D( GL_TEXTURE_2D, 0, _dummyTexture.nOfColors, w, h, 0, _dummyTexture.format, GL_UNSIGNED_BYTE, textura ); GLERRORCHECK();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); GLERRORCHECK();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); GLERRORCHECK();
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); GLERRORCHECK();
    _dummyTexture.inited = true;
    //FreeImage_Unload(sourceImage);
}

PGE_Texture GlRenderer::loadTexture(QString path, QString maskPath)
{
    PGE_Texture target;
    loadTextureP(target, path, maskPath);
    return target;
}

void GlRenderer::loadTextureP(PGE_Texture &target, QString path, QString maskPath)
{
    //QImage sourceImage;
    //SDL_Surface * sourceImage;
    FIBITMAP* sourceImage;

    // Load the OpenGL texture
    //sourceImage = GraphicsHelps::loadQImage(path); // Gives us the information to make the texture
    sourceImage = GraphicsHelps::loadImage(path);

    //Don't load mask if PNG image is used
    if(path.endsWith(".png", Qt::CaseInsensitive)) maskPath.clear();

    if(!sourceImage)
    {
        WriteToLog(QtWarningMsg, QString("Error loading of image file: \n%1\nReason: %2.")
            .arg(path).arg(QFileInfo(path).exists()?"wrong image format":"file not exist"));
        target = _dummyTexture;
        return;
    }

    //Apply Alpha mask
    if(!maskPath.isEmpty() && QFileInfo(maskPath).exists())
    {
        //QImage maskImage = GraphicsHelps::loadQImage(maskPath);
        //sourceImage = GraphicsHelps::setAlphaMask(sourceImage, maskImage);
        GraphicsHelps::mergeWithMask(sourceImage, maskPath);
    }

    int w = FreeImage_GetWidth(sourceImage);
    int h = FreeImage_GetHeight(sourceImage);

    if((w<=0) || (h<=0))
    {
        FreeImage_Unload(sourceImage);
        WriteToLog(QtWarningMsg, QString("Error loading of image file: \n%1\nReason: %2.")
            .arg(path).arg("Zero image size!"));
        target = _dummyTexture;
        return;
    }

    //sourceImage=sourceImage.convertToFormat(QImage::Format_ARGB32);
    //Uint8 upperColor = GraphicsHelps::getPixel(sourceImage, 0, 0); //sourceImage.pixel(0,0);
    //target.ColorUpper.r = float((upperColor>>24)&0xFF)/255.0f;
    //target.ColorUpper.g = float((upperColor>>16)&0xFF)/255.0f;
    //target.ColorUpper.b = float((upperColor>>8)&0xFF)/255.0f;
    RGBQUAD upperColor;
    FreeImage_GetPixelColor(sourceImage, 0, 0, &upperColor);
    target.ColorUpper.r = float(upperColor.rgbRed)/255.0f;
    target.ColorUpper.b = float(upperColor.rgbBlue)/255.0f;
    target.ColorUpper.g = float(upperColor.rgbGreen)/255.0f;

    //Uint8 lowerColor = GraphicsHelps::getPixel(sourceImage, 0, sourceImage->h-1);//sourceImage.pixel(0, sourceImage.height()-1);
    //target.ColorLower.r = float((lowerColor>>24)&0xFF)/255.0f;
    //target.ColorLower.g = float((lowerColor>>16)&0xFF)/255.0f;
    //target.ColorLower.b = float((lowerColor>>8)&0xFF)/255.0f;
    RGBQUAD lowerColor;
    FreeImage_GetPixelColor(sourceImage, 0, h-1, &lowerColor);
    target.ColorLower.r = float(lowerColor.rgbRed)/255.0f;
    target.ColorLower.b = float(lowerColor.rgbBlue)/255.0f;
    target.ColorLower.g = float(lowerColor.rgbGreen)/255.0f;

    FreeImage_FlipVertical(sourceImage);

    //qDebug() << path << sourceImage.size();

    //sourceImage = GraphicsHelps::convertToGLFormat(sourceImage).mirrored(false, true);

    target.nOfColors = GL_RGBA;
    target.format = GL_BGRA;

    //glEnable(GL_TEXTURE_2D);
    // Have OpenGL generate a texture object handle for us
    glGenTextures( 1, &(target.texture) ); GLERRORCHECK();

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, target.texture ); GLERRORCHECK();

    // Edit the texture object's image data using the information SDL_Surface gives us
    target.w = w;
    target.h = h;
    // Set the texture's stretching properties

    GLubyte* textura= (GLubyte*)FreeImage_GetBits(sourceImage);

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, target.texture ); GLERRORCHECK();

//    glTexImage2D(GL_TEXTURE_2D, 0, target.nOfColors, sourceImage.width(), sourceImage.height(),
//         0, target.format, GL_UNSIGNED_BYTE, sourceImage.bits() );
    glTexImage2D(GL_TEXTURE_2D, 0, target.nOfColors, w, h,
           0, target.format, GL_UNSIGNED_BYTE, textura ); GLERRORCHECK();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); GLERRORCHECK();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); GLERRORCHECK();

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); GLERRORCHECK();
    glBindTexture( GL_TEXTURE_2D, 0); GLERRORCHECK();

    target.inited = true;

    //SDL_FreeSurface(sourceImage);
    FreeImage_Unload(sourceImage);

    return;
}

GLuint GlRenderer::QImage2Texture(QImage *img)
{
    if(!img) return 0;
    QImage text_image = GraphicsHelps::convertToGLFormat(*img);//.mirrored(false, true);

    GLuint texture=0;
    glActiveTexture( GL_TEXTURE0 ); GLERRORCHECK();
    glGenTextures(1, &texture);  GLERRORCHECK();
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texture);  GLERRORCHECK();
    glTexImage2D(GL_TEXTURE_2D, 0,  4, text_image.width(), text_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, text_image.bits() );  GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GLERRORCHECK();
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); GLERRORCHECK();
    return texture;
}

void GlRenderer::deleteTexture(PGE_Texture &tx)
{
    if( (tx.inited) && (tx.texture != _dummyTexture.texture))
    {
        //glDisable(GL_TEXTURE_2D);
        glDeleteTextures( 1, &(tx.texture) );
    }
    tx.inited = false;
    tx.inited=false;
    tx.w=0;
    tx.h=0;
    tx.texture_layout=NULL; tx.format=0;tx.nOfColors=0;
    tx.ColorUpper.r=0; tx.ColorUpper.g=0; tx.ColorUpper.b=0;
    tx.ColorLower.r=0; tx.ColorLower.g=0; tx.ColorLower.b=0;
}

void GlRenderer::deleteTexture(GLuint tx)
{
    glDeleteTextures( 1, &tx );
}










PGE_PointF GlRenderer::mapToOpengl(PGE_Point s)
{
    qreal nx  =  s.x() - qreal(PGE_Window::Width)  /  2;
    qreal ny  =  s.y() - qreal(PGE_Window::Height)  /  2;
    return PGE_PointF(nx, ny);
}

QString GlRenderer::ScreenshotPath = "";

struct PGE_GL_shoot{
    uchar* pixels;
    GLsizei w,h;
};

void GlRenderer::makeShot()
{
    if(!_isReady) return;

    // Make the BYTE array, factor of 3 because it's RBG.
    int w, h;
    SDL_GetWindowSize(PGE_Window::window, &w, &h);
    if((w==0) || (h==0))
    {
        PGE_Audio::playSoundByRole(obj_sound_role::WeaponFire);
        return;
    }

    w=w-offset_x*2;
    h=h-offset_y*2;

    uchar* pixels = new uchar[4*w*h];
    //glLoadIdentity();
    glReadPixels(offset_x, offset_y, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    PGE_GL_shoot *shoot=new PGE_GL_shoot();
    shoot->pixels=pixels;
    shoot->w=w;
    shoot->h=h;
    thread = SDL_CreateThread( makeShot_action, "scrn_maker", (void*)shoot );

    PGE_Audio::playSoundByRole(obj_sound_role::PlayerTakeItem);
}

int GlRenderer::makeShot_action(void *_pixels)
{
    PGE_GL_shoot *shoot = (PGE_GL_shoot*)_pixels;

    FIBITMAP* shotImg = FreeImage_ConvertFromRawBits((BYTE*)shoot->pixels, shoot->w, shoot->h,
                                     3*shoot->w+shoot->w%4, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
    if(!shotImg)
    {
        delete []shoot->pixels;
        shoot->pixels=NULL;
        delete []shoot;
        return 0;
    }

    FIBITMAP* temp;
    temp = FreeImage_ConvertTo32Bits(shotImg);
    if(!temp)
    {
        FreeImage_Unload(shotImg);
        delete []shoot->pixels;
        shoot->pixels=NULL;
        delete []shoot;
        return 0;
    }
    FreeImage_Unload(shotImg);
    shotImg = temp;

    if((shoot->w!=window_w)||(shoot->h!=window_h))
    {
        FIBITMAP* temp;
        temp = FreeImage_Rescale(shotImg, window_w, window_h, FILTER_BOX);
        if(!temp) {
            FreeImage_Unload(shotImg);
            delete []shoot->pixels;
            shoot->pixels=NULL;
            delete []shoot;
            return 0;
        }
        FreeImage_Unload(shotImg);
        shotImg = temp;
    }

    //QImage shotImg(shoot->pixels, shoot->w, shoot->h, QImage::Format_RGB888);
    //shotImg=shotImg.scaled(window_w, window_h).mirrored(false, true);
    if(!QDir(ScreenshotPath).exists()) QDir().mkpath(ScreenshotPath);

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();

    QString saveTo = QString("%1Scr_%2_%3_%4_%5_%6_%7_%8.png").arg(ScreenshotPath)
            .arg(date.year()).arg(date.month()).arg(date.day())
            .arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec());

    qDebug() << saveTo << shoot->w << shoot->h;
    //shotImg.save(saveTo, "PNG");
    if(FreeImage_HasPixels(shotImg) == FALSE) {
        qWarning() <<"Can't save screenshot: no pixel data!";
    } else {
        FreeImage_Save(FIF_PNG, shotImg, saveTo.toUtf8().data(), PNG_Z_BEST_COMPRESSION);
    }

    FreeImage_Unload(shotImg);

    delete []shoot->pixels;
    shoot->pixels=NULL;
    delete []shoot;

    return 0;
}

bool GlRenderer::ready()
{
    return _isReady;
}


void GlRenderer::setRGB(float Red, float Green, float Blue, float Alpha)
{
    color_level_red=Red;
    color_level_green=Green;
    color_level_blue=Blue;
    color_level_alpha=Alpha;
}

void GlRenderer::resetRGB()
{
    color_level_red=1.f;
    color_level_green=1.f;
    color_level_blue=1.f;
    color_level_alpha=1.f;
}


PGE_PointF GlRenderer::MapToGl(PGE_Point point)
{
    return MapToGl(point.x(), point.y());
}

PGE_PointF GlRenderer::MapToGl(float x, float y)
{
    double nx1 = roundf(x)/(viewport_w_half)-1.0;
    double ny1 = (viewport_h-(roundf(y)))/viewport_h_half-1.0;
    return PGE_PointF(nx1, ny1);
}

PGE_Point GlRenderer::MapToScr(PGE_Point point)
{
    return MapToScr(point.x(), point.y());
}

PGE_Point GlRenderer::MapToScr(int x, int y)
{
    return PGE_Point(((float(x))/viewport_scale_x)-offset_x, ((float(y))/viewport_scale_y)-offset_y);
}

int GlRenderer::alignToCenter(int x, int w)
{
    return x+(viewport_w_half-(w/2));
}

void GlRenderer::setViewport(int x, int y, int w, int h)
{
    glViewport(offset_x+x*viewport_scale_x,
               offset_y+(window_h-(y+h))*viewport_scale_y,
               w*viewport_scale_x, h*viewport_scale_y);  GLERRORCHECK();
    viewport_x=x;
    viewport_y=y;
    setViewportSize(w, h);
}

void GlRenderer::resetViewport()
{
    float w, w1, h, h1;
    int   wi, hi;
    SDL_GetWindowSize(PGE_Window::window, &wi, &hi);
    w=wi;h=hi; w1=w;h1=h;
    scale_x=(float)((float)(w)/(float)window_w);
    scale_y=(float)((float)(h)/(float)window_h);
    viewport_scale_x = scale_x;
    viewport_scale_y = scale_y;
    if(scale_x>scale_y)
    {
        w1=scale_y*window_w;
        viewport_scale_x=w1/window_w;
    }
    else if(scale_x<scale_y)
    {
        h1=scale_x*window_h;
        viewport_scale_y=h1/window_h;
    }

    offset_x=(w-w1)/2;
    offset_y=(h-h1)/2;

    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity(); GLERRORCHECK();
    glViewport(offset_x, offset_y, (GLsizei)w1, (GLsizei)h1); GLERRORCHECK();
    //gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    setViewportSize(window_w, window_h);
}

void GlRenderer::setViewportSize(int w, int h)
{
    viewport_w=w;
    viewport_h=h;
    viewport_w_half=w/2;
    viewport_h_half=h/2;
}

void GlRenderer::setWindowSize(int w, int h)
{
    window_w=w;
    window_h=h;
    resetViewport();
}

void GlRenderer::renderRect(float x, float y, float w, float h, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, bool filled)
{
    PGE_PointF point;
        point = MapToGl(x, y);
    float left = point.x();
    float top = point.y();
        point = MapToGl(x+w, y+h);
    float right = point.x();
    float bottom = point.y();

    glBindTexture( GL_TEXTURE_2D, 0 );  GLERRORCHECK();

    glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    #ifdef GL_GLEXT_PROTOTYPES
    glBlendEquation(GL_FUNC_ADD); GLERRORCHECK();
    #endif
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GLERRORCHECK();

    GLfloat Vertices[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0
    };
    GLfloat Colors[] = { red, green, blue, alpha,
                         red, green, blue, alpha,
                         red, green, blue, alpha,
                         red, green, blue, alpha };

    glVertexPointer(3, GL_FLOAT, 0, Vertices); GLERRORCHECK();
    glColorPointer(4, GL_FLOAT, 0, Colors); GLERRORCHECK();

    if(filled)
    {
        GLubyte indices[] = {
            0, 1, 2, // (bottom left - top left - top right)
            0, 2, 3  // (bottom left - top right - bottom right)
        };
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); GLERRORCHECK();
    } else {
        glDrawArrays(GL_LINE_LOOP, 0, 4); GLERRORCHECK();
    }

}

void GlRenderer::renderRectBR(float _left, float _top, float _right, float _bottom, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    PGE_PointF point;
        point = MapToGl(_left, _top);
    float left = point.x();
    float top = point.y();
        point = MapToGl(_right, _bottom);
    float right = point.x();
    float bottom = point.y();

    glBindTexture( GL_TEXTURE_2D, 0 ); GLERRORCHECK();

    glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  GLERRORCHECK();

    #ifdef GL_GLEXT_PROTOTYPES
    glBlendEquation(GL_FUNC_ADD); GLERRORCHECK();
    #endif
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GLERRORCHECK();

    GLfloat Vertices[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0
    };

    GLfloat Colors[] = { red, green, blue, alpha,
                         red, green, blue, alpha,
                         red, green, blue, alpha,
                         red, green, blue, alpha };

    GLubyte indices[] = {
        0, 1, 2, // (bottom left - top left - top right)
        0, 2, 3  // (bottom left - top right - bottom right)
    };
    glVertexPointer(3, GL_FLOAT, 0, Vertices); GLERRORCHECK();
    glColorPointer(4, GL_FLOAT, 0, Colors); GLERRORCHECK();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); GLERRORCHECK();
}

void GlRenderer::renderTexture(PGE_Texture *texture, float x, float y)
{
    if(!texture) return;

    PGE_PointF point;
        point = MapToGl(x, y);
    float left = point.x();
    float top = point.y();
        point = MapToGl(x+texture->w, y+texture->h);
    float right = point.x();
    float bottom = point.y();

    //glEnable(GL_TEXTURE_2D);

    glBindTexture( GL_TEXTURE_2D, texture->texture ); GLERRORCHECK();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_CLAMP); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP); GLERRORCHECK();

    glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    #ifdef GL_GLEXT_PROTOTYPES
    glBlendEquation(GL_FUNC_ADD); GLERRORCHECK();
    #endif
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); GLERRORCHECK();

    glColor4f( color_level_red, color_level_green, color_level_blue, color_level_alpha);
    GLfloat Vertices[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0
    };
    GLfloat TexCoord[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    GLubyte indices[] = {
        0, 1, 2, // (bottom left - top left - top right)
        0, 2, 3  // (bottom left - top right - bottom right)
    };

    GLfloat Colors[] = { 1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f,
                         1.0f, 1.0f, 1.0f, 1.0f };
    glColorPointer(4, GL_FLOAT, 0, Colors); GLERRORCHECK();
    glVertexPointer(3, GL_FLOAT, 0, Vertices); GLERRORCHECK();
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord); GLERRORCHECK();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); GLERRORCHECK();

    //glDisableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    glBindTexture( GL_TEXTURE_2D, 0 );
}

void GlRenderer::renderTexture(PGE_Texture *texture, float x, float y, float w, float h, float ani_top, float ani_bottom, float ani_left, float ani_right)
{
    if(!texture) return;
    PGE_PointF point;
        point = MapToGl(x, y);
    float left = point.x();
    float top = point.y();
        point = MapToGl(x+w, y+h);
    float right = point.x();
    float bottom = point.y();

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_CLAMP); GLERRORCHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP); GLERRORCHECK();

    glBindTexture( GL_TEXTURE_2D, texture->texture );

    glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    #ifdef GL_GLEXT_PROTOTYPES
    glBlendEquation(GL_FUNC_ADD); GLERRORCHECK();
    #endif
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); GLERRORCHECK();

    glColor4f( color_level_red, color_level_green, color_level_blue, color_level_alpha);
    GLfloat Vertices[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0
    };
    GLfloat TexCoord[] = {
        ani_left, ani_top,
        ani_right, ani_top,
        ani_right, ani_bottom,
        ani_left, ani_bottom
    };
    GLubyte indices[] = {
        0, 1, 2, // (bottom left - top left - top right)
        0, 2, 3  // (bottom left - top right - bottom right)
    };

    GLfloat Colors[] = { color_level_red, color_level_green, color_level_blue, color_level_alpha,
                         color_level_red, color_level_green, color_level_blue, color_level_alpha,
                         color_level_red, color_level_green, color_level_blue, color_level_alpha,
                         color_level_red, color_level_green, color_level_blue, color_level_alpha };
    glVertexPointer(3, GL_FLOAT, 0, Vertices); GLERRORCHECK();
    glColorPointer(4, GL_FLOAT, 0, Colors); GLERRORCHECK();
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord); GLERRORCHECK();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); GLERRORCHECK();

    //glDisableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    glBindTexture( GL_TEXTURE_2D, 0 );
}


void GlRenderer::BindTexture(PGE_Texture *texture)
{
    //glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->texture); GLERRORCHECK();

    glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    #ifdef GL_GLEXT_PROTOTYPES
    glBlendEquation(GL_FUNC_ADD); GLERRORCHECK();
    #endif
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); GLERRORCHECK();

    //glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();
}

void GlRenderer::BindTexture(GLuint &texture_id)
{
    //glEnable(GL_TEXTURE_2D); GLERRORCHECK();
    glBindTexture(GL_TEXTURE_2D, texture_id); GLERRORCHECK();

    glEnableClientState(GL_VERTEX_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_COLOR_ARRAY); GLERRORCHECK();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); GLERRORCHECK();

    #ifdef GL_GLEXT_PROTOTYPES
    glBlendEquation(GL_FUNC_ADD); GLERRORCHECK();
    #endif
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); GLERRORCHECK();
}

void GlRenderer::setTextureColor(float Red, float Green, float Blue, float Alpha)
{
    color_binded_texture[0]=Red;
    color_binded_texture[1]=Green;
    color_binded_texture[2]=Blue;
    color_binded_texture[3]=Alpha;

    color_binded_texture[4]=Red;
    color_binded_texture[5]=Green;
    color_binded_texture[6]=Blue;
    color_binded_texture[7]=Alpha;

    color_binded_texture[8]=Red;
    color_binded_texture[9]=Green;
    color_binded_texture[10]=Blue;
    color_binded_texture[11]=Alpha;

    color_binded_texture[12]=Red;
    color_binded_texture[13]=Green;
    color_binded_texture[14]=Blue;
    color_binded_texture[15]=Alpha;
}

void GlRenderer::renderTextureCur(float x, float y, float w, float h, float ani_top, float ani_bottom, float ani_left, float ani_right)
{
    PGE_PointF point;
        point = MapToGl(x, y);
    float left = point.x();
    float top = point.y();
        point = MapToGl(x+w, y+h);
    float right = point.x();
    float bottom = point.y();

    GLfloat Vertices[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0
    };

    GLfloat TexCoord[] = {
        ani_left, ani_top,
        ani_right, ani_top,
        ani_right, ani_bottom,
        ani_left, ani_bottom
    };

    GLubyte indices[] = {
        0, 1, 2, // (bottom left - top left - top right)
        0, 2, 3  // (bottom left - top right - bottom right)
    };

    glColorPointer(4, GL_FLOAT, 0, color_binded_texture); GLERRORCHECK();
    glVertexPointer(3, GL_FLOAT, 0, Vertices); GLERRORCHECK();
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord); GLERRORCHECK();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); GLERRORCHECK();
}

void GlRenderer::renderTextureCur(float x, float y)
{
    GLint w;
    GLint h;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WIDTH, &w); GLERRORCHECK();
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_HEIGHT,&h); GLERRORCHECK();
    if(w<0) return;
    if(h<0) return;

    PGE_PointF point;
        point = MapToGl(x, y);
    float left = point.x();
    float top = point.y();
        point = MapToGl(x+w, y+h);
    float right = point.x();
    float bottom = point.y();

    GLfloat Vertices[] = {
        left, top, 0,
        right, top, 0,
        right, bottom, 0,
        left, bottom, 0
    };
    GLfloat TexCoord[] = {
        0.f, 0.f,
        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f
    };
    GLubyte indices[] = {
        0, 1, 2, // (bottom left - top left - top right)
        0, 2, 3  // (bottom left - top right - bottom right)
    };

    glColorPointer(4, GL_FLOAT, 0, color_binded_texture); GLERRORCHECK();
    glVertexPointer(3, GL_FLOAT, 0, Vertices); GLERRORCHECK();
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoord); GLERRORCHECK();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); GLERRORCHECK();
}

void GlRenderer::getCurWidth(GLint &w)
{
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WIDTH, &w); GLERRORCHECK();
}

void GlRenderer::getCurHeight(GLint &h)
{
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_HEIGHT, &h); GLERRORCHECK();
}

void GlRenderer::UnBindTexture()
{
    glBindTexture( GL_TEXTURE_2D, 0 ); GLERRORCHECK();
}

