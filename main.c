/*
 * I am making a game based off of the Disney movie Frozen.
 * I have a state system that can switch between Menu,
 *  Instructions, Game, Pause, Win, and Lose states.
 * I have my Elsa sprite animated as well as an enemy
 *  sprite and an Anna sprite.
 * My game has background music for both the menu states and in the game state
 *  as well as three simultaneous backgrounds in the game state. 
 * When you push the A button, Elsa attacks the enemy by freezing him and there is 
 *  an attack sound. Also, snowflakes appear as the attack. 
 * You win the game by reaching Princess Anna after freezing the snow creatures.
 * You lose the game if a snow creature touches you before you freeze it.
 * You can cheat by pushing the B button and freezing all of the snow creatures at once.
 * 
 */

#include "myLib.h"
#include "labTiles-2.h"
#include "Vuelie.h"
#include "Wind.h"
#include "winScreen.h"
#include "startScreen.h"
#include "pauseScreen.h"
#include "loseScreen.h"
#include "FrozenSnow-2.h"
#include "FrozenSprites-3.h"
#include "FrozenTiles.h"
#include "instrScreen.h"
#include "FrozenHills.h"
#include "SummitSiege.h"

unsigned int buttons;
unsigned int oldButtons;

int hOffZero;
int vOffZero;
int hOffOne;
int vOffOne;

#define STARTSCREEN 0
#define GAMESCREEN 1
#define LOSESCREEN 2
#define WINSCREEN 3
#define PAUSESCREEN 4
#define INSTRSCREEN 5

#define BLACKINDEX 0
#define REDINDEX 1
#define BLUEINDEX 2
#define GREENINDEX 3
#define WHITEINDEX 4

OBJ_ATTR shadowOAM[128];

int bigRow, bigCol;
int collMapSize = 256;

#define ROWMASK 0xFF
#define COLMASK 0x1FF
typedef struct {
	int row;
	int col;
        int rdel;
        int cdel;
	int width;
        int height;
        int aniCounter;
        int aniState;
        int prevAniState;
        int currFrame;
        int spriteRow;
        int spriteCol;
} MOVOBJ2;

MOVOBJ2 elsa, anna, attack;
MOVOBJ2 snowball[10];

void hideSprites();

enum {ELSARIGHT, ELSAIDLE};

typedef struct{
    unsigned char* data;
    int length;
    int frequency;
    int isPlaying;
    int loops;
    int duration;
}SOUND;

SOUND soundA;
SOUND soundB;
int vbCountA;
int vbCountB;

int state;
int flip;
int frozen;

int place;
int frozencol[10];

//Prototypes
void setupSounds();
void playSoundA( const unsigned char* sound, int length, int frequency);
void playSoundB( const unsigned char* sound, int length, int frequency);
void setupInterrupts();
void interruptHandler();

int main()
{
	setupInterrupts();
        setupSounds();
    
	hOffZero = 0;
	vOffZero = 0;
        hOffOne = 0;
        vOffOne = 0;
        flip = 0;

	buttons = BUTTONS;
        
        REG_DISPCTL = MODE3 | BG2_ENABLE;
        state = STARTSCREEN;
        drawImage3(startScreenBitmap, 0, 0, 160, 240);
        playSoundA(Vuelie,VUELIELEN,VUELIEFREQ);
        
	while(1)
	{
            oldButtons = buttons;
	    buttons = BUTTONS;
            switch(state){
                case STARTSCREEN:
                    start();
                    break;
                case GAMESCREEN:
                    game();
                    break;
                case PAUSESCREEN:
                    pause();
                    break;
                case LOSESCREEN:
                    lose();
                    break;
                case WINSCREEN:
                    win();
                    break;
                case INSTRSCREEN:
                    instr();
                    break;
                    
            }
            
        }
	return 0;
}

