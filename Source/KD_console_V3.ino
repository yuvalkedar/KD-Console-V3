/*
	year: 2018
	name: KD Console V3
	author: yuval_Kedar
	platform: Arduino nano.
	0.91" monochrome OLED display
	
	choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "bitmaps.h"

#define MOTOR_PIN		(3)
#define LEFT_BTN		(5)
#define RIGHT_BTN		(12)
#define SELECT_BTN		(6)
#define CONTROLLER_SIZE	(10)
#define SNAKE			(1)
#define PING_PONG		(2)
#define BRICKS			(3)
#define SPACE_INVADERS	(4)
#define TED_SHOW		(5)
#define RACE			(6)

#define PADDLE_SPEED	(5)
#define BALL_SPEED		(16)
#define PADDLE_HEIGHT	(10)
#define PLAYER_X		(115)
#define CPU_X			(12)

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

bool r_pressed;
bool l_pressed;
bool s_pressed;
unsigned long previous_millis = 0;	// will store last time motor was updated
bool motor_state = true;
uint8_t x_position = 5;
uint8_t current_selection = 0;
uint8_t menu_selected;

uint8_t ball_x = 64, ball_y = 15;
uint8_t ball_dir_x = 1, ball_dir_y = 1;
uint8_t cpu_y = 16;
uint8_t player_y = 16;
uint8_t player_score = 0;
uint8_t cpu_score = 0;

const char *string_list = 
	"snake\n"
	"ping-pong\n"
	"bricks\n"
	"space invaders\n"
	"ted show\n"
	"race";


void setup(){
	u8g2.begin(SELECT_BTN, RIGHT_BTN, LEFT_BTN, RIGHT_BTN, LEFT_BTN);
	u8g2.enableUTF8Print();
	Serial.begin(115200);
	Serial.println("KD console V3");
	init_controller();
	init_display();
	menu();
}

void loop(){
	menu_selected = u8x8_GetStringLineStart(current_selection - 1, string_list);
	if(menu_selected){
		switch(current_selection){
			case SNAKE:
				// Serial.println("SNAKE has been selected. Bon-apetit :)");
				snake_update();
				break;
			case RACE:
				// Serial.println("RACE has been selected. Go rout them!");
				race_update();
				break;
			case TED_SHOW:
				// Serial.println("TED-SHOW has been selected. Enjoy :)");
				ted_show_update();
				break;
			case SPACE_INVADERS:
				// Serial.println("SPACE-INVADERS has been selected. Let's kick some alien's A$$ :)");
				space_invaders_update();
				break;
			case BRICKS:
				// Serial.println("BRICKS has been selected. Enjoy :)");
				bricks_update();
				break;
			case PING_PONG:
				// Serial.println("PING-PONG has been selected. Enjoy :)");
				pong_update();
				break;
			default: menu();
		}
		Serial.println(current_selection);
	}
	controller_update();
}

//_________ DISPLAY ____________

void init_display(){
	u8g2.setFont(u8g2_font_6x10_tf);
	
	for(uint8_t i = 0; i < 6; i++){
		u8g2.clearBuffer();
		u8g2.drawXBMP(0, 0, KD_LOGO_W, KD_LOGO_H, kd_console1_bits);
		u8g2.sendBuffer();
		delay(500);
	
		u8g2.clearBuffer();
		u8g2.drawXBMP(0, 0, KD_LOGO_W, KD_LOGO_H, kd_console2_bits);
		u8g2.sendBuffer();
		delay(500);
	}
}

void new_game(){
	u8g2.clearDisplay();

	delay(200);
}

void player_movement(){
	u8g2.clearBuffer();
	if(r_pressed && x_position <= 118){
		x_position+=5;
	}
	if(l_pressed && x_position >= 1){
		x_position-=5;
	}
	for(uint8_t l = 0; l < CONTROLLER_SIZE; l++){
		u8g2.drawPixel(x_position + l, 30);
	}
	u8g2.sendBuffer();
}

void display_update(){
	player_movement();
}

//_________ CONTROLLER ____________

void init_controller(){
	pinMode(LEFT_BTN, INPUT_PULLUP);
	pinMode(RIGHT_BTN, INPUT_PULLUP);
	pinMode(SELECT_BTN, INPUT_PULLUP);
	pinMode(MOTOR_PIN, OUTPUT);
}

void controller_update(){
	r_pressed = !digitalRead(RIGHT_BTN);
	l_pressed = !digitalRead(LEFT_BTN);
	s_pressed = !digitalRead(SELECT_BTN);
	
	// digitalWrite(13, r_pressed || l_pressed || s_pressed);
}

void vibrate(){
	unsigned long current_millis = millis();
	// previous_millis = current_millis;	// save the last time motor has blinked	
	Serial.println(current_millis);
	if(current_millis - previous_millis >= 100){
		motor_state = false;
	}
	digitalWrite(MOTOR_PIN, motor_state);
}

//_________ MENU ____________

void menu(){
	current_selection = u8g2.userInterfaceSelectionList("GAMES", current_selection, string_list);
	// u8g2.userInterfaceMessage(NULL, u8x8_GetStringLineStart(current_selection - 1, string_list), "", " ok \n cancel ");
}

//_________ PONG______________

void pong_update(){
	u8g2.clearBuffer();

	static bool up_state = false;
	static bool down_state = false;

	up_state |= (digitalRead(RIGHT_BTN) == LOW);
	down_state |= (digitalRead(LEFT_BTN) == LOW);

	uint8_t new_x = ball_x + ball_dir_x;
	uint8_t new_y = ball_y + ball_dir_y;
	
	// Check if we hit the vertical walls
	if(new_x == 0 || new_x == 127){
		ball_dir_x = -ball_dir_x;
		new_x += ball_dir_x;
		// vibrate();
		if(new_x > PLAYER_X) cpu_score++;
		if(new_x < CPU_X) player_score++;
	}
	u8g2.setCursor(1, 20);
	u8g2.print(cpu_score);
	u8g2.setCursor(123, 20);
	u8g2.print(player_score);
	
	// Check if we hit the horizontal walls.
	if(new_y == 0 || new_y == 32){
		ball_dir_y = -ball_dir_y;
		new_y += ball_dir_y;
	}
	
	// Check if we hit the CPU paddle
	if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT){
		ball_dir_x = -ball_dir_x;
		new_x += ball_dir_x;
	}
	
	// Check if we hit the player paddle
	if(new_x == PLAYER_X && new_y >= player_y && new_y <= player_y + PADDLE_HEIGHT){
		ball_dir_x = -ball_dir_x;
		new_x += ball_dir_x;
	}
	
	u8g2.drawPixel(ball_x, ball_y);	//black
	u8g2.drawPixel(new_x, new_y);	//white
	ball_x = new_x;
	ball_y = new_y;
	
	
	// CPU paddle
	u8g2.drawVLine(CPU_X, cpu_y, PADDLE_HEIGHT);	//black
	const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
	if(cpu_y + half_paddle > ball_y) cpu_y -= 1;
	if(cpu_y + half_paddle < ball_y) cpu_y += 1;
	if(cpu_y < 1) cpu_y = 1;
	if(cpu_y + PADDLE_HEIGHT > 32) cpu_y = 32 - PADDLE_HEIGHT;
	
	u8g2.drawVLine(CPU_X, cpu_y, PADDLE_HEIGHT);	//white
	
	// Player paddle
	u8g2.drawVLine(PLAYER_X, player_y, PADDLE_HEIGHT);	//black
	if(up_state) player_y -= 1;
	if(down_state) player_y += 1;
	
	up_state = down_state = false;
	if(player_y < 1) player_y = 1;
	if(player_y + PADDLE_HEIGHT > 32) player_y = 32 - PADDLE_HEIGHT;
	
	u8g2.drawVLine(PLAYER_X, player_y, PADDLE_HEIGHT);	//white
	
	u8g2.sendBuffer();
}

//_________ SNAKE______________

void snake_update(){
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}

//_________ RACE______________

void race_update(){
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}

//_________ BRICKS______________

void bricks_update(){
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}

//_________ TED-SHOW______________

void ted_show_update(){
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}

//_________ SPACE-INVADERS______________

void space_invaders_update(){
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}
