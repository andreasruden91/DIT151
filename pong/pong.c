/*
 * 	pong.c
 *
 */
 
#include "displays.h"
#include "gameobject.h"
#include "keyboard.h"

void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup ( void )
{
__asm volatile(
    " LDR R0,=0x2001C000\n"
    " MOV SP,R0\n"
    " BL main\n"
    "_exit: B .\n"
    ) ;
}

static const POINT ballGeometry[] =
{
    {0,1}, {0,2}, {1,0}, {1,1}, {1,2},
    {1,3}, {2,0}, {2,1}, {2,2}, {2,3},
    {3,1}, {3,2}
};

static const POINT padGeometry[] =
{
    {0,0}, {1,0},
    {0,1}, {1,1},
    {0,2}, {1,2},
    {0,3}, {1,3},
    {0,4}, {1,4},
    {0,5}, {1,5},
    {0,6}, {1,6},
    {0,7}, {1,7},
    {0,8}, {1,8},
    {0,9}, {1,9},
};

#define BALL_START_X 61
#define BALL_START_Y 29
#define LEFT_PADDLE_START_X 3
#define LEFT_PADDLE_START_Y 26
#define RIGHT_PADDLE_START_X 122
#define RIGHT_PADDLE_START_Y 26

GAMEOBJECT ball;
GAMEOBJECT leftPad, rightPad;
int ballStartDirection = 0;
int leftScore = 0;
int rightScore = 0;

void init_app(void)
{
#ifdef USBDM
    /* starta klockor port D och E */
    *((unsigned long*)0x40023830) = 0x18;
    /* initiera PLL */
    __asm volatile("LDR R0, =0x08000209\n BLX R0 \n")
#endif

    lcd_init();
    ascii_init();
    kbd_init();
    
    ball = make_gameobject_raw(BALL_START_X, BALL_START_Y, 4, 4, 12, ballGeometry);
    leftPad = make_gameobject_raw(LEFT_PADDLE_START_X, LEFT_PADDLE_START_Y, 2, 10, 20, padGeometry);
    rightPad = make_gameobject_raw(RIGHT_PADDLE_START_X, RIGHT_PADDLE_START_Y, 2, 10, 20, padGeometry);
}

void start_round(void)
{
    gameobject_erase(&ball);
    gameobject_erase(&leftPad);
    gameobject_erase(&rightPad);
    
    ball.x = BALL_START_X;
    ball.y = BALL_START_Y;
    ball.lastWallCollide = 0;
    leftPad.x = LEFT_PADDLE_START_X;
    leftPad.y = LEFT_PADDLE_START_Y;
    rightPad.x = RIGHT_PADDLE_START_X;
    rightPad.y = RIGHT_PADDLE_START_Y;
    
    switch (ballStartDirection++ % 4)
    {
    case 0: gameobject_set_speed(&ball, 3, 2); break;
    case 1: gameobject_set_speed(&ball, -3, 2); break;
    case 2: gameobject_set_speed(&ball, 3, -2); break;
    case 3: gameobject_set_speed(&ball, -3, -2); break;
    }
    
    gameobject_set_speed(&leftPad, 0, 0);
    gameobject_set_speed(&rightPad, 0, 0);
    
    ball.forceRedraw = 1;
    leftPad.forceRedraw = 1;
    rightPad.forceRedraw = 1;
}

void write_str(const char* s)
{
    while (*s)
        ascii_write_char(*s++);
}

char its_buf[11];
const char* int_to_str(int n)
{
    int stack[10];
    int buf_index = 0, stack_index = 0;
    
    if (n < 0)
    {
        its_buf[buf_index++] = '-';
        n = -n;
    }
    
    // Zero special case
    if (n == 0)
        stack[stack_index++] = 0;
    
    while (n)
    {
        stack[stack_index++] = n % 10;
        n /= 10;
    }
    
    while (stack_index > 0)
        its_buf[buf_index++] = '0' + stack[--stack_index];
    
    its_buf[buf_index] = '\0';
    
    return its_buf;
}

void print_score(void)
{
    ascii_clear();
    
    ascii_gotoxy(0, 0);
    write_str("Left ");
    write_str(int_to_str(leftScore));
    write_str(" - ");
    write_str(int_to_str(rightScore));
    write_str(" Right");
}

void bounce_up(GAMEOBJECT* ball, GAMEOBJECT* pad, int rightSide)
{
    gameobject_set_speed(ball, rightSide ? -3 : 3, -2);
    pad->forceRedraw = 1;
}

void bounce_forward(GAMEOBJECT* ball, GAMEOBJECT* pad, int rightSide)
{
    gameobject_set_speed(ball, rightSide ? -3 : 3, 0);
    pad->forceRedraw = 1;
}

void bounce_down(GAMEOBJECT* ball, GAMEOBJECT* pad, int rightSide)
{
    gameobject_set_speed(ball, rightSide ? -3 : 3, 2);
    pad->forceRedraw = 1;
}

void ball_pad_collision(GAMEOBJECT* ball, GAMEOBJECT* pad, int rightSide)
{
    int touchDist;
    
    // Rectangle to Rectangle collision detection
    if (!(pad->x < ball->x + ball->h &&
       pad->x + pad->w > ball->x &&
       pad->y < ball->y + ball->h &&
       pad->h + pad->y > ball->y)) {
        return; // No collision
    }
        
    // Check where on paddle ball's middle touch
    touchDist = (ball->y + ball->h/2) - pad->y;
    if (touchDist < 3)
        bounce_up(ball, pad, rightSide);
    else if (touchDist < 7)
        bounce_forward(ball, pad, rightSide);
    else 
        bounce_down(ball, pad, rightSide);
}

int main(void)
{
    int restart = 1;
    
    init_app();
    
#ifndef SIMULATOR
    lcd_clear();
#endif
    
    while (1)
    {
        if (restart)
        {
            start_round();
            print_score();
            restart = 0;
        }
        
        // Keyboard input
        if (iskeydown(1))
            gameobject_set_speed(&leftPad, 0, -2);
        else if (iskeydown(7))
            gameobject_set_speed(&leftPad, 0, 2);
        if (iskeydown(3))
            gameobject_set_speed(&rightPad, 0, -2);
        else if (iskeydown(9))
            gameobject_set_speed(&rightPad, 0, 2);
        
        // Frame Update
        ball_pad_collision(&ball, &leftPad, 0);
        ball_pad_collision(&ball, &rightPad, 1);
        gameobject_update(&ball);
        gameobject_update(&leftPad);
        gameobject_update(&rightPad);
        
#ifndef SIMULATOR
        delay_milli(40);
#endif
    
        if (ball.lastWallCollide == LEFT_WALL)
        {
            ++rightScore;
            restart = 1;
        }
        else if (ball.lastWallCollide == RIGHT_WALL)
        {
            ++leftScore;
            restart = 1;
        }
    }
    
    return 0;
}