void game(){

        elsa.prevAniState = elsa.aniState;
        elsa.aniState = ELSAIDLE;
        int snow = 0;
                
         if(elsa.aniCounter%20==0){
                  elsa.currFrame++;
                  elsa.currFrame = (elsa.currFrame) % 2;
        }
        elsa.spriteCol = (elsa.currFrame * 2);
                
        bigRow = elsa.row + vOffZero;
        bigCol = elsa.col + hOffZero;

        if(BUTTON_HELD(BUTTON_UP)){
           elsa.aniState = ELSAIDLE;
           elsa.spriteRow = 0;
           elsa.spriteCol = 0;
           
           //vOffZero--;
        }
	if(BUTTON_HELD(BUTTON_DOWN)){
             elsa.aniState = ELSAIDLE;
             elsa.spriteRow = 0;
             elsa.spriteCol = 0;
             
             //vOffZero++;
	}
	if(BUTTON_HELD(BUTTON_LEFT)){
		hOffZero--;
                if(frozen){hOffOne--;}
                elsa.aniState = ELSARIGHT;
                //elsa.spriteRow = 0;
                //elsa.spriteCol = 0;
                //shadowOAM[0].attr1 = (COLMASK & elsa.col) | ATTR1_SIZE32 | ATTR1_HFLIP;
                flip = 1;
	}
	if(BUTTON_HELD(BUTTON_RIGHT)){ 
		hOffZero++;
                if(frozen){hOffOne++;}
                elsa.aniState = ELSARIGHT;
                flip = 0;
                //shadowOAM[0].attr1 = (COLMASK & elsa.col) | ATTR1_SIZE32;
                //elsa.spriteRow = 0;
                //elsa.spriteCol = 0;
	}

        if(elsa.aniState == ELSAIDLE){
                 elsa.aniState = elsa.prevAniState;
                 elsa.currFrame = 0;
        }
        else{
                 elsa.aniCounter++;
        }
            
        int s;
        
        for(s = 0; s < 10; s++){
            if(snowball[s].spriteCol < 9){
                snowball[s].col = (place - hOffZero) + (200 * s);
            }
            else{
                snowball[s].col = (frozencol[s] - hOffZero);
            }
                
        }
        for(s = 0; s < 10; s++){
            if(snowball[s].col < 240){
                snowball[s].row = 110 - snowball[s].height;
            }
            if(snowball[s].col <= -30){
                snowball[s].row = 200;
            }
        }
        place--;
        if(hOffZero > 800){
            anna.col  = 520 - hOffZero;
        }
        //anna.col = 400 - hOffZero;
                
        shadowOAM[0].attr0 = (ROWMASK & elsa.row) | ATTR0_4BPP | ATTR0_TALL;
        if(flip){shadowOAM[0].attr1 = (COLMASK & elsa.col) | ATTR1_SIZE32 | ATTR1_HFLIP;}
	else{shadowOAM[0].attr1 = (COLMASK & elsa.col) | ATTR1_SIZE32;}
	shadowOAM[0].attr2 = SPRITEOFFSET16(elsa.spriteRow, elsa.spriteCol);
        
        shadowOAM[1].attr0 = (ROWMASK & anna.row) | ATTR0_4BPP | ATTR0_TALL;
        shadowOAM[1].attr1 = (COLMASK & anna.col) | ATTR1_SIZE32 | ATTR1_HFLIP;
	shadowOAM[1].attr2 = SPRITEOFFSET16(anna.spriteRow, anna.spriteCol);
        
        shadowOAM[2].attr0 = (ROWMASK & attack.row) | ATTR0_4BPP | ATTR0_WIDE;
        if(!flip){shadowOAM[2].attr1 = (COLMASK & attack.col) | ATTR1_SIZE8;}
        else{shadowOAM[2].attr1 = (COLMASK & (attack.col - 27)) | ATTR1_SIZE8 | ATTR1_HFLIP;}
	shadowOAM[2].attr2 = SPRITEOFFSET16(attack.spriteRow, attack.spriteCol);
        
        for(s = 0; s < 10; s++){
                shadowOAM[(s+3)].attr0 = (ROWMASK & snowball[s].row) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[(s+3)].attr1 = (COLMASK & snowball[s].col) | ATTR1_SIZE32;
                shadowOAM[(s+3)].attr2 = SPRITEOFFSET16(snowball[s].spriteCol, snowball[s].spriteRow);
        }
        
        if(!frozen){
            hOffOne--;
            vOffOne--;
        }
                
        REG_BG1HOFS = hOffZero;
        REG_BG1VOFS = vOffZero;
        REG_BG0HOFS = hOffOne;
        REG_BG0VOFS = vOffOne;
        REG_BG2VOFS = 30;
                
        if(hOffZero >= 900){
               state = WINSCREEN;
               stopSounds();
        
               REG_DISPCTL = MODE3 | BG2_ENABLE;
               drawImage3(winScreenBitmap, 0, 0, 160, 240); 
               playSoundA(Vuelie,VUELIELEN,VUELIEFREQ);
        }
                
	waitForVblank();
        DMANow(3, shadowOAM, OAM, 128 * 4);
                
    if(BUTTON_PRESSED(BUTTON_START)){
        muteSound();
        state = PAUSESCREEN;
        
        REG_DISPCTL = MODE3 | BG2_ENABLE;
        drawImage3(pauseScreenBitmap, 0, 0, 160, 240);
    
     }
        
        if(BUTTON_PRESSED(BUTTON_A)){
            playSoundB(Wind, WINDLEN, WINDFREQ);
            //attack.row = 90;
            snow = 30;
            
            //elsa.spriteCol = 8;
            for(s = 0; s < 10; s++){
                if(elsa.col >= (snowball[s].col - 30)){
                    snowball[s].spriteCol = 9;
                    frozencol[s] = (snowball[s].col + hOffZero);
                }
            }
        }
        
        if(snow > 0){
            attack.row = 90;
            //elsa.spriteCol = 8;
            //snow--;
        }
        else{
            attack.row = -10;
            //elsa.spriteCol = 0;
            snow--;
        }
        
        
        if(BUTTON_PRESSED(BUTTON_B)){
            playSoundB(Wind, WINDLEN, WINDFREQ);
            attack.row = 90;
            frozen = 1;
            //elsa.spriteCol = 8;
            for(s = 0; s < 10; s++){
                //if(elsa.col >= (snowball[s].col - 30)){
                    snowball[s].spriteCol = 9;
                    frozencol[s] = (snowball[s].col + hOffZero);
                //}
            }
        }
        
        for(s = 0; s < 10; s++){
            if((elsa.col >= (snowball[s].col - 12)) & (snowball[s].spriteCol < 9)){
                state = LOSESCREEN;
                stopSounds();
                
                REG_DISPCTL = MODE3 | BG2_ENABLE;
                drawImage3(loseScreenBitmap, 0, 0, 160, 240);
                playSoundA(Vuelie,VUELIELEN,VUELIEFREQ);
            }
        }
        
        //waitForVblank();
        //snow--;
}

