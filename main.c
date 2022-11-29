#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

void nokia_lcd_init(void);
void nokia_lcd_clear(void);
void nokia_lcd_set_cursor(uint8_t x, uint8_t y);
void nokia_lcd_write_string(const char *str, uint8_t scale);
void nokia_lcd_render(void);
void nokia_lcd_drawline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void nokia_lcd_drawrect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void nokia_lcd_drawcircle(uint8_t x1, uint8_t y1, uint8_t r);
void startScreen();
void victoryScreen();
void defeatScreen();
void drawEnemy();
void drawElements();
void resetElements();
void drawEnemy2();

// floor
uint8_t floor_x1 = 0;
uint8_t floor_y1 = 43;
uint8_t floor_x2 = 83;
uint8_t floor_y2 = 43;

// ceiling
uint8_t ceiling_x1 = 0;  
uint8_t ceiling_y1 = 12; 
uint8_t ceiling_x2 = 83;
uint8_t ceiling_y2 = 12;

// player
uint8_t player_lb = 7;  
uint8_t player_tb = 33; // cresce pra 0
uint8_t player_rb = 17;
uint8_t player_bb = 43; // chão do eixo y

// enemy
uint8_t enemy_lb = 76;
uint8_t enemy_tb = 33; // cresce pra 0
uint8_t enemy_rb = 80; 
uint8_t enemy_bb = 43; // chão do eixo y

// enemy2
uint8_t enemy2_lb = 76;
uint8_t enemy2_tb = 12; // cresce crescente
uint8_t enemy2_rb = 80; 
uint8_t enemy2_bb = 31;  //topo do eixo y

// other variables
uint8_t points = 0;
uint8_t jogando = 1;
uint8_t pressed = 0;
uint8_t pulo = 0;
uint8_t solo = 1;
uint8_t start = 0;
uint8_t reset = 0;
uint8_t randomV = 0;
uint8_t enemyReset = 0;
uint8_t enemyChoice = 1;

ISR(INT0_vect)
{
    if (!(PIND & (1 << PD2)))
    {   
        if(start == 1){
            pressed = 1;
        }
        if(reset == 0){
            start = 1;
        }
    }
}

int main()
{
    nokia_lcd_init();
    nokia_lcd_clear();

    // Definindo as saídas
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);

    // Definindo as entradas
    DDRD &= (0 << PD2);

    // Definindo as interrupções
    PCICR |= (1 << PCIE2); // habilita vetor de interrupção de todos os PDs
    PCMSK2 |= (1 << PCINT16); // habilita interrupção de PD0

    // Habilitando o pull-up (sempre passa 1)
    PORTD |= (1 << PD2);

    EICRA = 0b00000010; // interrupcao externa INT0 na borda de descida
    EIMSK = 0b00000001; // habilita a interrupcao externa INT0

    sei();
    
    // start screen loop

    // game loop
    while(jogando == 1){


        while(start == 0){
            startScreen();
            reset = 0;
        }
        // victory condition
        if(points == 10){
            pressed = 0;
            while(1){
                victoryScreen();
                if(pressed == 1){
                    start = 0;
                    reset = 1;
                    nokia_lcd_clear();
                    resetElements();
                    break;
                }
            }
        }

        else{
            // allowed to jump
            if(pressed == 1){
                pulo = 1;
                pressed = 0;
            }
            // up
            if(pulo == 1 && solo == 1){
                if(player_tb > 13){
                    player_tb -= 1;
                    player_bb -= 1;
                }
                else{
                    pulo = 0;
                    solo = 0;
                }
            // down
            }else{
                if(player_bb < 43){
                    // _delay_ms(10);
                    player_tb += 1;
                    player_bb += 1;
                }
                if(player_bb == 43){
                    solo = 1;
                }
            }
            
            // draw elements on screen
            _delay_ms(1);

            if(enemyChoice == 1){
                drawEnemy();
            }
            else{
                drawEnemy2();
            }

            if(enemyReset == 1 && (randomV%3) == 0){
                enemyChoice = 2;
            }
            else if(enemyReset == 1){
                enemyChoice = 1;
            }
            
            drawElements();
            // verify colision and defeat condition
            if((enemy_lb <= player_rb && enemy_tb <= player_bb && enemy_bb >= player_tb) || (enemy2_lb <= player_rb && enemy2_tb <= player_bb && enemy2_bb >= player_tb)){
                pressed = 0;
                while(1){
                    defeatScreen();
                    if(pressed == 1){
                        start = 0;
                        reset = 1;
                        nokia_lcd_clear();
                        resetElements();
                        break;
                    }
                }
            }
        }
        randomV++;
        if(randomV > 250){
            randomV = 0;
        }
    }

}

