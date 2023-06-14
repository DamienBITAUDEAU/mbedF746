#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>
#include <ctime>

#include <MPU6050.h>

#include <pwm_tone.h>

ThreadLvgl threadLvgl(30);

MPU6050 accelero;

volatile char game_state = 's';

struct position
{
   int x;
   int y;
};

PwmOut Buzz(A0);

int melody_1[] = {  NOTE_E5, NOTE_E5, NOTE_E5, REST, NOTE_C5, NOTE_E5, 
                    NOTE_G5, REST, NOTE_G4, REST,
                    NOTE_C5, NOTE_G4, REST, NOTE_E4, 
                    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
                    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
                    REST, NOTE_E5,NOTE_C5, NOTE_D5, NOTE_B4,
                    NOTE_C5, NOTE_G4, REST, NOTE_E4,
                    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
                    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
                    REST, NOTE_E5,NOTE_C5, NOTE_D5, NOTE_B4, FIN};

int melody_2[] = {  REST, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_E5,
                    REST, NOTE_GS4, NOTE_A4, NOTE_C4, REST, NOTE_A4, NOTE_C5, NOTE_D5,
                    REST, NOTE_DS5, REST, NOTE_D5,
                    NOTE_C5, REST,};

int active_gs_melody_position = 0;



void myEvent(lv_event_t *event){    
    if (game_state == 's') game_state = 'g';
    else if (game_state == 'l')game_state = 's';   
}



bool contact (struct position enemy, struct position player);

struct position enemy_move(struct position current_enemy, struct position scnd_enemy, struct position thrd_enemy, struct position frth_enemy, int enemy_speed);

int play_melody ( int melody[], int position);


