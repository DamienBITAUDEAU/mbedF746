#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>
#include <ctime>

#include <MPU6050.h>

ThreadLvgl threadLvgl(30);

MPU6050 accelero;

volatile char game_state = 's';

void myEvent(lv_event_t *event)
{    
    if (game_state == 's') game_state = 'g';
    else if (game_state == 'l')game_state = 's';
}

int main() {
    srand(time(NULL));
    float player_speed = 1.7;    
    char direction_p = 'l';
    int player_y = 125;
    int player_x = 460;

    float enemy1_speed = 1.5;
    int enemy1_x = -20;
    int enemy1_y = rand() % 245 + 10;

    int score = 0;
    int highest_scr = 0;
    

    i2c.frequency(400000);
    accelero.resetMPU6050(); // Reset registers to default in preparation for device calibration
    accelero.calibrateMPU6050(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers  
    accelero.initMPU6050(); printf("MPU6050 initialized for active data mode....\n\r"); // Initialize device for active mode read of acclerometer, gyroscope, and temperature

   
    static lv_point_t line_point_p[]={{0, 0},
                                    {0, 12},
                                    {12, 12},
                                    {12, 0},{0, 0}};

    static lv_style_t style_line_p;
    lv_style_init(&style_line_p);
    lv_style_set_line_width(&style_line_p, 8);
    lv_style_set_line_color(&style_line_p, lv_palette_main(LV_PALETTE_DEEP_PURPLE));
    lv_style_set_line_rounded(&style_line_p, true);


    static lv_point_t line_point_e[]={{0, 0},
                                    {0, 18},
                                    {18, 18},
                                    {18, 0},{0, 0}};

    static lv_style_t style_line_e;
    lv_style_init(&style_line_e);
    lv_style_set_line_width(&style_line_e, 10);
    lv_style_set_line_color(&style_line_e, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_line_rounded(&style_line_e, true);


    threadLvgl.lock();
    
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);   

    lv_obj_t * player;
    lv_obj_t * enemy1;
     
    lv_obj_t * label_score; 
    lv_obj_t * label_highest_scr;  

    player = lv_line_create(lv_scr_act());
    lv_line_set_points(player, line_point_p, 5);
    lv_obj_add_style(player, &style_line_p, 0);

    enemy1 = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy1, line_point_e, 5);
    lv_obj_add_style(enemy1, &style_line_e, 0);

    label_highest_scr = lv_label_create(lv_scr_act());
    lv_obj_align(label_highest_scr, LV_ALIGN_DEFAULT, 5, 5); 

    label_score = lv_label_create(lv_scr_act());
    lv_obj_align(label_score, LV_ALIGN_DEFAULT, 5, 20);   

       

    //positionnment de départ
    lv_obj_align(player, LV_ALIGN_DEFAULT,0,0);
    lv_obj_align(enemy1, LV_ALIGN_DEFAULT,0,0);    

    lv_obj_set_x(enemy1, enemy1_x);
    lv_obj_set_y(enemy1, player_y);
    lv_obj_set_x(player, player_x);
    lv_obj_set_y(player, player_y);

    lv_obj_add_event_cb(lv_scr_act(), myEvent, LV_EVENT_CLICKED, nullptr);

    threadLvgl.unlock();
    
    

    while (1) {

        switch (game_state)
        {
            //////////////START GAME MENU STATE//////////////
            /////WAIT FOR TOUCH EVENT TO START THE GAME/////
            case 's':
                if(enemy1_x <500)enemy1_x = enemy1_x + enemy1_speed;
                 else{
                    enemy1_x = -20;
                    enemy1_y = rand() % 245 + 10;
                 }  
                switch (direction_p)
                {
                    case 'l':
                        if(player_y < 255)player_y = player_y + 1;
                        else  direction_p ='r';
                        break;
                    
                    case 'r':
                        if(player_y > 7)player_y = player_y - 1;
                        else direction_p ='l';
                        break;                    
                    default:
                        break;
                }       
                break;
            
            //////////////ACTIVE GAME STATE//////////////
            case 'g':
                 score +=2;

                // put your main code here, to run repeatedly:
                /////////////////////////////////////MOUVEMENT DE L'ENNEMI/////////////////////////////////////     
                if(enemy1_x <500)enemy1_x = enemy1_x + enemy1_speed;
                else{
                    enemy1_x = -20;
                    enemy1_y = rand() % 245 + 10;
                    
                }
                if ((score/10)%20 == 0 && score/10 < 300)enemy1_speed +=0.1;
                if ((score/10)%500 == 0 && score/10 < 3000)enemy1_speed +=0.1;
                //////////////////////////////////FIN MOUVEMENT DE L'ENNEMI//////////////////////////////////

                /////////////////////////////////////MOUVEMENT DU JOUEUR/////////////////////////////////////
                if(accelero.readByte(MPU6050_ADDRESS, INT_STATUS) & 0x01) {  // check if data ready interrupt
                    accelero.readAccelData(accelCount);  // Read the x/y/z adc values
                    accelero.getAres();
                    
                    ay = (float)accelCount[1]*aRes;        

                    if(ay*1000 > 50){direction_p= 'r';player_speed = 1.3*ay*10;}
                    else if (ay*1000 < -50){direction_p= 'l';player_speed = 1.3*abs(ay)*10;}
                    else direction_p= 's';
                }

                switch (direction_p)
                {
                case 'l':
                    if(player_y < 255)player_y = player_y + player_speed;
                    else  direction_p ='r';
                    break;
                
                case 'r':
                    if(player_y > 7)player_y = player_y - player_speed;
                    else direction_p ='l';
                    break;
                
                default:
                    break;
                }
                //////////////////////////////////FIN MOUVEMENT DU JOUEUR//////////////////////////////////

                /////////////////////////////////////VERIFICATION DE CONTACTE/////////////////////////////////////
                if (enemy1_x>player_x-27 && enemy1_x<player_x+20){
                    if(enemy1_y>player_y-26 && enemy1_y<player_y+20 ){game_state = 'l';}
                }
                ///////////////////////////////////FIN VERIFICATION DE CONTACTE///////////////////////////////////
                break;

            //////////////LOST GAME STATE//////////////
            case 'l':
                    if (highest_scr< score)highest_scr=score;
                    score = 0;
                    osDelay(1800);
                    enemy1_x = -20;
                    player_y = 125;
                    enemy1_speed = 1.5;
                    game_state = 's';
                break;

            default:
                break;
        }
       

        
       
        threadLvgl.lock();
        lv_obj_set_x(enemy1, enemy1_x);
        lv_obj_set_y(enemy1, enemy1_y);
        lv_obj_set_y(player, player_y);        
        
        lv_label_set_text_fmt(label_score, "Score: %d", score/10);
        lv_label_set_text_fmt(label_highest_scr, "High Score: %f", enemy1_speed);
        threadLvgl.unlock();
        

        ThisThread::sleep_for(20ms);
    }

    
}