void resetElements(){
    player_lb = 7;  
    player_tb = 33;
    player_rb = 17;
    player_bb = 43; 
    enemy_lb = 76;
    enemy_tb = 33;
    enemy_rb = 80; 
    enemy_bb = 43;
    enemy2_lb = 76;
    enemy2_tb = 12;
    enemy2_rb = 80; 
    enemy2_bb = 31;
    points = 0;
    pressed = 0;
    solo = 1;
    pulo = 0;
}

void victoryScreen(){
    nokia_lcd_clear();
    nokia_lcd_set_cursor(20, 0);
    nokia_lcd_write_string("You Won!", 1);
    nokia_lcd_set_cursor(12, 12);
    char str[13];
    sprintf(str, "Score: %d", points);
    nokia_lcd_write_string(str, 1);
    nokia_lcd_set_cursor(12,28);
    nokia_lcd_write_string("Press w To", 1);
    nokia_lcd_set_cursor(12,40);
    nokia_lcd_write_string("Play Again", 1);
    nokia_lcd_render();
}

void startScreen()
{
    nokia_lcd_set_cursor(29, 0);
    nokia_lcd_write_string("Game", 1);
    nokia_lcd_set_cursor(22, 20);
    nokia_lcd_write_string("Press w", 1);
    nokia_lcd_set_cursor(18, 30);
    nokia_lcd_write_string("to start", 1);
    nokia_lcd_render();
}

void defeatScreen(){
    nokia_lcd_clear();
    nokia_lcd_set_cursor(15, 0);
    nokia_lcd_write_string("You Lost!", 1);
    nokia_lcd_set_cursor(15, 12);
    char str[13];
    sprintf(str, "Score: %d", points);
    nokia_lcd_write_string(str, 1);
    nokia_lcd_set_cursor(12,28);
    nokia_lcd_write_string("Press w To", 1);
    nokia_lcd_set_cursor(12,40);
    nokia_lcd_write_string("Play Again", 1);
    nokia_lcd_render();
}

void drawElements(){
    nokia_lcd_clear();
    nokia_lcd_drawline(floor_x1, floor_y1, floor_x2, floor_y2);
    nokia_lcd_drawline(ceiling_x1, ceiling_y1, ceiling_x2, ceiling_y2);
    nokia_lcd_drawrect(player_lb, player_tb, player_rb, player_bb);
    if(enemyChoice == 1){
        nokia_lcd_drawrect(enemy_lb, enemy_tb, enemy_rb, enemy_bb);
    }
    else{
        nokia_lcd_drawrect(enemy2_lb, enemy2_tb, enemy2_rb, enemy2_bb);
    }
    char str[13];
    sprintf(str, "points: %d", points);
    nokia_lcd_write_string(str, 1);
    nokia_lcd_render();
}

void drawEnemy(){
    enemy_lb -= 1;
    enemy_rb -= 1;
    enemyReset = 0;
    if(enemy_lb <= 0 || enemy_rb <= 0){
        enemy_lb = 80;
        enemy_rb = 76;
        points++;
        enemyReset = 1;
    }
}

void drawEnemy2(){
    enemy2_lb -= 1;
    enemy2_rb -= 1;
    enemyReset = 0;
    if(enemy2_lb <= 0 || enemy2_rb <= 0){
        enemy2_lb = 80;
        enemy2_rb = 76;
        points++;
        enemyReset = 1;
    }
}