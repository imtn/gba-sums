// Main file
// Name: Niandong Xu

#include <stdlib.h> //for randomness and absolute value
#include <stdio.h> //for sprintf
#include "TitleScreen.h"
#include "FinishScreen.h"
#include "pieces.h"
#include "font.h"
#include "myLib.h"


int main() {

	//set REG_DISPCNT
	REG_DISPCNT = (MODE_3 | BG2_EN);
	GBASTATE state = TITLESTATE;
	const u32 maxDie = 6;
	volatile dieObj dice[maxDie];
	volatile u32 bucketsFilled = 0;
	volatile u32 bucketsToWin = 3;
	volatile u32 currScore = 1;
	volatile u32 totalScore = 0;
	volatile u32 leavePlay = 0; //0 if continue playing, 1 to go to finish state
	volatile u32 getNewDice = 0; //0 if no need to get new dice, 1 if need to get new dice
	volatile u32 iterToDiceRefresh = 240; //how many iterations of the while loop until we refresh die
	volatile u32 iterCounter = 0; //
	player initialBucket = {115, 130, 10, 10, 0, 15, DIRECTION_NONE, Bucket};
	player bucket = initialBucket;
	char blank[2] = {' ', ' '};

	while(1) {
		iterCounter++;

		if (KEY_DOWN_NOW(BUTTON_SELECT)) {
			//Reset Game State
			currScore = 1;
			totalScore = 0;
			bucketsFilled = 0;
			leavePlay = 0;
			getNewDice = 0;
			iterCounter = 0;
			bucket = initialBucket;
			state = TITLESTATE;
		}
		else if (KEY_DOWN_NOW(BUTTON_A) && state == TITLESTATE) {//move from title screen to playing screen
			//create random dice
			for (unsigned int i = 0; i < maxDie; i++) { //populate dice array
				volatile u32 xPosCheck, yPosCheck;
				do {
					volatile u32 tempNum = rand() % 7;
					dieObj temp = {
						rand() % 230, //xpos
						rand() % 130, //ypos
						possibleDie[tempNum].num,
						possibleDie[tempNum].width,
						possibleDie[tempNum].height,
						possibleDie[tempNum].image,
					};
					xPosCheck = temp.xPos;
					yPosCheck = temp.yPos;
					dice[i] = temp;
				} while (abs(bucket.xPos - xPosCheck) <= bucket.width
							&& abs(bucket.yPos - yPosCheck) <= bucket.height); //continue finding a new die while the current one collides with die in currect position
			}

			//erase title screen
			waitForVblank();
			drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
			for (unsigned int i = 0; i < maxDie; i++) {//draw new dice
				drawImage3(dice[i].xPos, dice[i].yPos, dice[i].width, dice[i].height, dice[i].image);
			}
			drawImage3(bucket.xPos, bucket.yPos, bucket.width, bucket.height, bucket.image);

			state = PLAYSTATE;
		}

		switch (state) {
			case TITLESTATE:
				waitForVblank();
				drawImage3(0, 0, TITLESCREEN_WIDTH, TITLESCREEN_HEIGHT, TitleScreen);
				break;

			case PLAYSTATE:
				//get movement, if any, and update player position
				if (KEY_DOWN_NOW(BUTTON_UP)
					|| KEY_DOWN_NOW(BUTTON_LEFT)
					|| KEY_DOWN_NOW(BUTTON_RIGHT)
					|| KEY_DOWN_NOW(BUTTON_DOWN)) {
					if (KEY_DOWN_NOW(BUTTON_UP) && bucket.yPos > 0) {
						bucket.yPos--;
						bucket.direction = DIRECTION_UP;
					} else if (KEY_DOWN_NOW(BUTTON_DOWN) && bucket.yPos < 130) {
						bucket.yPos++;
						bucket.direction = DIRECTION_DOWN;
					} else if (KEY_DOWN_NOW(BUTTON_LEFT) && bucket.xPos > 0) {
						bucket.xPos--;
						bucket.direction = DIRECTION_LEFT;
					} else if (KEY_DOWN_NOW(BUTTON_RIGHT) && bucket.xPos < 230) {
						bucket.xPos++;
						bucket.direction = DIRECTION_RIGHT;
					}
				} else {
					bucket.direction = DIRECTION_NONE;
				}

				//Collision check code
				for (unsigned int i = 0; i < maxDie; i++) { //check each dice for collision with bucket
						dieObj currDie = dice[i];
						if (abs(bucket.xPos - currDie.xPos) <= bucket.width
							&& abs(bucket.yPos - currDie.yPos) <= bucket.height) {
							//if collide
							if (currDie.num < (bucket.max-bucket.sum)) {
								//if underflow
								bucket.sum += currDie.num; //increase sum
								currScore *= currDie.num;

								getNewDice = 1;

								break;
							} else if (currDie.num == (bucket.max-bucket.sum)) {
								//if equal
								//get rid of the die, update currScore into total score
								currScore *= currDie.num;
								totalScore += currScore;
								currScore = 1;

								getNewDice = 1;
								//get new bucket with random max and 0 sum
								bucketsFilled++;
								if (bucketsFilled == bucketsToWin) {
									state = FINISHSTATE;
									leavePlay = 1;
									waitForVblank();
									drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
								} else {
									//make new random bucket.
									bucket.sum = 0;
									bucket.max = (rand()%20)+15; // 15 to 35, inclusive
									break;
								}
							} else { // currDie.num > (bucket.max-bucket.sum)
								//if overflow
								//lose, immediately go to finish screen
								state = FINISHSTATE;
								leavePlay = 1;
								waitForVblank();
								drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
								break;
							} //anything else is error
						}
					}

				char sumStr[10];
				char maxStr[10];
				char bucketsFilledStr[10];
				sprintf(sumStr, "%d", bucket.sum); //Print these as information
				sprintf(maxStr, "%d", bucket.max);
				sprintf(bucketsFilledStr, "%d", bucketsFilled);

				if (!leavePlay) {
					waitForVblank();

					if(getNewDice == 1 || iterCounter >= iterToDiceRefresh) {
						for (unsigned int i = 0; i < maxDie; i++) {//delete old dice
							drawRect(dice[i].xPos, dice[i].yPos, dice[i].width, dice[i].height, BLACK); //erase old die
						}

						for (unsigned int i = 0; i < maxDie; i++) {
							//populate dice array
							volatile u32 xPosCheck, yPosCheck;
							do {
								volatile u32 tempNum = rand() % 7;
								dieObj temp = {
									rand() % 230, //xpos
									rand() % 130, //ypos
									possibleDie[tempNum].num,
									possibleDie[tempNum].width,
									possibleDie[tempNum].height,
									possibleDie[tempNum].image,
								};
								xPosCheck = temp.xPos;
								yPosCheck = temp.yPos;
								dice[i] = temp;
							} while (abs(bucket.xPos - xPosCheck) <= bucket.width
										&& abs(bucket.yPos - yPosCheck) <= bucket.height); //continue finding a new die while the current one collides with die in currect position
						}


						for (unsigned int i = 0; i < maxDie; i++) {//draw new dice
							drawImage3(dice[i].xPos, dice[i].yPos, dice[i].width, dice[i].height, dice[i].image);
						}
						getNewDice = 0;
						iterCounter = 0;
					}

					switch (bucket.direction) {
						case DIRECTION_UP:
							drawRect(bucket.xPos, bucket.yPos+bucket.height, bucket.width, 1, BLACK);
							break;
						case DIRECTION_RIGHT:
							drawRect(bucket.xPos-1, bucket.yPos, 1, bucket.height, BLACK);
							break;
						case DIRECTION_DOWN:
							drawRect(bucket.xPos, bucket.yPos-1, bucket.width, 1, BLACK);
							break;
						case DIRECTION_LEFT:
							drawRect(bucket.xPos+bucket.width, bucket.yPos, 1, bucket.height, BLACK);
							break;
						case DIRECTION_NONE:
							//Didn't move, nothing to erase
							break;
						default:
							//Should never get here
							break;
					}
					drawImage3(bucket.xPos, bucket.yPos, bucket.width, bucket.height, bucket.image);//draw updated bucket

					drawString(6*0, 146, "SUM:", WHITE);
					drawString(6*4, 146, blank, WHITE);
					drawString(6*4, 146, sumStr, WHITE);
					drawString(6*7, 146, "GOAL:", WHITE);
					drawString(6*12, 146, maxStr, WHITE);
					drawString(6*15, 146, "Filled:", WHITE);
					drawString(6*22, 146, bucketsFilledStr, WHITE);
				}
				break;

			case FINISHSTATE:
			{
				char score[10];
				sprintf(score, "%d", totalScore);
				char winMessage[35] = "A winner is you! Your score is ";
				char loseMessage[35] = "You lost. Your score was ";
				char retry[35] = "Press Select to play again";
				
				//use text to write the score.
				waitForVblank();
				drawImage3(0, SCREEN_HEIGHT/2, FINISHSCREEN_WIDTH, FINISHSCREEN_HEIGHT, FinishScreen);
				if (bucketsFilled == bucketsToWin) {
					drawString(0, 0, winMessage, WHITE);
					drawString(31*6, 0, score, WHITE);
				} else if (bucketsFilled < bucketsToWin) {
					drawString(0, 0, loseMessage, WHITE);
					drawString(25*6, 0, score, WHITE);
				}
				drawString(0, 10, retry, WHITE);
				break;
			}

			default:
				//Nothing. If we enter here there had been an error.
				break;
		}
	}
	return 0;
}

