#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

ThreadLvgl threadLvgl(30);

int main() {

    int player_position = 0;
    int enemy_position = 0;
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
    lv_obj_t * palyer;
    lv_obj_t * enemy;
    palyer = lv_line_create(lv_scr_act());
    lv_line_set_points(palyer, line_point_p, 5);
    lv_obj_add_style(palyer, &style_line_p, 0);

    enemy = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy, line_point_e, 5);
    lv_obj_add_style(enemy, &style_line_e, 0);

    //lv_obj_center(line1);
    lv_obj_align(palyer, LV_ALIGN_RIGHT_MID,0,player_position);
    lv_obj_align(enemy, LV_ALIGN_LEFT_MID,enemy_position,0);
    threadLvgl.unlock();

    while (1) {
        // put your main code here, to run repeatedly:
        switch (direction_p)
        {
        case 'l':
            if(player_position < 126)player_position = player_position +1;
            else  direction_p ='r';
            break;
        
        case 'r':
            if(player_position > -118)player_position = player_position -1;
            else direction_p ='l';
            break;
        
        default:
            break;
        }
        enemy_position = enemy_position + 1;

        lv_obj_align(enemy, LV_ALIGN_LEFT_MID,enemy_position,0);
        lv_obj_align(palyer, LV_ALIGN_RIGHT_MID,0,player_position);
        ThisThread::sleep_for(20ms);
    }
}