void start(){
    if(BUTTON_PRESSED(BUTTON_START)){
        state = GAMESCREEN;
        stopSounds();
        playSoundA(SummitSiege,SUMMITSIEGELEN,SUMMITSIEGEFREQ);
        place = 240;
        frozen = 0;
        
        REG_DISPCTL = MODE0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | SPRITE_ENABLE;
        REG_BG0CNT = BG_SIZE0 | SBB(31) | CBB(0) | COLOR256;
        //REG_BG1CNT = BG_SIZE1 | SBB(30) | CBB(1) | COLOR256;
        REG_BG1CNT = BG_SIZE1 | SBB(29) | CBB(1) | COLOR256;
        REG_BG2CNT = BG_SIZE0 | SBB(28) | CBB(2) | COLOR256;
        loadPalette(FrozenTilesPal);

	DMANow(3, FrozenTilesTiles, &CHARBLOCKBASE[1], FrozenTilesTilesLen/2);
        //DMANow(3, FrozenTilesMap, &SCREENBLOCKBASE[30], 2048);
        DMANow(3, FrozenTilesMap, &SCREENBLOCKBASE[29], 2048);
        
        DMANow(3, FrozenSnowTiles, &CHARBLOCKBASE[0], FrozenSnowTilesLen/2);
        DMANow(3, FrozenSnowMap, &SCREENBLOCKBASE[31], 1024);
        
        DMANow(3, FrozenHillsTiles, &CHARBLOCKBASE[2], FrozenHillsTilesLen/2);
        DMANow(3, FrozenHillsMap, &SCREENBLOCKBASE[28], 1024);
        
        DMANow(3, (unsigned int*) FrozenSpritesTiles, &CHARBLOCKBASE[4], FrozenSpritesTilesLen/2);
        DMANow(3, (unsigned int*) FrozenSpritesPal, SPRITE_PALETTE, 256);
         
        hideSprites();
        initialize();
    }
    
    if(BUTTON_PRESSED(BUTTON_SELECT)){
        state = INSTRSCREEN;
        
        drawImage3(instrScreenBitmap, 0, 0, 160, 240);
    }
}