void setPixel(int x, int y, unsigned short color)
{
	videoBuffer[OFFSET(x,y, 240)] = color;
}

void waitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

void drawRect(int x, int y, int width, int height, volatile unsigned short color)
{
	for(int i=0; i<height; i++)
	{
		REG_DMA3SAD = (u32)&color;
		REG_DMA3DAD = (u32)(&videoBuffer[OFFSET(x, y+i, 240)]);
		REG_DMA3CNT = width | DMA_ON | DMA_SOURCE_FIXED;
	}
}

void drawImage3(int x, int y, int width, int height, const unsigned short *image) {

	// TODO Draw the image with the top left corner at (x,y).
	// Recall that the GBA screen is 240 pixels wide, and 160 tall.
	// The videoBuffer is a 1-dimensional array of these pixels.
	// For example, to set pixel (37,21) to white:
	//   videoBuffer[21 * 240 + 37] = 0x7FFF;

	for (int i = 0; i < height; i++) { //copy each row of the image
		REG_DMA3SAD = (u32)(&image[i*width]);
		REG_DMA3DAD = (u32)(&videoBuffer[OFFSET(x, y+i, 240)]);
		REG_DMA3CNT = width | DMA_ON | DMA_SOURCE_INCREMENT;
	}
}

void drawChar(int x, int y, char ch, u16 color)
{
	for(int i=0; i<6; i++)
	{
		for(int j=0; j<8; j++)
		{
			if(fontdata_6x8[OFFSET(i, j, 6)+ch*48])
			{
				setPixel(x+i, y+j, color);
			} else {
				setPixel(x+i, y+j, BLACK);
			}
		}
	}
}

void drawString(int x, int y, char *str, u16 color)
{
	while(*str)
	{
		drawChar(x, y, *str++, color);
		x += 6;
	}
}