int main() {
    srand(time(NULL));
    float player_speed = 1.7;    
    char direction_p = 'l';
    char * message = "";
    char * no_msg = "";
    char * start_msg = "TAP TO PLAY";
    char * Game_over_msg = "GAME OVER";

    struct position player;
    player.x = 460;
    player.y = 125; 

    float enemies_speed = 1.5;

    struct position enemy_1;
    enemy_1.x = -20;
    enemy_1.y = rand() % 245 + 10;

    struct position enemy_2;
    enemy_2.x = rand() % -200 - 400;
    enemy_2.y = rand() % 245 + 10;

    struct position enemy_3;
    enemy_3.x = rand() % -400 - 600;
    enemy_3.y = rand() % 245 + 10;

    struct position enemy_4;
    enemy_4.x = rand() % -800 - 1000;
    enemy_4.y = rand() % 245 + 10; 

    struct position temp_pos; 

    int score = 0;
    int highest_scr = 0;

    int blink_delay = 0;
    bool blink_state = true;
    

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

    lv_obj_t * obj_player;
    lv_obj_t * enemy1;
    lv_obj_t * enemy2;
    lv_obj_t * enemy3;
    lv_obj_t * enemy4;
    
     
    lv_obj_t * label_score; 
    lv_obj_t * label_highest_scr;  
    lv_obj_t * label_message;

    obj_player = lv_line_create(lv_scr_act());
    lv_line_set_points(obj_player, line_point_p, 5);
    lv_obj_add_style(obj_player, &style_line_p, 0);

    enemy1 = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy1, line_point_e, 5);
    lv_obj_add_style(enemy1, &style_line_e, 0);

    enemy2 = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy2, line_point_e, 5);
    lv_obj_add_style(enemy2, &style_line_e, 0);

    enemy3 = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy3, line_point_e, 5);
    lv_obj_add_style(enemy3, &style_line_e, 0);

    enemy4 = lv_line_create(lv_scr_act());
    lv_line_set_points(enemy4, line_point_e, 5);
    lv_obj_add_style(enemy4, &style_line_e, 0);

    label_highest_scr = lv_label_create(lv_scr_act());
    lv_obj_align(label_highest_scr, LV_ALIGN_DEFAULT, 5, 5); 

    label_score = lv_label_create(lv_scr_act());
    lv_obj_align(label_score, LV_ALIGN_DEFAULT, 5, 20); 
    
    label_message = lv_label_create(lv_scr_act());
    lv_obj_align(label_message, LV_ALIGN_CENTER,0,0);  
    lv_obj_add_style(label_message, &style_line_e, 0);

       

    //positionnment de départ
    lv_obj_align(obj_player, LV_ALIGN_DEFAULT,0,0);
    lv_obj_align(enemy1, LV_ALIGN_DEFAULT,0,0);    
    lv_obj_align(enemy2, LV_ALIGN_DEFAULT,0,0);    
    lv_obj_align(enemy3, LV_ALIGN_DEFAULT,0,0);    
    lv_obj_align(enemy4, LV_ALIGN_DEFAULT,0,0);    

    lv_obj_set_x(enemy1, enemy_1.x);
    lv_obj_set_y(enemy1, enemy_1.y);

    lv_obj_set_x(enemy2, enemy_2.x);
    lv_obj_set_y(enemy2, enemy_2.y);

    lv_obj_set_x(enemy3, enemy_3.x);
    lv_obj_set_y(enemy3, enemy_3.y);

    lv_obj_set_x(enemy4, enemy_4.x);
    lv_obj_set_y(enemy4, enemy_4.y);

    lv_obj_set_x(obj_player, player.x);
    lv_obj_set_y(obj_player, player.y);

    lv_obj_add_event_cb(lv_scr_act(), myEvent, LV_EVENT_CLICKED, nullptr);

    threadLvgl.unlock();
    
    

    while (1) {

        switch (game_state)
        {
            //////////////START GAME MENU STATE//////////////
            /////WAIT FOR TOUCH EVENT TO START THE GAME/////
            case 's':

                blink_delay +=1;
                if (blink_delay == 30){
                    if(blink_state == true){
                        message = start_msg;
                        blink_state = false;                    
                    }else if (blink_state == false){
                       message = no_msg;
                        blink_state = true;  
                    }
                    blink_delay = 0;                    
                }
               
                temp_pos = enemy_move(enemy_1, enemy_2, enemy_3, enemy_4, enemies_speed);
                enemy_1.x = temp_pos.x;
                enemy_1.y = temp_pos.y;

                temp_pos = enemy_move(enemy_2, enemy_1, enemy_3, enemy_4, enemies_speed);
                enemy_2.x = temp_pos.x;
                enemy_2.y = temp_pos.y;

                temp_pos = enemy_move(enemy_3, enemy_1, enemy_2, enemy_4, enemies_speed);
                enemy_3.x = temp_pos.x;
                enemy_3.y = temp_pos.y;

                temp_pos = enemy_move(enemy_4, enemy_1, enemy_2, enemy_3, enemies_speed);
                enemy_4.x = temp_pos.x;
                enemy_4.y = temp_pos.y;

                switch (direction_p)
                {
                    case 'l':
                        if(player.y < 255)player.y = player.y + 1;
                        else  direction_p ='r';
                        break;
                    
                    case 'r':
                        if(player.y > 7)player.y = player.y - 1;
                        else direction_p ='l';
                        break;                    
                    default:
                        break;
                }       
                break;
            
            //////////////ACTIVE GAME STATE//////////////
            case 'g':
                blink_state = false;
                score +=2;
                if(active_gs_melody_position !=9999)active_gs_melody_position=play_melody(melody_1, active_gs_melody_position);
                /////////////////////////////////////MOUVEMENT DE L'ENNEMI/////////////////////////////////////    
                temp_pos = enemy_move(enemy_1, enemy_2, enemy_3, enemy_4, enemies_speed);
                enemy_1.x = temp_pos.x;
                enemy_1.y = temp_pos.y;

                temp_pos = enemy_move(enemy_2, enemy_1, enemy_3, enemy_4, enemies_speed);
                enemy_2.x = temp_pos.x;
                enemy_2.y = temp_pos.y;

                temp_pos = enemy_move(enemy_3, enemy_1, enemy_2, enemy_4, enemies_speed);
                enemy_3.x = temp_pos.x;
                enemy_3.y = temp_pos.y;

                temp_pos = enemy_move(enemy_4, enemy_1, enemy_2, enemy_3, enemies_speed);
                enemy_4.x = temp_pos.x;
                enemy_4.y = temp_pos.y;

                 
                if ((score/10)%20 == 0 && score/10 < 300)enemies_speed +=0.1;
                if ((score/10)%500 == 0 && score/10 < 3000)enemies_speed +=0.1;
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
                    if(player.y < 255)player.y = player.y + player_speed;
                    else  direction_p ='r';
                    break;
                
                case 'r':
                    if(player.y > 7)player.y = player.y - player_speed;
                    else direction_p ='l';
                    break;
                
                default:
                    break;
                }
                //////////////////////////////////FIN MOUVEMENT DU JOUEUR//////////////////////////////////

                /////////////////////////////////////VERIFICATION DE CONTACTE/////////////////////////////////////
                if(contact(enemy_1, player) == true ||
                   contact(enemy_2, player) == true ||
                   contact(enemy_3, player) == true ||
                   contact(enemy_4, player) == true )game_state = 'l';               
                
                ///////////////////////////////////FIN VERIFICATION DE CONTACTE///////////////////////////////////
                break;

            //////////////LOST GAME STATE//////////////
            case 'l':
                    if (highest_scr< score)highest_scr=score;
                    score = 0;

                
                    Buzz.period_us(Do5);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(2*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(So4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(2*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(Mi4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(2*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(La4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(Ti4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(La4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(So4s);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(La4s);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(So4s);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(So4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(Re4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(4*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    Buzz.period_us(Mi4);
                    Buzz.write(0.15f); // 50% duty cycle
                    osDelay(2*63); // 1 beat
                    Buzz.write(0.0f); // Sound off

                    
                    osDelay(1300);
                    enemy_1.x = -20;
                    player.y = 125;
                    enemies_speed = 1.5;
                    game_state = 's';
                    direction_p = 'l';
                    active_gs_melody_position = 0;
                    start_msg = Game_over_msg;

                break;

            default:
                break;
        }
       

        
       
        threadLvgl.lock();
        lv_obj_set_x(enemy1, enemy_1.x);
        lv_obj_set_y(enemy1, enemy_1.y);
        lv_obj_set_x(enemy2, enemy_2.x);
        lv_obj_set_y(enemy2, enemy_2.y);
        lv_obj_set_x(enemy3, enemy_3.x);
        lv_obj_set_y(enemy3, enemy_3.y);
        lv_obj_set_x(enemy4, enemy_4.x);
        lv_obj_set_y(enemy4, enemy_4.y);
        lv_obj_set_y(obj_player, player.y);        
        
        lv_label_set_text_fmt(label_score, "Score: %d", score/10);
        lv_label_set_text_fmt(label_highest_scr, "High Score: %d", highest_scr/10);
        lv_label_set_text_fmt(label_message,"%s", message);

        threadLvgl.unlock();
        

        ThisThread::sleep_for(20ms);
    }

    
}

bool contact (struct position enemy, struct position player){
    if (enemy.x>player.x-27 && enemy.x<player.x+20){
        if(enemy.y>player.y-26 && enemy.y<player.y+20 ){
            return true;
        }else{return false;}
    }else{return false;}
}


struct position enemy_move(struct position current_enemy, struct position scnd_enemy, struct position thrd_enemy, struct position frth_enemy, int enemy_speed){
    struct position new_position;
    if(current_enemy.x <500)current_enemy.x = current_enemy.x + enemy_speed;
    else{
        current_enemy.x = rand() % -20 - 110;
        current_enemy.y = rand() % 245 + 10;
        while ((current_enemy.y>scnd_enemy.y-30 && current_enemy.y<scnd_enemy.y+30) || (current_enemy.y>thrd_enemy.y-30 && current_enemy.y<thrd_enemy.y+30) || (current_enemy.y>frth_enemy.y-30 && current_enemy.y<frth_enemy.y+30))
        {
            current_enemy.y = rand() % 245 + 10;
        }                    
    }  
    new_position.x = current_enemy.x;
    new_position.y = current_enemy.y;
    return new_position;
}

int play_melody ( int melody[], int position){
    
    Buzz.period_us(melody[position]);
    Buzz.write(0.15f); // 50% duty cycle
    //osDelay(4*63); // 1 beat
    //
    if (melody[position] >= 9999){
        Buzz.write(0.0f); // Sound off        
        return position = 9999;
    }
    return position + 1;
}


bool message_blink(int blink_delay, bool blink_state){
    if(blink_delay<10000){
        return blink_state;
    }
    else{
        if(blink_state == true)return false;
        else if (blink_state == false)return true;        
    }
    
}


