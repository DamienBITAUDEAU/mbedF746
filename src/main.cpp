//Ce programme est un jeu developper pour sur un carte mbed avec un ecran lcd tactile
//on controle un joueur positionné en bas de l'écran, ce joueur peut se déplacer de gauche a droite
//les mouvements sont enregistrer grace a acceleromèetre fixé sur la carte
//
//des ennemies tombent du haut de l'écran et le but est de les esquiver
//les ennemeis réapparaissent alétoirement sur l'axe y une fois passer le bas de l'écran
//la vitesse des ennemies augmente en fonction du score du joueur pour augmenter la difficulté 
//
//un buzzer rythme le jeu lors du début et la fin de la partie


#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>
#include <ctime>

#include <MPU6050.h>

#include <pwm_tone.h>

ThreadLvgl threadLvgl(30);

MPU6050 accelero;


//variable utilisé pour définir l'état du jeu dans un switch case
volatile char game_state = 's';


//structure utilisé pour le positionnement des élements mobile
struct position
{
   int x;
   int y;
};

//pin du buzzer
PwmOut Buzz(A0);


//melody_1 et melody_2 sont utilisé pour créer les musique avec le buzzer, chaque valeur dans le tableau corresponds a une note
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


//fonction d'évenement lors d'un appuis sur l'écran
void myEvent(lv_event_t *event){    
    if (game_state == 's') game_state = 'g';
    else if (game_state == 'l')game_state = 's';   
}


//fonction de verification du contact entre le joueur et un ennemie
bool contact (struct position enemy, struct position player);

//fonction de déplacement d'un ennemie
struct position enemy_move(struct position current_enemy, struct position scnd_enemy, struct position thrd_enemy, struct position frth_enemy, int enemy_speed);

//fonction pour jouer la mélodie
int play_melody ( int melody[], int position);