void pause(){
    if(BUTTON_PRESSED(BUTTON_START)){
        state = GAMESCREEN;
        unmuteSound();
        
        REG_DISPCTL = MODE0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | SPRITE_ENABLE;
        REG_BG0CNT = BG_SIZE0 | SBB(31) | CBB(0) | COLOR256;
        //REG_BG1CNT = BG_SIZE1 | SBB(30) | CBB(1) | COLOR256;
        REG_BG1CNT = BG_SIZE1 | SBB(29) | CBB(1) | COLOR256;
        REG_BG2CNT = BG_SIZE0 | SBB(28) | CBB(2) | COLOR256;
        loadPalette(FrozenTilesPal);

	DMANow(3, FrozenTilesTiles, &CHARBLOCKBASE[1], FrozenTilesTilesLen/2);
        //DMANow(3, FrozenTilesMap, &SCREENBLOCKBASE[30], 2048);
        DMANow(3, FrozenTilesMap, &SCREENBLOCKBASE[29], 2048);
        
        DMANow(3, FrozenSnowTiles, &CHARBLOCKBASE[0], FrozenSnowTilesLen/2);
        DMANow(3, FrozenSnowMap, &SCREENBLOCKBASE[31], 1024);
        
        DMANow(3, FrozenHillsTiles, &CHARBLOCKBASE[2], FrozenHillsTilesLen/2);
        DMANow(3, FrozenHillsMap, &SCREENBLOCKBASE[28], 1024);
        
        DMANow(3, (unsigned int*) FrozenSpritesTiles, &CHARBLOCKBASE[4], FrozenSpritesTilesLen/2);
        DMANow(3, (unsigned int*) FrozenSpritesPal, SPRITE_PALETTE, 256);
        
        //hideSprites();
        //initialize();
    }
    
    if(BUTTON_PRESSED(BUTTON_SELECT)){
        state = STARTSCREEN;
        drawImage3(startScreenBitmap, 0, 0, 160, 240);
        stopSounds();
        playSoundA(Vuelie,VUELIELEN,VUELIEFREQ);
    }
}

void win(){
    if(BUTTON_PRESSED(BUTTON_START)){
        state = STARTSCREEN;
        drawImage3(startScreenBitmap, 0, 0, 160, 240);
        
    }
}

void lose(){
   if(BUTTON_PRESSED(BUTTON_START)){
        state = STARTSCREEN;
        drawImage3(startScreenBitmap, 0, 0, 160, 240);
        
    }
}

void instr(){
    if(BUTTON_PRESSED(BUTTON_START)){
        state = STARTSCREEN;
        drawImage3(startScreenBitmap, 0, 0, 160, 240);
    }
}

void setupSounds(){
        REG_SOUNDCNT_X = SND_ENABLED;

	REG_SOUNDCNT_H = SND_OUTPUT_RATIO_100 | 
                        DSA_OUTPUT_RATIO_100 | 
                        DSA_OUTPUT_TO_BOTH | 
                        DSA_TIMER0 | 
                        DSA_FIFO_RESET |
                        DSB_OUTPUT_RATIO_100 | 
                        DSB_OUTPUT_TO_BOTH | 
                        DSB_TIMER1 | 
                        DSB_FIFO_RESET;

	REG_SOUNDCNT_L = 0;
}

void playSoundA( const unsigned char* sound, int length, int frequency) {

    soundA.data = sound;
    soundA.length = length;
    soundA.frequency = frequency;
    soundA.duration = ((60*length)/frequency) - ((length/frequency)*3)-1;
    soundA.loops = 1;
    soundA.isPlaying = 1;
    
        dma[1].cnt = 0;
        vbCountA = 0;
	
        int interval = 16777216/frequency;
	
        if(soundA.isPlaying){
                DMANow(1, sound, REG_FIFO_A, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);
        }
	
        REG_TM0CNT = 0;
	
        REG_TM0D = -interval;
        REG_TM0CNT = TIMER_ON;
}

