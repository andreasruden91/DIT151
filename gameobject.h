#ifndef GEOMETRY_H
#define GEOMETRY_H

#define MAX_PIXELS 20

typedef struct Point
{
    unsigned char x;
    unsigned char y;
} POINT;

typedef struct GameObject
{
    int x;
    int y;
    int w;
    int h;
    int vecx;
    int vecy;
    
    
    int numPixels;
    POINT pixels[MAX_PIXELS];
} GAMEOBJECT;

GAMEOBJECT make_gameobject_raw(int x, int y, int w, int h, int num, POINT points[]);

/* todo */
GAMEOBJECT make_rectangle(int width, int height);
GAMEOBJECT make_circle(int radius);

// prerequisite: using lcd display
void gameobject_draw(GAMEOBJECT* object);

// prerequisite: using lcd display
void gameobject_erase(GAMEOBJECT* object);

void gameobject_set_speed(GAMEOBJECT* object, int vx, int vy);

void gameobject_update(GAMEOBJECT* object);

#endif