int main() {
    srand(time(NULL));
    float player_speed = 1.7;    
    char direction_p = 'l';
    char * message = "";
    char * no_msg = "";
    char * start_msg = "TAP TO PLAY";
    char * Game_over_msg = "GAME OVER";

//creation des objet
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
    

    //variables et fonction d'initialisation du MPU6050
    i2c.frequency(400000);
    accelero.resetMPU6050(); // Reset le registre en preparation pour sa calibration
    accelero.calibrateMPU6050(gyroBias, accelBias); // Calibration du gyro et de l'acceleromètre
    accelero.initMPU6050(); printf("MPU6050 initialized for active data mode....\n\r"); // Initialisation du MPU pour passer en mode de lecture actif

   //objet ligne de points lvgl pour former le joueur p
    static lv_point_t line_point_p[]={{0, 0},
                                    {0, 12},
                                    {12, 12},
                                    {12, 0},{0, 0}};

    //style de la a ligne de points du joueur
    static lv_style_t style_line_p;
    lv_style_init(&style_line_p);
    lv_style_set_line_width(&style_line_p, 8);
    lv_style_set_line_color(&style_line_p, lv_palette_main(LV_PALETTE_DEEP_PURPLE));
    lv_style_set_line_rounded(&style_line_p, true);

    //objet ligne de points lvgl pour les ennemies
    static lv_point_t line_point_e[]={{0, 0},
                                    {0, 18},
                                    {18, 18},
                                    {18, 0},{0, 0}};

    //style de la a ligne de points des ennemies
    static lv_style_t style_line_e;
    lv_style_init(&style_line_e);
    lv_style_set_line_width(&style_line_e, 10);
    lv_style_set_line_color(&style_line_e, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_line_rounded(&style_line_e, true);


    threadLvgl.lock();
    
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE); //desactivation de la fonction de scroll de l'écran

    //déclaration des objets lvgl
    lv_obj_t * obj_player;
    lv_obj_t * enemy1;
    lv_obj_t * enemy2;
    lv_obj_t * enemy3;
    lv_obj_t * enemy4;    
     
    lv_obj_t * label_score; 
    lv_obj_t * label_highest_scr;  
    lv_obj_t * label_message;

    //creation des objets et définition du style
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
        /////////////DEBUT DU JEU /////////////
        switch (game_state)
        {
                    //////////////START GAME MENU STATE//////////////
            /////EN ATTENTE DU PRESSION DU JOUEUR POUR DEBUTER LA PARTIE/////
            case 's':
                //clignotement du message du jeu 
                //lors de la première partie : "TAP TO PLAY"
                //a partir de la seconde partie : "GAME OVER"
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
               
                //fonction de déplacement des 4 ennemies qui retourne leurs nouveuax x et y
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

                //mouvement automatique du joueur en que est en attente
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
            //////////COMMENCEMENT DU JEU//////////
            case 'g':
                blink_state = false;// arret du message d'accueil
                message = no_msg;
                score +=2;
                //ici on va a chaque tour du switch case appeler la fonction play_melody() qui va jouer la note suivante de la melodie
                //quand toutes les notes seront jouer la condition n'est plus verifiée
                if(active_gs_melody_position !=9999)active_gs_melody_position=play_melody(melody_1, active_gs_melody_position);
                /////////////////////////////////////MOUVEMENT DE L'ENNEMI/////////////////////////////////////    
                //fonction de déplacement des 4 ennemies qui retourne leurs nouveuax x et y
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

                //gestion de la vitesse des joueur en focntion du score
                if ((score/10)%20 == 0 && score/10 < 300)enemies_speed +=0.1;
                if ((score/10)%500 == 0 && score/10 < 3000)enemies_speed +=0.1;
                //////////////////////////////////FIN MOUVEMENT DE L'ENNEMI//////////////////////////////////

                /////////////////////////////////////MOUVEMENT DU JOUEUR/////////////////////////////////////
                if(accelero.readByte(MPU6050_ADDRESS, INT_STATUS) & 0x01) {  // check if data ready interrupt
                    accelero.readAccelData(accelCount);  // lit le x,y et z de l'accelero
                    accelero.getAres();
                    
                    ay = (float)accelCount[1]*aRes; //recuperation du y dans la variable ay    

                    //gestion du mouvement du joueur vers la guache ou la droite
                    //si ay est superieure a 50 on va droite
                    //si ay est inférieure a -50 on va gauche
                    //on a une zone morte de 100 entre les deux et un acceleration plus on penche la carte
                    if(ay*1000 > 50){direction_p= 'r';player_speed = 1.3*ay*10;}
                    else if (ay*1000 < -50){direction_p= 'l';player_speed = 1.3*abs(ay)*10;}
                    else direction_p= 's';
                }

                //changement de position du joueur 
                switch (direction_p)
                {
                case 'l'://a gauche
                    if(player.y < 255)player.y = player.y + player_speed;
                    else  direction_p ='r';
                    break;
                
                case 'r'://a droite
                    if(player.y > 7)player.y = player.y - player_speed;
                    else direction_p ='l';
                    break;
                
                default:
                    break;
                }
                //////////////////////////////////FIN MOUVEMENT DU JOUEUR//////////////////////////////////

                /////////////////////////////////////VERIFICATION DE CONTACTE/////////////////////////////////////
                //on verifie le contact entre le joueur et les ennemies si vrai fin du jeu et GAME OVER
                if(contact(enemy_1, player) == true ||
                   contact(enemy_2, player) == true ||
                   contact(enemy_3, player) == true ||
                   contact(enemy_4, player) == true )game_state = 'l';               
                
                ///////////////////////////////////FIN VERIFICATION DE CONTACTE///////////////////////////////////
                break;

            //////////////LOST GAME STATE//////////////
            case 'l':
                    if (highest_scr< score)highest_scr=score;//sauvegarde du score si plus que le highscore
                    score = 0;

                    //on joue la musqiue de game over
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

                    //reinitialisation des variables et retour a l'état de départ
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
       

        
       //mise a jour des élements sur l'écran
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

//fonction de vérification de contact entre le joueur et les ennemies
//entrées: position de l'ennemie, position du joueur
//sortie: booléen -> true si contact, false si aucun contact
bool contact (struct position enemy, struct position player){
    if (enemy.x>player.x-27 && enemy.x<player.x+20){
        if(enemy.y>player.y-26 && enemy.y<player.y+20 ){
            return true;
        }else{return false;}
    }else{return false;}
}


//fonction du positionnement des ennemies
//entrées : la position de l'ennemie qui nous interesse, position de tous les autres ennemies, vitesse de déplacement des ennemies
//sortie : nouveaux x et y de l'ennemie
struct position enemy_move(struct position current_enemy, struct position scnd_enemy, struct position thrd_enemy, struct position frth_enemy, int enemy_speed){
    struct position new_position;
    if(current_enemy.x <500)current_enemy.x = current_enemy.x + enemy_speed; //si l'ennemie est toujours sur l'écran incrémentation de son x avec la vitesse de déplacement 
    else{ //sinon l'ennemie n'est plus sur l'écran et il doit être remit en haut de l'écran a un position aléatoire qui n'est pas la même que celle d'un autre ennemie
        current_enemy.x = rand() % -20 - 110;
        current_enemy.y = rand() % 245 + 10;
        while ((current_enemy.y>scnd_enemy.y-30 && current_enemy.y<scnd_enemy.y+30) || (current_enemy.y>thrd_enemy.y-30 && current_enemy.y<thrd_enemy.y+30) || (current_enemy.y>frth_enemy.y-30 && current_enemy.y<frth_enemy.y+30))
        {
            current_enemy.y = rand() % 245 + 10;
        }                    
    }  
    new_position.x = current_enemy.x;
    new_position.y = current_enemy.y;
    return new_position;//retourne la nouvelle position x et y
}

//fonction qui joue la melodie 
//entrées: tableau melody qui contient les notes a jouer, position du curseur dans le tableau
//sortie: la position du curseur incrémenté
int play_melody ( int melody[], int position){
    
    Buzz.period_us(melody[position]);//on joue la note dans la melody a la postion en cours
    Buzz.write(0.15f); // 15% duty cycle
    
    if (melody[position] >= 9999){//si on a atteint la fin du tableau la position devient 9999 pour signaler que la melodie est finie
        Buzz.write(0.0f); // Sound off        
        return position = 9999;
    }
    return position + 1;
}


