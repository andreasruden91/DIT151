#include "gameobject.h"
#include "displays.h"

GAMEOBJECT make_gameobject_raw(int x, int y, int w, int h, int num, POINT points[])
{
    int i;
    GAMEOBJECT object;
    
    object.x = x;
    object.y = y;
    object.w = w;
    object.h = h;
    object.vecx = 0;
    object.vecy = 0;
    object.lastWallCollide = 0;
    object.forceRedraw = 0;
    
    if (num > MAX_PIXELS)
        num = MAX_PIXELS;
    object.numPixels = num;
    for (i = 0; i < object.numPixels; ++i)
        object.pixels[i] = points[i];
    
    return object;
}

void gameobject_draw(GAMEOBJECT* object)
{
    int i;
    
    for (i = 0; i < object->numPixels; ++i)
    {
        int x = object->x + object->pixels[i].x;
        int y = object->y + object->pixels[i].y;
        lcd_draw(x, y);
    }
}

void gameobject_erase(GAMEOBJECT* object)
{
    int i;
    
    for (i = 0; i < object->numPixels; ++i)
    {
        int x = object->x + object->pixels[i].x;
        int y = object->y + object->pixels[i].y;
        lcd_erase(x, y);
    }
}

void gameobject_set_speed(GAMEOBJECT* object, int vx, int vy)
{
    object->vecx = vx;
    object->vecy = vy;
}

void gameobject_update(GAMEOBJECT* object)
{
    int x, y;
    
    x = object->x + object->vecx;
    y = object->y + object->vecy;
    
    if (x <= 0 || (x + object->w) >= 128)
    {
        object->vecx = -object->vecx;
        object->lastWallCollide = (x <= 0) ? LEFT_WALL : RIGHT_WALL;
    }
        
    if (y <= 0 || (y + object->h) >= 64)
    {
        object->vecy = -object->vecy;
        object->lastWallCollide = (y <= 0) ? UP_WALL : DOWN_WALL;
    }
    
    x = object->x + object->vecx; // Recalculate x and y in case vector changed
    y = object->y + object->vecy;
    if (x != object->x || y != object->y || object->forceRedraw)
    {
        object->forceRedraw = 0;
        gameobject_erase(object);
        object->x = x;
        object->y = y;
        gameobject_draw(object);
    }
}
