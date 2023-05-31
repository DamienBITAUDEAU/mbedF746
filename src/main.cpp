#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

ThreadLvgl threadLvgl(30);

void myEvent(lv_event_t *event)
{    
    printf("Event\n");
    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t p;
    lv_indev_get_point(indev, &p);
    
}

int main() {

    int player_position = 0;
    int enemy1_position = -20;
    char direction_p = 'l';
    static lv_point_t line_point_p[]={{0, 0},
                                    {0, 12},
                                    {12, 12},
                                    {12, 0}};

    static lv_style_t style_line_p;
    lv_style_init(&style_line_p);
    lv_style_set_line_width(&style_line_p, 8);
    lv_style_set_line_color(&style_line_p, lv_palette_main(LV_PALETTE_DEEP_PURPLE));
    lv_style_set_line_rounded(&style_line_p, true);


    static lv_point_t line_point_e[]={{0, 0},
                                    {0, 18},
                                    {18, 18},
                                    {18, 0}};

    static lv_style_t style_line_e;
    lv_style_init(&style_line_e);
    lv_style_set_line_width(&style_line_e, 10);
    lv_style_set_line_color(&style_line_e, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_line_rounded(&style_line_e, true);


    threadLvgl.lock();
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    

    lv_obj_t * player;
    lv_obj_t * enemy1;
    player = lv_line_create(lv_scr_act());
    lv_line_set_points(player, line_point_p, 5);
    lv_obj_add_style(player, &style_line_p, 0);

    enemy1 = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy1, line_point_e, 5);
    lv_obj_add_style(enemy1, &style_line_e, 0);

    //positionnment de départ
    lv_obj_align(player, LV_ALIGN_DEFAULT,0,0);
    lv_obj_align(enemy1, LV_ALIGN_DEFAULT,0,0);    

    lv_obj_set_x(enemy1, enemy1_position);
    lv_obj_set_y(enemy1, 136);
    lv_obj_set_x(player, 460);
    lv_obj_set_y(player, player_position);

    lv_obj_add_event_cb(lv_scr_act(), myEvent, LV_EVENT_PRESSING, nullptr);

    threadLvgl.unlock();

    while (1) {
        // put your main code here, to run repeatedly:
        switch (direction_p)
        {
        case 'l':
            if(player_position < 255)player_position = player_position +1;
            else  direction_p ='r';
            break;
        
        case 'r':
            if(player_position > 7)player_position = player_position -1;
            else direction_p ='l';
            break;
        
        default:
            break;
        }

        if(enemy1_position <500)enemy1_position = enemy1_position + 1.5;
        else{
            enemy1_position = -20;
            lv_obj_set_y(enemy1, rand() % 245 + 10 );
        } 

        threadLvgl.lock();
        lv_obj_set_x(enemy1, enemy1_position);
        lv_obj_set_y(player, player_position);
        threadLvgl.unlock();

        ThisThread::sleep_for(20ms);
    }

    
}