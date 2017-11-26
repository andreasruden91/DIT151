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
    
    gameobject_erase(object);
    
    x = object->x + object->vecx;
    y = object->y + object->vecy;
    
    if (x <= 0 || (x + object->w) >= 128)
        object->vecx = -object->vecx;
        
    if (y <= 0 || (y + object->h) >= 64)
        object->vecy = -object->vecy;
        
    // Recalculate x and y in case vector changed
    object->x = object->x + object->vecx;
    object->y = object->y + object->vecy;
    
    gameobject_draw(object);
}