void playSoundB( const unsigned char* sound, int length, int frequency) {

    soundB.length = length;
    soundB.frequency = frequency;
    soundB.duration = ((60*length)/frequency) - ((length/frequency)*3)-1;
    soundB.loops = 0;
    
        dma[2].cnt = 0;
        vbCountB = 0;

        int interval = 16777216/frequency;

        DMANow(2, sound, REG_FIFO_B, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);

        REG_TM1CNT = 0;
	
        REG_TM1D = -interval;
        REG_TM1CNT = TIMER_ON;
	
}

void setupInterrupts(){
	REG_IME = 0;
	REG_INTERRUPT = (unsigned int)interruptHandler;
	REG_IE |= INT_VBLANK;
	REG_DISPSTAT |= INT_VBLANK_ENABLE;
	REG_IME = 1;
}

void interruptHandler(){
    
	REG_IME = 0;
	if(REG_IF & INT_VBLANK)
	{
            if(soundA.isPlaying){
                vbCountA++;
            }
            
                vbCountB++;
            
            if(vbCountA > soundA.duration){
                REG_TM0CNT = 0;
                soundA.isPlaying = 0;
                if(soundA.loops){
                    playSoundA(soundA.data, soundA.length, soundA.frequency);
                }
            }
            if(vbCountB > soundB.duration){
                REG_TM1CNT = 0;
                soundB.isPlaying = 0;
                if(soundB.loops){
                    playSoundB(soundB.data, soundB.length, soundB.frequency);
                }
            }
            
		REG_IF = INT_VBLANK; 
	}

	REG_IME = 1;
}

void muteSound(){
    REG_SOUNDCNT_H &= ~DSA_OUTPUT_TO_BOTH;
    REG_SOUNDCNT_H &= ~DSB_OUTPUT_TO_BOTH;
}

void unmuteSound(){
    REG_SOUNDCNT_H |= DSA_OUTPUT_TO_BOTH;
    REG_SOUNDCNT_H |= DSB_OUTPUT_TO_BOTH;
}

void stopSounds(){
    soundA.isPlaying = 0;
    soundB.isPlaying = 0;
    setupSounds();
    REG_TM0CNT = 0;
    REG_TM0CNT &= ~ TIMER_ON;
}

void hideSprites(){
    int i;
    for(i=1; i<128; i++) {
    	shadowOAM[i].attr0 = ATTR0_HIDE;
    }
}

void initialize(){
        //ELSA:
        elsa.width = 16;
        elsa.height = 32;
        elsa.rdel = 1;
        elsa.cdel = 1;
	elsa.row = 110 - elsa.height;
	elsa.col = 240/2-elsa.width/2;
        elsa.spriteCol = 0;
        elsa.spriteRow = 0;
        elsa.aniCounter = 0;
        elsa.currFrame = 0;
        elsa.aniState = ELSAIDLE; 
        
        //SNOWBALL:
        int s;
        for(s = 0; s < 10; s++){
            snowball[s].width = 32;
            snowball[s].height = 24;
            snowball[s].rdel = 1;
            snowball[s].cdel = 1;
            snowball[s].row = -30;
            snowball[s].col = 190;
            snowball[s].spriteCol = 5;
            snowball[s].spriteRow = 0;
            snowball[s].aniCounter = 0;
            snowball[s].currFrame = 0;
        }
        
        //ANNA:
        anna.width = 16;
        anna.height = 32;
        anna.rdel = 1;
        anna.cdel = 1;
	anna.row = 110 - anna.height;
	anna.col = 300;
        anna.spriteCol = 4;
        anna.spriteRow = 0;
        anna.aniCounter = 0;
        anna.currFrame = 0;
        
        //SNOW ATTACK:
        attack.width = 32;
        attack.height = 16;
        attack.rdel = 1;
        attack.cdel = 1;
	attack.row = -10;
	attack.col = 125;
        attack.spriteCol = 0;
        attack.spriteRow = 4;
        attack.aniCounter = 0;
        attack.currFrame = 0;
        
        buttons = BUTTONS;
        
	hOffZero = 0;
	vOffZero = 35;

}//Initialize Sprites