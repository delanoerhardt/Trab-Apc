/*	Nome: Gabriel Delolmo Erhardt
*	
*	Obj: Criar um jogo onde e possivel andar por um mapa e procurar, enfrentar, derrotar ou ser derrotado pelos inimigos
*
*	Compilar com "gcc -ansi -Wall trab3apc.c -lm"
*/

#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#define CONSONANTS char consoantes[18] = {'b', 'c', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'r', 's', 't', 'v', 'x', 'y'}; /* Sem q, w e z por puro preconceito mesmo. */
#define VOGALS char vogais[5] = {'a', 'e', 'i', 'o', 'u'};

#define NAORODOUAINDA 0
#define FAILED 1
#define SUCCESS 2

#define MAPLENGHTTILLARRAYS 510

#define MAPWIDTH 120
#define MAPHEIGHT 30

#define HPATTRSCALE 7
#define LIFESTEALATTRSCALE 1
#define PRECISATTRSCALE 3
#define ADATTRSCALE 1.15f

#define BASEMAXHP 80
#define BASELIFESTEAL 0
#define BASEPRECISION 40
#define BASELEVEL 1
#define BASEATKDAMAGE 8
#define BASEMOVE 4

#define MINNAMELENGTH 4

#define ARMOVE 0
#define AFMOVE 1
#define DEFMOVE 2
#define POTMOVE 3
#define EXITMOVE 4

#define MINMAPWIDTH 70
#define MAXMAPWIDTH 130
#define MINCOLUMNWIDTH 25
#define MAXCOLUMNWIDTH 40
#define MINMAPHEIGHT 25
#define MAXMAPHEIGHT 54

#define MINSCREENWIDTH MINMAPWIDTH + MINCOLUMNWIDTH + MINCOLUMNWIDTH + 4 /* 124 */
#define MAXSCREENWIDTH MAXMAPWIDTH + MAXCOLUMNWIDTH + MAXCOLUMNWIDTH + 4 /* 154 */
#define MINSCREENHEIGHT MINMAPHEIGHT + 2 /* 27 */
#define MAXSCREENHEIGHT MAXMAPHEIGHT + 2 /* 56 */

#define UP 0
#define DOWN 1
#define RIGHT 2
#define LEFT 3
#define Z 4
#define X 5
#define P 6
#define EXIT 7

#define CUP 'A'
#define CDOWN 'B'
#define CRIGHT 'C'
#define CLEFT 'D'

#define KEYSAMOUNT 8
#define MAXLINESONMENU 10
#define MENUSAMOUNT 11

#define MAXPAGES 5

#define ESC 27

#define EMPTYTILE ' '
#define WALLTILE '#'
#define CLOSEDDOOR '+'
#define OPENDOOR '-'

#define ENEMYVAMP 'V'
#define DEADVAMP 'M'
#define ENEMYDRAC 'D'

#define ENTITYLISTSIZE 270
#define DOORLISTSIZE 600
#define ROOMLISTSIZE 256
#define ITEMLISTSIZE 100

#define WEAPON 'W'
#define ARMOR 'A'
#define POTION 'P'

#define CONFIGFILENAME "config.txt"

#define CLEAR 1

#define MOVINMAP 0
#define MAPMENU 1
#define BATTLING 2
#define UPDATING 3
#define LOSTSCREEN 4
#define WONSCREEN 5
#define WATCHING 6
#define MAINMENU 7
#define HOWTOPLAY 8
#define CONFIGUR 9
#define CONFIGURSCREEN 10
#define NAMING 11

typedef struct map map;

typedef struct vampire vampire;

typedef struct vampire {

	int id;

	unsigned int alive : 1;
	unsigned int stunned : 1;
	unsigned int lostFirst : 1;
	unsigned int named : 1;
	int xMotion:2;
	unsigned int vampireType : 2; /* 0 - enemy normal vampire; 1 - player vampire; 2 - dracula; 3 - System helper vampire */

	int yMotion:2;
	int move:6;
	
	float atkDamage;
	float currentHP;
	int maxHP;
	int lifeSteal;
	int potAmount;
	int precision;

	int level;
	int experience;
	int nextLevel;
	int spentPoints;

	int battling;

	int x, y; /* map positions */
	int xStart,
		yStart;

	char name[21];
	int itemListIDS[3];

	void (*functionAI)(struct map*, struct vampire*);
} vampire;

typedef vampire* ptr_vampire;

typedef struct arrowMenu {
	char *arrow;
	char *lineList[MAXLINESONMENU];

	int lineState[MAXLINESONMENU]; /* 0 - Not an option; 1 - Enabled; 2 - Not Shown */
	int arrowSize;

	int changeState[MENUSAMOUNT];

	int curLine,
		state; /* 0 - Not shown/created; 1 - Showing; 2 - Unfocused*/
	
	int maxLine;

	int x, /* -2 - left column; -1 - right column; w/e - x coordinate */
		y;

	int width,
		height;
} arrowMenu;

typedef struct scroll {
	char *(*pageList[MAXPAGES]);

	int curPage,
		maxPage;

	int curLine[MAXPAGES],
		maxLine[MAXPAGES],
		totalCharsOnPage[MAXPAGES];


	int state; /* 0 - Not shown/created; 1 - Showing */
	int x,
		y;

	int width,
		height;
} scroll;

typedef struct screen {
	char screenPixels[11984];


	int	offsetTop,
		offsetRight;

	int shownWidth,
		shownHeight;

	int columnWidth,
		mapWidth;

	int currentMenu;

	int drawColored;

	int width,
		height;

	arrowMenu menuList[MENUSAMOUNT];
	scroll helpScroll;
} screen;

typedef struct door {
	int id;

	int x,
		y,
		doorDirection,
		state,
		prevRoom,
		nextRoom;
} door;

typedef struct room {
	int roomsToStart;
	int id;

	int level;

	int _doorAmount,
		doorListSize;

	int state;

	int *doorListIDs;
} room;

typedef struct usable {
	int id;
	int x, y;
	int type;
	int hp,
		damage,
		lifeSteal;
	int state;
	int rarity;
	int amount;
	char name[20];
} usable;

typedef struct map {
	unsigned short *mapTiles;

	int width,
		height;


	int offsetRight,
		offsetTop;

	int minRoomTiles,
		deltaRoomTiles,
		maxRoomWidth,
		maxRoomHeight;

	int minCorridorLenght,
		deltaCorridorLenght;

	int maxDoors;
	int _doorAmount;
	int workingDoorIndex;

	int _entitiesAmount;
	int ticksTillComeBack;

	int maxRooms;
	int _roomAmount;
	int currentRoom;

	int maxItems,
		_usablesAmount,
		maxItemsOffset;

	int turnsWithFew,
		maxTurnsWithFew;

	char roundStrings[10][41];

	vampire entities[ENTITYLISTSIZE];
	door doorList[DOORLISTSIZE];
	room roomList[ROOMLISTSIZE];
	usable itemList[ITEMLISTSIZE];
} map;

typedef struct config {
	unsigned int running:1;
	unsigned int changed:1;
	unsigned int lives:3;

	unsigned int playerMoved:1;
	unsigned int messageShown:1;
	unsigned int getInput:1;
	unsigned int dontCheckBattleFor:3;
	unsigned int difficulty:2;

	unsigned int mode:4;
	unsigned int lastMode:4;

	unsigned int rightArrowSelect:1;

	int option:4;
} config;

typedef struct keyboard {
	char keys[KEYSAMOUNT][3];
	int keyParts[KEYSAMOUNT];
	int keyEnabled[KEYSAMOUNT];
} keyboard;

/*************************************************************************************************************************************************
																GAME FUNCTIONS
*************************************************************************************************************************************************/

void startGame(map *mapa, ptr_vampire player, screen *tela, keyboard *teclado, config *bits);

void saveGame(map *mapa, screen *tela, config *bits);

void readGame(map *mapa, screen *tela, config *bits);

int doGameTick(map* mapa);

void tickVampires(map *mapa);

int moveVamp(ptr_vampire vamp, map *mapa);

void moveToSelectedOption(int option, ptr_vampire vamp);

int checkForBattle(map *mapa);

int checkForWalls(map* mapa, vampire vamp1, vampire vamp2, float dist);

void calculateRoundResult(ptr_vampire enemy, ptr_vampire player, char *roundStrings, int roundResultPartSize);

int executeRound(ptr_vampire vamp1, ptr_vampire vamp2, char *roundStrings, int lineLength);

void resetMap(map *mapa);


/*************************************************************************************************************************************************
																AI FUNCTIONS
*************************************************************************************************************************************************/

void doAI(map *mapa);

void deadRising(map *mapa, ptr_vampire vamp);

void randomDirection(map *mapa, ptr_vampire vamp);

int getEnemyMove(ptr_vampire enemy, ptr_vampire player, int difficulty);

void updateEnemyAttr(ptr_vampire enemy);

/*************************************************************************************************************************************************
																	MAP FUNCTIONS
*************************************************************************************************************************************************/

void startMap(map *mapa);

void clearMap(map* mapa);

void populateMap(map *mapa);

int generateRoom(map *mapa, int x, int y, int roomSize, int roomDirection, int seen);

int selectDoorTile(map *mapa, int x, int y, int *prevdoorInfo, int doorAmount);

void changeRoomState(map* mapa, int roomID, int state);

int isPassable(short tile, char type);

/*************************************************************************************************************************************************
																MAP I/O FUNCTIONS
*************************************************************************************************************************************************/

void readMap(map *mapa);

void saveMap(map *mapa);

void readItemList(usable **itemList, int *size);

void fillMap(map *mapa, int x, int y);

void fillRoom(map *mapa, int x, int y, int level);

/*************************************************************************************************************************************************
																ARRAY MANAGMENT FUNCTIONS
*************************************************************************************************************************************************/

void addVampToMap(vampire vamp, map *mapa);

void addItemToMap(usable item, map *mapa);

void addDoorToMap(door *porta, map *mapa);

void addDoorToRoom(door porta, room *sala);

void clearPointerArray(void **array, int size);

void clearArray(char* array, int length, int stopAtFirstNull);

void freeAll(map *mapa, screen *tela);

/*************************************************************************************************************************************************
																  MENU FUNCTIONS
*************************************************************************************************************************************************/

void startMenu(arrowMenu *menu, int state, int x, int y, char* arrow);

void addLine(arrowMenu *menu, char *line, int lineState);

void setArrow(arrowMenu *menu, char *arrow);

void changeMenuState(screen *tela, int menuIndex);

int doMenuInput(screen *tela, int option, int rightArrowSelect, int resetAfterDone);

int nextLineTo(screen *tela, int menuIndex, int downOrUp);

void resetMenuValues(screen *tela, int menuIndex);

/*************************************************************************************************************************************************
																SCROLL FUNCTIONS
*************************************************************************************************************************************************/

void startScroll(scroll *papiro);

void addPage(scroll *papiro, char *page);

int doScrollInput(scroll *papiro, int option);

/*************************************************************************************************************************************************
																SCREEN FUNCTIONS
*************************************************************************************************************************************************/

void startScreen(screen *tela);

void clearScreen(screen *tela);

void drawScreen(screen* tela, map *mapa, config bits);

void showScreen(screen *tela);

void updateOffsets(map *mapa, screen *tela, int vampireToCenter);

int drawLine(screen *tela, int x, int y, char *string);

void drawBox(screen *tela, int x, int y, int width, int height);

int printVampireAt(screen *tela, int x, int y, int width, vampire vamp, int printItems);

int printItemAt(screen *tela, int x, int y, int width, usable item);

int printMenuAt(screen *tela, int x, int y, int width, int height, arrowMenu menu);

int printLives(screen *tela, int lives);

int printScrollAt(screen *tela, int x, int y, int width, int height, scroll *papiro);

/*************************************************************************************************************************************************
																	UTIL
*************************************************************************************************************************************************/

int existsValidFile(char *fileName);

double power(double a, int b);

int round(double f);

int calculateDigitsAmount(int a);

int randomWeightedAvarage(int weightsSize, int* weights);

/*************************************************************************************************************************************************
																VAMPIRE FUNCTIONS
*************************************************************************************************************************************************/

void startVampire(ptr_vampire vamp);

void nameVampire(ptr_vampire vamp);

float addCurrentHP(ptr_vampire vamp, float HP);

float setCurrentHP(ptr_vampire vamp, float HP);

void clearVampState(ptr_vampire vamp);

/*************************************************************************************************************************************************
																INPUT
*************************************************************************************************************************************************/

int readInt();

int readKeyPress(keyboard teclado);

char getch();

int kbhit();

/*************************************************************************************************************************************************
																ITEM FUNCTIONS
*************************************************************************************************************************************************/

void checkAndCreateItem(map* mapa, int doALL);

void distributeItemsToVampsAndMap(map *mapa);

int getPotionHeal(ptr_vampire vamp);

void dropItem(map *mapa, ptr_vampire vamp, int type, int dropUnder);

void pickItem(map *mapa, ptr_vampire vamp);

void giveRandomItems(ptr_vampire vamp);

usable getRandomItemOfType(int type);

usable getItemAt(int index);

usable getItem(int id, int type, int rarity, int hp, int damage, int lifeSteal, char *name, int amount);


int main()
{

	#if(CLEAR)
		system("clear");
	#endif
	srand(time(NULL));

	map mapa = {};
	vampire player = {};
	screen tela = {};
	keyboard teclado = {};
	config bits = {.running = 1, .changed = 1, .playerMoved = 1, .mode = MAINMENU, .getInput = 1, .rightArrowSelect = 1};

	startGame(&mapa, &player, &tela, &teclado, &bits);

	bits.running = bits.changed = 1;

	while(bits.running)
	{
		if(bits.changed)
		{
			updateOffsets(&mapa, &tela, bits.mode == WATCHING ? ENTITYLISTSIZE - 2 : 0);

			tela.menuList[1].lineState[0] = mapa.entities[0].itemListIDS[1] == -1 ? 0 : 1;
			tela.menuList[1].lineState[1] = mapa.entities[0].itemListIDS[2] == -1 ? 0 : 1;
			tela.menuList[1].lineState[2] = mapa.entities[0].itemListIDS[0] == -1 || mapa.entities[0].potAmount == 0 ? 0 : 1;
			tela.menuList[1].lineState[3] = mapa.entities[0].spentPoints == mapa.entities[0].level * 3 ? 0 : 1;
			tela.menuList[2].lineState[3] = mapa.entities[0].itemListIDS[0] == -1 || mapa.entities[0].potAmount == 0 ? 0 : 1;
			tela.menuList[4].lineState[0] = tela.menuList[4].lineState[1] = tela.menuList[4].lineState[2] = mapa.entities[ENTITYLISTSIZE - 1].spentPoints == mapa.entities[ENTITYLISTSIZE - 1].level * 3 ? 0 : 1;
			tela.menuList[4].lineState[3] = mapa.entities[ENTITYLISTSIZE - 1].precision != 100 && mapa.entities[ENTITYLISTSIZE - 1].spentPoints != mapa.entities[ENTITYLISTSIZE - 1].level * 3 ? 1 : 0;

			bits.messageShown = 0;

			drawScreen(&tela, &mapa, bits);
			if(!bits.playerMoved && bits.mode == 0 && bits.option >= UP && bits.option <= LEFT) {
				drawLine(&tela, 1, tela.shownHeight - 2, "Voce nao pode ir ai!");
				bits.messageShown = 1;
			}

			if(bits.mode == MOVINMAP || bits.mode == MAPMENU || bits.mode == BATTLING || bits.mode == WATCHING)
				printLives(&tela, bits.lives);

			showScreen(&tela);
			bits.changed = 0;
		}

		if(bits.dontCheckBattleFor == 0)
		{
			if(bits.mode == MOVINMAP || bits.mode == MAPMENU)
			{
				mapa.entities[0].battling = checkForBattle(&mapa);

				if(mapa.entities[0].battling != -1) {
					mapa.entities[mapa.entities[0].battling].functionAI = NULL;
					bits.changed = 1;
					bits.mode = BATTLING;
					bits.playerMoved = 0;
					changeMenuState(&tela, 2);
					putchar('\n');
					continue;
				}
			}
		} else
			bits.dontCheckBattleFor--;


		if(bits.getInput)
			bits.option = readKeyPress(teclado);

		if(bits.option == P) {
			saveGame(&mapa, &tela, &bits);
			saveMap(&mapa);
			bits.changed = 1;
			continue;
		}

		switch(bits.mode)
		{
			case MAINMENU:
			{
				if(bits.option == EXIT) {
					bits.changed = 1;
					bits.running = 0;
					break;
				}
				bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 1);
				bits.changed = 1;
				if(bits.option == -1)
					break;

				if(bits.option == tela.menuList[tela.currentMenu].maxLine - 1) {
					bits.running = 0;
					break;
				} else if(bits.option == 0) {
					bits.mode = mapa.entities[0].battling == -1 ? MOVINMAP : BATTLING;
					tela.drawColored = 1;
					tela.menuList[6].state = 0;
					changeMenuState(&tela, mapa.entities[0].battling == -1 ? 0 : mapa.entities[0].stunned == 1 ? 3 : 2);
					putchar('\n');
					break;
				} else if(bits.option == 1) {
					resetMenuValues(&tela, 5);
					startVampire(&player);
					bits.lives = 5;
					freeAll(&mapa, &tela);
					getItemAt(0xff00ff00);
					clearScreen(&tela);
					clearMap(&mapa);
					addVampToMap(player, &mapa);
					populateMap(&mapa);
					if(player.named) {
						bits.mode = MOVINMAP;
						tela.drawColored = 1;
						changeMenuState(&tela, 0);
					} else {
						bits.lastMode = bits.mode;
						bits.mode = NAMING;
						bits.getInput = 0;
						tela.menuList[6].state = 0;
						tela.drawColored = 0;
					}

				} else if(bits.option == 2) {
					bits.mode = HOWTOPLAY;
					changeMenuState(&tela, 7);
					tela.helpScroll.state = 1;
					tela.drawColored = 0;
				} else if(bits.option == 3) {
					bits.mode = CONFIGUR;
					tela.drawColored = 0;
					changeMenuState(&tela, 8);
				}
			}
			break;

			case HOWTOPLAY:
			{
				if(bits.option == EXIT) {
					bits.mode = MAINMENU;
					bits.changed = 1;
					changeMenuState(&tela, 6);
					tela.drawColored = 1;
					break;
				}
				

				bits.option = doScrollInput(&(tela.helpScroll), bits.option);

				if(bits.option == -1) {
					bits.changed = 1;
					break;
				} else if(bits.option == X || bits.option == Z) {
					bits.mode = MAINMENU;
					bits.changed = 1;
					tela.drawColored = 1;
					changeMenuState(&tela, 6);
				}
			}
			break;

			case CONFIGUR:
			{
				bits.changed = 1;
				if(bits.option == EXIT) {
					bits.mode = MAINMENU;
					changeMenuState(&tela, 6);
					tela.drawColored = 1;
					break;
				}

				bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 0);

				if(bits.option == -1) {
					break;
				}

				if(bits.option == tela.menuList[tela.currentMenu].maxLine - 1) {
					bits.mode = MAINMENU;
					resetMenuValues(&tela, 8);
					changeMenuState(&tela, 6);
					break;
				} else if(bits.option == 0) {
					bits.difficulty = (bits.difficulty + 1) % 3;
				} else if(bits.option == 1) {
					bits.mode = CONFIGURSCREEN;
					changeMenuState(&tela, 9);
				} else if(bits.option == 2) {
					bits.lastMode = bits.mode;
					bits.mode = NAMING;
					tela.drawColored = 0;
					tela.menuList[8].state = 0;
					bits.getInput = 0;
				} else if(bits.option == 3) {
					bits.rightArrowSelect = !bits.rightArrowSelect;
				}
			}
			break;

			case CONFIGURSCREEN:
			{
				bits.changed = 1;
				if(bits.option == EXIT) {
					bits.mode = CONFIGUR;
					changeMenuState(&tela, 8);
					break;
				}

				bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 1);

				if(bits.option == -1)
					break;

				if(bits.option == tela.menuList[tela.currentMenu].maxLine - 1) {
					bits.mode = CONFIGUR;
					changeMenuState(&tela, 8);
					break;
				} else if(bits.option == 0) {
					int read = 0, stage = 0;
					while(stage != 2) {
						system("clear");
						if(stage == 0) {
							printf("Escreva o comprimento da tela (>=%d):", MINSCREENWIDTH);
							read = readInt();
							if(read >= MINSCREENWIDTH && read < 1000) {
								tela.width = read;
								stage++;
							putchar('\n');
							}
						} else if(stage == 1) {
							printf("Escreva a altura da tela (>=%d):", MINSCREENHEIGHT);
							read = readInt();
							if(read >= MINSCREENHEIGHT && read < 1000) {
								tela.height = read;
								stage++;
							}
							putchar('\n');
						}
						read = 0;
					}
				} else if(bits.option == 1) {
					char read = '\0';
					int x = tela.width > MINSCREENWIDTH ? tela.width : MINSCREENWIDTH,
						y = tela.height > MINSCREENHEIGHT ? tela.height : MINSCREENHEIGHT;

					while(read != '\n') {
						system("clear");
						printf("Posicione o '@' no EXTREMO DIREITO INFERIOR da tela\nde forma que haja somente uma linha preenchida por '='\ne esse texto continue aparecendo completamente.\nComprimento atual: %d\nAltura atual: %d\n", x, y);
						int i = 0;
						for(;i < y - 6; i++)
							printf("\n");
						for(i = 0;i < x-1;i++)
							printf("=");
						printf("@");
						read = getch();
						if(read == ESC) {
							if(!kbhit())
								continue;
							read = getch();
							if(read == '[') {
								if(!kbhit())
									continue;
								read = getch();
								switch (read)
								{
									case CUP:
										y -= 1;
										if(y < MINSCREENHEIGHT) y = MINSCREENHEIGHT;
										break;

									case CDOWN:
										y += 1;
										break;

									case CRIGHT:
										x += 1;
										break;

									case CLEFT:
										x -= 1;
										if(x < MINSCREENWIDTH) x = MINSCREENWIDTH;
										break;
								}
							}
						}
					}

					tela.width = x;
					tela.height = y;
				}

				double proportion = 0.0;
				int _minMapWidth = MINMAPWIDTH, _minScreenWidth = MINSCREENWIDTH, _maxScreenWidth = MAXSCREENWIDTH, _maxScreenHeight = MAXSCREENHEIGHT;
				if(tela.width >= _maxScreenWidth) {
					tela.shownWidth = _maxScreenWidth;
					tela.columnWidth = MAXCOLUMNWIDTH;
					tela.mapWidth = MAXMAPWIDTH;
					tela.offsetRight = (tela.width - _maxScreenWidth) / 2;
				} else {
					tela.shownWidth = tela.width > _minScreenWidth ? tela.width : _minScreenWidth;
					proportion = ((double)_minMapWidth/(_minScreenWidth - 4));
					tela.mapWidth = proportion * (tela.shownWidth - 4);
					tela.columnWidth = (tela.shownWidth - 4 - tela.mapWidth) / 2;
					tela.offsetRight = 0;
				}
				
				if(tela.height >= MAXSCREENHEIGHT) {
					tela.shownHeight = MAXSCREENHEIGHT;
					tela.offsetTop = (tela.height - _maxScreenHeight) / 2;

				} else {
					tela.shownHeight = tela.height > MINSCREENHEIGHT ? tela.height : MINSCREENHEIGHT;
					tela.offsetTop = 0;
				}

				clearScreen(&tela);

				changeMenuState(&tela, 9);
			}
			break;

			case NAMING:
			{
				int nameLen = strlen(mapa.entities[0].name);
				char a = getch();
				if(a == '\n') {
					if(bits.lastMode == MAINMENU) {
						bits.mode = mapa.entities[0].stunned ? BATTLING : MOVINMAP;
						changeMenuState(&tela, mapa.entities[0].stunned ? 3 : 0);
						tela.drawColored = 1;
					} else {
						bits.mode = bits.lastMode;
						tela.menuList[8].state = 1;
					}
					mapa.entities[0].named = 1;
					bits.changed = 1;
					bits.getInput = 1;
					putchar('\n');
					break;
				} else if(a == ESC) {
					if(kbhit()){
						while(kbhit()) getchar();
						break;
					}
					if(bits.lastMode == MAINMENU) {
						tela.menuList[6].state = 1;
					} else {
						tela.menuList[8].state = 1;
					}
					bits.mode = bits.lastMode;
					clearArray(mapa.entities[0].name, 21, 1);
					bits.getInput = 1;
					bits.changed = 1;
					break;
				} else if(nameLen < 20 && (a >= ' ' && a <= '~')) {
					mapa.entities[0].name[nameLen] = a;
					mapa.entities[0].name[nameLen + 1] = '\0';
					bits.changed = 1;
				} else if(a == 127 && nameLen > 0) {
					mapa.entities[0].name[nameLen - 1] = '\0';
					bits.changed = 1;
				}
			}
			break;

			case MOVINMAP:
			{
				if(bits.option == EXIT) {
					bits.changed = 1;
					bits.mode = MAINMENU;
					changeMenuState(&tela, 6);
					break;
				}
				if(bits.option >= UP && bits.option <= LEFT) {
					moveToSelectedOption(bits.option, &(mapa.entities[0]));

					bits.playerMoved = doGameTick(&mapa);

					if(bits.playerMoved) {
						addCurrentHP(&(mapa.entities[0]), 2);
					}

					if(bits.playerMoved)
						bits.changed = 1;
					else if(!bits.messageShown)
						bits.changed = 1;

				} else if (bits.option == Z) {
					pickItem(&mapa, &(mapa.entities[0]));
					bits.changed = 1;
					bits.playerMoved = 1;
				} else if (bits.option == X) {
					changeMenuState(&tela, 1);
					bits.mode = MAPMENU;
					bits.changed = 1;
				}
			}
			break;

			case MAPMENU:
			{
				if(bits.option == EXIT) {
					bits.changed = 1;
					bits.mode = MOVINMAP;
					changeMenuState(&tela, 0);
					break;
				}

				bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 1);
				bits.changed = 1;
				if(bits.option == -1)
					break;

				if(bits.option == tela.menuList[tela.currentMenu].maxLine - 1) {
					bits.mode = MAINMENU;
					changeMenuState(&tela, 6);
					saveMap(&mapa);
					saveGame(&mapa, &tela, &bits);
					break;
				} else if(bits.option == tela.menuList[tela.currentMenu].maxLine - 2) {
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 1);
					changeMenuState(&tela, 0);
					saveMap(&mapa);
					saveGame(&mapa, &tela, &bits);
					break;
				} else if(bits.option == tela.menuList[tela.currentMenu].maxLine - 3) {
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 1);
					changeMenuState(&tela, 0);
				} else if(bits.option == 0) {
					dropItem(&mapa, &(mapa.entities[0]), 1, 1);
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 1);
					changeMenuState(&tela, 0);
				} else if(bits.option == 1) {
					dropItem(&mapa, &(mapa.entities[0]), 2, 1);
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 1);
					changeMenuState(&tela, 0);
				} else if(bits.option == 2) {
					dropItem(&mapa, &(mapa.entities[0]), 0, 1);
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 1);
					changeMenuState(&tela, 0);
				} else if(bits.option == 3) {
					changeMenuState(&tela, 4);
					mapa.entities[ENTITYLISTSIZE - 1] = mapa.entities[0];
					mapa.entities[ENTITYLISTSIZE - 1].vampireType = 3;
					tela.drawColored = 0;
					bits.mode = UPDATING;
					putchar('\n');
					break;
				} else if(bits.option == 4) {
					mapa.entities[ENTITYLISTSIZE - 2] = mapa.entities[0];
					mapa.entities[ENTITYLISTSIZE - 2].vampireType = 3;
					changeMenuState(&tela, 10);
					bits.mode = WATCHING;
				}
			}
			break;

			case BATTLING:
			{
				if(bits.option == EXIT)
					break;

				bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 0);
				bits.changed = 1;
				if(bits.option == -1) {
					break;
				}

				if(bits.option == tela.menuList[tela.currentMenu].maxLine - 1) {
					if(tela.currentMenu == 3) {
						resetMenuValues(&tela, 3);
					}
					bits.mode = MAINMENU;
					changeMenuState(&tela, 6);
					break;
				} else if(bits.option == 4) {
					mapa.entities[mapa.entities[0].battling].functionAI = randomDirection;
					mapa.entities[mapa.entities[0].battling].level++;
					mapa.entities[0].experience = 0;
					bits.dontCheckBattleFor = 1;
					updateEnemyAttr(&(mapa.entities[mapa.entities[0].battling]));
					changeMenuState(&tela, 0);
					clearArray(&(mapa.roundStrings[0][0]), 10 * 41, 0);
					mapa.entities[0].battling = -1;
					bits.mode = MOVINMAP;
				} else if (bits.option != -2) {
					doGameTick(&mapa);

					clearArray(&(mapa.roundStrings[0][0]), 10 * 41, 0);
					mapa.entities[0].move = bits.option;
					mapa.entities[mapa.entities[0].battling].move = getEnemyMove(&(mapa.entities[mapa.entities[0].battling]), &(mapa.entities[0]), bits.difficulty);
					calculateRoundResult(&(mapa.entities[mapa.entities[0].battling]), &(mapa.entities[0]), &(mapa.roundStrings[0][0]), sizeof(mapa.roundStrings[0]) );

					if(mapa.entities[0].stunned) {
						if(!mapa.entities[mapa.entities[0].battling].lostFirst) {
							changeMenuState(&tela, 3);
						} else {
							mapa.entities[0].stunned = 0;
						}
					} else if(tela.currentMenu == 3) {
						changeMenuState(&tela, 2);
					}

					if(mapa.entities[0].lostFirst)
					{
						clearArray(&(mapa.roundStrings[0][0]), 10 * 41, 0);
						bits.lives--;
						if(bits.lives == 0) {
							mapa.entities[0].battling = -1;
							bits.mode = LOSTSCREEN;
							bits.changed = 1;
							tela.drawColored = 0;
							changeMenuState(&tela, 5);
							putchar('\n');
							break;
						}
						int curLevel = mapa.entities[0].level;

						resetMap(&mapa);
						mapa.entities[0].level = (curLevel == 1) ? curLevel : (curLevel - 1);

						bits.mode = MOVINMAP;
						bits.changed = 1;
						mapa.entities[0].battling = -1;
						resetMenuValues(&tela, 2);
						changeMenuState(&tela, 0);
						putchar('\n');
					}

					if(mapa.entities[mapa.entities[0].battling].lostFirst)
					{
						clearArray(&(mapa.roundStrings[0][0]), 10 * 41, 0);
						if(mapa.entities[mapa.entities[0].battling].potAmount != 0)
							dropItem(&mapa, &(mapa.entities[mapa.entities[0].battling]), 0, 0);

						if(mapa.entities[mapa.entities[0].battling].itemListIDS[1] != -1)
							dropItem(&mapa, &(mapa.entities[mapa.entities[0].battling]), 1, 0);

						if(mapa.entities[mapa.entities[0].battling].itemListIDS[2] != -1)
							dropItem(&mapa, &(mapa.entities[mapa.entities[0].battling]), 2, 0);


						if(mapa.entities[mapa.entities[0].battling].vampireType == 2) {
							mapa.entities[0].battling = -1;
							bits.mode = WONSCREEN;
							tela.drawColored = 0;
							changeMenuState(&tela, 5);
							putchar('\n');
							break;
						}
						mapa.entities[mapa.entities[0].battling].lostFirst = 0;
						mapa.entities[mapa.entities[0].battling].alive = 0;
						mapa.entities[mapa.entities[0].battling].currentHP = 0;
						mapa.entities[mapa.entities[0].battling].functionAI = deadRising;
						mapa.entities[0].experience += mapa.entities[mapa.entities[0].battling].level * 1.7;
						mapa.entities[0].stunned = 0;
						while(mapa.entities[0].experience >= mapa.entities[0].nextLevel) {
							mapa.entities[0].experience -= mapa.entities[0].nextLevel;
							mapa.entities[0].level++;
							mapa.entities[0].nextLevel += 2;
						}
						bits.mode = MOVINMAP;
						changeRoomState(&mapa, (mapa.mapTiles[mapa.entities[mapa.entities[0].battling].y * mapa.width + mapa.entities[mapa.entities[0].battling].x] & 0xff00) >> 8, 1);
						mapa.entities[0].battling = -1;
						resetMenuValues(&tela, 2);
						changeMenuState(&tela, 0);
						putchar('\n');
					}
				}
			}
			break;

			case UPDATING:
			{
				if(bits.option == EXIT) {
					tela.drawColored = 1;
					bits.changed = 1;
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 4);
					changeMenuState(&tela, 0);
				} else {
					bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 0);
					bits.changed = 1;
					if(bits.option == -1)
						continue;

					if(bits.option == tela.menuList[tela.currentMenu].maxLine - 1) {
						tela.drawColored = 1;
						bits.mode = MOVINMAP;
						mapa.entities[0] = mapa.entities[ENTITYLISTSIZE - 1];
						mapa.entities[0].vampireType = 1;
						resetMenuValues(&tela, 4);
						changeMenuState(&tela, 0);
					} else if(bits.option == 0) {
						mapa.entities[ENTITYLISTSIZE - 1].maxHP += HPATTRSCALE;
						mapa.entities[ENTITYLISTSIZE - 1].currentHP += HPATTRSCALE;
						mapa.entities[ENTITYLISTSIZE - 1].spentPoints++;
					} else if(bits.option == 1) {
						mapa.entities[ENTITYLISTSIZE - 1].atkDamage += ADATTRSCALE;
						mapa.entities[ENTITYLISTSIZE - 1].spentPoints++;
					} else if(bits.option == 2) {
						mapa.entities[ENTITYLISTSIZE - 1].lifeSteal += LIFESTEALATTRSCALE;
						mapa.entities[ENTITYLISTSIZE - 1].spentPoints++;
					} else if(bits.option == 3) {
						mapa.entities[ENTITYLISTSIZE - 1].precision += PRECISATTRSCALE;
						if(mapa.entities[ENTITYLISTSIZE - 1].precision >= 100) {
							mapa.entities[ENTITYLISTSIZE - 1].precision = 100;
						}
						mapa.entities[ENTITYLISTSIZE - 1].spentPoints++;
					} else if(bits.option == 4) {
						mapa.entities[ENTITYLISTSIZE - 1] = mapa.entities[0];
					}
				}
			}
			break;

			case WONSCREEN:
			case LOSTSCREEN:
			{
				if(bits.option == EXIT) {
					bits.running = 0;
					bits.mode = MAINMENU;
					resetMenuValues(&tela, 5);
					startVampire(&player);
					bits.lives = 5;
					freeAll(&mapa, &tela);
					getItemAt(0xff00ff00);
					clearScreen(&tela);
					clearMap(&mapa);
					addVampToMap(player, &mapa);
					populateMap(&mapa);
					changeMenuState(&tela, 6);
					tela.drawColored = 1;
					break;
				}

				bits.option = doMenuInput(&tela, bits.option, bits.rightArrowSelect, 1);
				bits.changed = 1;

				if(bits.option == -1)
					break;

				if(bits.option == 0) {
					resetMap(&mapa);
					bits.mode = MOVINMAP;
					bits.lives = 5;
					changeMenuState(&tela, 0);
					resetMenuValues(&tela, 5);
					tela.drawColored = 1;
				} else if(bits.option == 1) {
					bits.mode = MOVINMAP;
					resetMenuValues(&tela, 5);
					startVampire(&player);
					bits.lives = 5;
					freeAll(&mapa, &tela);
					getItemAt(0xff00ff00);
					clearScreen(&tela);
					clearMap(&mapa);
					addVampToMap(player, &mapa);
					populateMap(&mapa);
					changeMenuState(&tela, 0);
					tela.drawColored = 1;
				} else if(bits.option == 2) {
					bits.mode = MAINMENU;
					resetMenuValues(&tela, 5);
					startVampire(&player);
					bits.lives = 5;
					freeAll(&mapa, &tela);
					getItemAt(0xff00ff00);
					clearScreen(&tela);
					clearMap(&mapa);
					addVampToMap(player, &mapa);
					populateMap(&mapa);
					changeMenuState(&tela, 6);
					tela.drawColored = 1;
					break;
				} else if(bits.option == 3) {
					bits.running = 0;
					bits.mode = MAINMENU;
					resetMenuValues(&tela, 5);
					startVampire(&player);
					bits.lives = 5;
					freeAll(&mapa, &tela);
					getItemAt(0xff00ff00);
					clearScreen(&tela);
					clearMap(&mapa);
					addVampToMap(player, &mapa);
					populateMap(&mapa);
					changeMenuState(&tela, 6);
					tela.drawColored = 1;
					break;
				}
			}
			break;

			case WATCHING:
			{
				bits.changed = 1;
				if(bits.option == EXIT || bits.option == X || bits.option == Z) {
					bits.mode = MOVINMAP;
					changeMenuState(&tela, 0);
					break;
				} else if(bits.option >= UP && bits.option <= LEFT) {
					moveToSelectedOption(bits.option, &(mapa.entities[ENTITYLISTSIZE - 2]));

					bits.playerMoved = moveVamp(&(mapa.entities[ENTITYLISTSIZE - 2]), &mapa);
				}
			}
			break;
		}
	}

	saveMap(&mapa);
	saveGame(&mapa, &tela, &bits);

	#if(CLEAR)
		system("clear");
	#endif

	freeAll(&mapa, &tela);
	return 0;
}

/*************************************************************************************************************************************************
																GAME FUNCTIONS
*************************************************************************************************************************************************/

void startGame(map *mapa, ptr_vampire player, screen *tela, keyboard *teclado, config *bits)
{
	/* Inicia o jogador */
	player->vampireType = 1;
	startVampire(player);
	player->xStart = player->x;
	player->yStart = player->y;

	/* Inicia a tela */
	startScreen(tela);

	getItemAt(0xff00ff00);

	/* Inicia o mapa */
	if(!existsValidFile("mapa.txt")) {
		startMap(mapa);
		addVampToMap(*player, mapa);
		populateMap(mapa);
		saveMap(mapa);
	} else {
		startMap(mapa);
		addVampToMap(*player, mapa);
		readMap(mapa);
		if(!existsValidFile("jogo.bin")) {
			resetMap(mapa);
			distributeItemsToVampsAndMap(mapa);
		} else {
			readGame(mapa, tela, bits);
		}
	}

	keyboard tecladoPadrao = {
	.keys = {	{ESC, '[', CUP},
				{ESC, '[', CDOWN},
				{ESC, '[', CRIGHT},
				{ESC, '[', CLEFT},
				{'Z', 0, 0},
				{'X', 0, 0},
				{'P', 0, 0},
				{ESC, 0, 0}		},
	.keyEnabled = 	{1, 1, 1, 1, 1, 1, 1, 1},
	.keyParts = {3, 3, 3, 3, 1, 1, 1, 1}};

	*teclado = tecladoPadrao;
}

void saveGame(map *mapa, screen *tela, config *bits)
{
	FILE *file = fopen("jogo.bin", "wb");
	if(file == NULL)
		return;
	fwrite(&(mapa->width), MAPLENGHTTILLARRAYS - sizeof(void *), 1, file);
	fwrite(&(mapa->entities[0]), sizeof(vampire), mapa->_entitiesAmount, file);
	fwrite(&(mapa->doorList[0]), sizeof(door), mapa->_doorAmount, file);
	fwrite(&(mapa->roomList[1]), sizeof(room), mapa->currentRoom, file);
	fwrite(&(mapa->itemList[0]), sizeof(usable), mapa->_usablesAmount + mapa->maxItemsOffset, file);
	int i = 0;
	for(i = 1;i <= mapa->_roomAmount;i++) {
		fwrite(&(mapa->roomList[i].doorListIDs[0]), sizeof(int), mapa->roomList[i]._doorAmount, file);
	}
	fwrite(&(tela->currentMenu), sizeof(int), 4, file);
	for(i = 0;i < MENUSAMOUNT;i++) {
		fwrite(&(tela->menuList[i].curLine), sizeof(int), 2, file);
	}
	char A = 0, B = 1, C = 2;
	for(i = 0;i < mapa->_entitiesAmount;i++) {
		fwrite(((mapa->entities[i].functionAI == NULL) ? &A : (mapa->entities[i].functionAI == randomDirection ? &B : &C)), sizeof(char), 1, file);
	}
	fwrite(bits, sizeof(config), 1, file);
	fclose(file);
}

void readGame(map *mapa, screen *tela, config *bits)
{
	FILE *file = fopen("jogo.bin", "rb");
	if(file == NULL)
		return;
	int values[2];
	fread(values, sizeof(int), 2, file);
	if(mapa->width == values[0] && mapa->height == values[1]) {
		mapa->width = values[0];
		mapa->height = values[1];
	} else {
		printf("%d =/= %d           %d =/= %d\nERRO! ARQUIVOS mapa.txt E jogo.bin NAO SAO COMPATIVEIS\nREINCIANDO JOGO A PARTIR DO MAPA PADRAO\n", mapa->width, values[0], mapa->height, values[1]);
		getchar();
		distributeItemsToVampsAndMap(mapa);
		return;
	}
	fread(&(mapa->offsetRight), MAPLENGHTTILLARRAYS - sizeof(void *) - (2 * sizeof(int)), 1, file);
	fread(&(mapa->entities[0]), sizeof(vampire), mapa->_entitiesAmount, file);
	fread(&(mapa->doorList[0]), sizeof(door), mapa->_doorAmount, file);
	fread(&(mapa->roomList[1]), sizeof(room), mapa->currentRoom, file);
	fread(&(mapa->itemList[0]), sizeof(usable), mapa->_usablesAmount + mapa->maxItemsOffset, file);
	int i = 0;
	for(i = 1;i <= mapa->_roomAmount;i++) {
		mapa->roomList[i].doorListIDs = malloc(mapa->roomList[i].doorListSize * sizeof(int));
		fread(&(mapa->roomList[i].doorListIDs[0]), sizeof(int), mapa->roomList[i]._doorAmount, file);
	}
	fread(&(tela->currentMenu), sizeof(int), 4, file);
	for(i = 0;i < MENUSAMOUNT;i++) {
		fread(&(tela->menuList[i].curLine), sizeof(int), 2, file);
	}
	char A = 0;
	for(i = 0;i < mapa->_entitiesAmount;i++) {
		fread(&A, sizeof(char), 1, file);
		if(A == 0)
			mapa->entities[i].functionAI = NULL;
		else if(A == 1)
			mapa->entities[i].functionAI = randomDirection;
		else if(A == 2)
			mapa->entities[i].functionAI = deadRising;
	}
	fread(bits, sizeof(config), 1, file);
	fclose(file);
	bits->changed = 1;
	bits->getInput = 1;
	bits->mode = MAINMENU;
	changeMenuState(tela, 6);
}

int doGameTick(map* mapa)
{
	int playerMoved = moveVamp(&(mapa->entities[0]), mapa);

	if(playerMoved || mapa->entities[0].battling != -1) {
		doAI(mapa);
		checkAndCreateItem(mapa, 0);
		tickVampires(mapa);
	}

	return playerMoved;
}

void tickVampires(map *mapa)
{
	int i = 1, moved = 0;
	for(;i < mapa->_entitiesAmount;i++) {
		moved = moveVamp(&(mapa->entities[i]), mapa);
		if(moved)
			pickItem(mapa, &(mapa->entities[i]));
		if(moved && mapa->entities[i].alive == 1)
			addCurrentHP(&(mapa->entities[i]), 2);
	}
}

int moveVamp(ptr_vampire vamp, map *mapa)
{
	int moved = 0;
	int signal = vamp->xMotion > 0 ? 1 : -1;
	vamp->xMotion *= signal;
	int finalXPos = vamp->x;
	for(;vamp->xMotion > 0; vamp->xMotion--) {
		if(finalXPos + signal < 0 || finalXPos + signal >= mapa->width) vamp->xMotion = 0;
		else if(isPassable((mapa->mapTiles)[(vamp->y * mapa->width) + finalXPos + signal], vamp->vampireType)) {
			finalXPos += signal;
			moved += 1;
		}
	}

	signal = vamp->yMotion > 0 ? 1 : -1;
	vamp->yMotion *= signal;
	int finalYPos = vamp->y;
	for(;vamp->yMotion > 0; vamp->yMotion--) {
		if(finalYPos + signal < 0 || finalYPos + signal >= mapa->height) vamp->yMotion = 0;
		else if(isPassable((mapa->mapTiles)[((finalYPos + signal) * mapa-> width) + finalXPos], vamp->vampireType)) {
			finalYPos += signal;
			moved += 1;
		}
	}

	vamp->x = finalXPos;
	vamp->y = finalYPos;

	return moved;
}

void moveToSelectedOption(int option, ptr_vampire vamp)
{
	if(option == UP)
		vamp->yMotion--;
	else if(option == DOWN)
		vamp->yMotion++;
	else if(option == RIGHT)
		vamp->xMotion++;
	else if(option == LEFT)
		vamp->xMotion--;
}

int checkForBattle(map *mapa)
{
	vampire player = mapa->entities[0], vampHold;
	short roomMask;
	char tile;
	door porta;
	int i = 1, j = 0;
	float dist;
	for(;i < mapa->_entitiesAmount;i++) {
		vampHold = mapa->entities[i];
		if(vampHold.alive == 0) continue;
		roomMask = mapa->mapTiles[player.y * mapa->width + player.x];
		tile = roomMask & 0x007f;
		roomMask = (roomMask & 0xff00) >> 8;
		if(tile == OPENDOOR) {
			for(j = 0;j < mapa->roomList[roomMask]._doorAmount;j++) {
				porta = mapa->doorList[ mapa->roomList[roomMask].doorListIDs[j] ];
				if(porta.prevRoom == roomMask || porta.nextRoom == roomMask) {
					dist = sqrt( ((vampHold.x - player.x) * (vampHold.x - player.x)) + ((vampHold.y - player.y) * (vampHold.y - player.y)) );
					if(dist <= 2.0 && checkForWalls(mapa, player, vampHold, dist)) {
						return i;
					}
				}
			}
		} else {
			if(roomMask == ((mapa->mapTiles[vampHold.y * mapa->width + vampHold.x] & 0xff00) >> 8)) {
				dist = sqrt( ((vampHold.x - player.x) * (vampHold.x - player.x)) + ((vampHold.y - player.y) * (vampHold.y - player.y)) );
				if(dist <= 2.0 && checkForWalls(mapa, player, vampHold, dist)) {

					return i;
				}
			}
		}
	}
	return -1;
}

int checkForWalls(map* mapa, vampire vamp1, vampire vamp2, float dist)
{
	int cos = (vamp2.x - vamp1.x),
		sin = (vamp2.y - vamp1.y);
	cos = cos == 2 ? cos - 1 : cos == -2 ? cos + 1 : cos;
	sin = sin == 2 ? sin - 1 : sin == -2 ? sin + 1 : sin;
	
	if(dist <= 1.0)
		return 1;
	else if(cos != 0 && sin != 0) {
		if((mapa->mapTiles[(vamp1.y + sin) * mapa->width + vamp1.x] & 0x007f) == WALLTILE && (mapa->mapTiles[(vamp1.y) * mapa->width + vamp1.x + cos] & 0x007f) == WALLTILE)
			return 0;
	} else if((mapa->mapTiles[(vamp1.y + sin) * mapa->width + vamp1.x + cos] & 0x007f) == WALLTILE) {
		return 0;
	}
	return 1;
}

/*
* Executa a acao de cada lutador de acordo com sua precisao. A menos que esteja usando pocao, nesse caso o que estiver usando uma pocao tem sua acao executada primeiro.
*
* 		Variaveis:
* enemy 				Ponteiro para vampiro inimigo
* player 				Ponteiro para vampiro do jogador
* roundStrings 			String onde sera impresso o resultado do round e que sera exibida na proxima tela
* roundResultPartSize	Size of the array inside the matrix
*/
void calculateRoundResult(ptr_vampire enemy, ptr_vampire player, char *roundStrings, int roundResultPartSize)
{
	int playerStunned = 0, enemyStunned = 0;

	if((player->precision>=enemy->precision || (player->move == POTMOVE && enemy->move != POTMOVE)) && !(player->move != POTMOVE && enemy->move == POTMOVE))
	{
		playerStunned = executeRound(player, enemy, roundStrings, roundResultPartSize);
		enemyStunned = executeRound(enemy, player, (roundStrings + 5 * roundResultPartSize	), roundResultPartSize);
	} else {
		enemyStunned = executeRound(enemy, player, (roundStrings + 5 * roundResultPartSize	), roundResultPartSize);
		playerStunned = executeRound(player, enemy, roundStrings, roundResultPartSize);
	}

	player->stunned = playerStunned;
	enemy->stunned = enemyStunned;
	return;
}

/*
* Executa a acao desejada pelo vamp1 ao vamp2.
*
* 		Variaveis:
* vamp1 			Vampiro que executa a acao
* vamp2 			Vampiro que recebe a acao
* roundStrings 	Array de strings que sera usada para retornar feedback ao jogador
* lineLength 		Tamanho da coluna onde sera escrito o feedback na tela
*
*		Retorna:
* Inteiro que eh 0 se o vamp1 nao sera atordoado apos sua jogada e 1 se ele sera atordoado.
*/
int executeRound(ptr_vampire vamp1, ptr_vampire vamp2, char *roundStrings, int lineLength)
{
	if(vamp1->lostFirst)
		return 0;

	if(vamp1->stunned)
	{
		strcpy(&roundStrings[0 * lineLength], "Nao fez nada pois\0");
		strcpy(&roundStrings[2 * lineLength], "estava atordoado!\0");
		strcpy(&roundStrings[3 * lineLength], "\0");
		if(vamp1->vampireType == 1)
			strcpy(&roundStrings[3 * lineLength], "O que fazer agora?\0");
		strcpy(&roundStrings[4 * lineLength], "\0");
		
		return 0;
	}
	int willBeStunned = 0;
	float damageDealt = 0;
	float lifeStolen = 0;
	int random = 0;
	int move = vamp1->move;
	int enemyMove = vamp2->move;
	int hasHit = 0;
	switch(move)
	{
		case ARMOVE:
			strcpy(&roundStrings[1 * lineLength], "ataque rapido!\0");
			random = -1;
			if(enemyMove != POTMOVE && !vamp2->stunned)
			{
				random = rand() % 100;
			}

			if(random < vamp1->precision)
			{
				damageDealt = vamp1->atkDamage;
				strcpy(&roundStrings[0 * lineLength], "Acertou o \0");
				hasHit = 1;
			} else {
				strcpy(&roundStrings[0 * lineLength], "Errou o \0");
			}
			sprintf(&roundStrings[0 * lineLength], "%s%s", &roundStrings[0 * lineLength], &roundStrings[1 * lineLength]);
			strcpy(&roundStrings[4 * lineLength], "                         ");

			if(enemyMove == DEFMOVE && !vamp2->stunned)
			{
				damageDealt /= 2;
				random = rand() % 100;
				if(random <  2 * (100 - vamp1->precision))
				{
					willBeStunned = 1;
					sprintf(&roundStrings[4 * lineLength], "... %s foi atordoado!", !hasHit ? "e" : "mas");
				}
			}

			lifeStolen = damageDealt * vamp1->lifeSteal;
			lifeStolen /= 100;
			if(hasHit)
			{
				sprintf(&roundStrings[2 * lineLength],"Tirou %.2f de HP!", damageDealt);
				sprintf(&roundStrings[3 * lineLength], "e roubou %.2f de vida", lifeStolen);
			} else {
				strcpy(&roundStrings[2 * lineLength],"\0");
				strcpy(&roundStrings[3 * lineLength], "\0");
			}

			break;

		case AFMOVE:
			strcpy(&roundStrings[1 * lineLength], "ataque forte!\0");
			random = -1;
			if(enemyMove != POTMOVE && !vamp2->stunned)
			{
				random = rand() % 100;
			}

			if(random < vamp1->precision)
			{
				damageDealt = 2 * vamp1->atkDamage;
				strcpy(&roundStrings[0 * lineLength], "Acertou o \0");
				hasHit = 1;
			} else {
				strcpy(&roundStrings[0 * lineLength], "Errou o \0");
			}

			sprintf(&roundStrings[0], "%s%s", &roundStrings[0 * lineLength], &roundStrings[1 * lineLength]);
			strcpy(&roundStrings[4 * lineLength], "\0");

			if(enemyMove == ARMOVE && !vamp2->stunned)
			{
				random = rand() % 100;

				if(random < 100 - vamp1->precision)
				{
					willBeStunned = 1;
					sprintf(&roundStrings[4 * lineLength], "... %s foi atordoado!", !hasHit ? "e" : "mas");
				}

			} else if(enemyMove == DEFMOVE && !vamp2->stunned) {
				damageDealt /= 2;
				willBeStunned = 1;
				sprintf(&roundStrings[4 * lineLength], "... %s foi atordoado!", !hasHit ? "e" : "mas");
			}

			if(hasHit)
				sprintf(&roundStrings[2 * lineLength],"Tirou %.2f de HP!", damageDealt);
			else
				strcpy(&roundStrings[2 * lineLength],"\0");
				
			strcpy(&roundStrings[3 * lineLength], "\0");

			break;

		case DEFMOVE:
			strcpy(&roundStrings[0 * lineLength], "Se defendeu!\0");
			strcpy(&roundStrings[2 * lineLength], "Absorvendo 50% do dano!\0");
			strcpy(&roundStrings[3 * lineLength], "\0");
			strcpy(&roundStrings[4 * lineLength], "\0");
			if((enemyMove == DEFMOVE && !vamp2->stunned) || vamp2->stunned)
			{
				float heal = addCurrentHP(vamp1, 0.1f * vamp1->maxHP);
				if(heal != 0)
				{
					sprintf(&roundStrings[3 * lineLength], "E recuperou %.2f", heal);
					strcpy(&roundStrings[4 * lineLength], "pontos de vida");
				}
			}
			break;

		case POTMOVE:
			strcpy(&roundStrings[0 * lineLength], "Usou pocao!\0");
			float heal = 0;
			if(getItemAt(vamp1->itemListIDS[0]).hp == 0)
			 	heal = addCurrentHP(vamp1, vamp1->maxHP);
			else
				heal = addCurrentHP(vamp1, getItemAt(vamp1->itemListIDS[0]).hp);
			

			if(heal > 0)
				sprintf(&roundStrings[2 * lineLength], "E curou %.2f de vida", heal);
			else 
				strcpy(&roundStrings[2 * lineLength], "E nao serviu pra nada!\0");

			strcpy(&roundStrings[3 * lineLength], "\0");
			strcpy(&roundStrings[4 * lineLength], "\0");
			vamp1->potAmount--;
			if(vamp1->potAmount == 0) vamp1->itemListIDS[0] = -1;
			break;
	}

	addCurrentHP(vamp2, -damageDealt);

	addCurrentHP(vamp1, lifeStolen);

	if(vamp2->currentHP <= 0 && !vamp1->lostFirst)
	{
		vamp2->lostFirst = 1;
	}

	return willBeStunned;
}

void resetMap(map *mapa)
{
	int i = 1;
	ptr_vampire vamp;
	for(;i < mapa->_entitiesAmount;i++) {
		vamp = &(mapa->entities[i]);
		vamp->named = 0;
		startVampire(vamp);
		vamp->x = vamp->xStart;
		vamp->y = vamp->yStart;
		vamp->level = mapa->roomList[(mapa->mapTiles[vamp->y * mapa->width + vamp->x] & 0xff00) >> 8].level;
		updateEnemyAttr(vamp);
	}

	for(i = 2;i <= mapa->_roomAmount;i++) {
		changeRoomState(mapa, i, 0);
	}
	changeRoomState(mapa, 1, 1);

	checkAndCreateItem(mapa, 1);

	startVampire(&mapa->entities[0]);

	vamp->x = vamp->xStart;
	vamp->y = vamp->yStart;
}


/*************************************************************************************************************************************************
																AI FUNCTIONS
*************************************************************************************************************************************************/

void doAI(map *mapa)
{
	int i = 1;
	for(;i < mapa->_entitiesAmount;i++) {
		ptr_vampire vamp = &(mapa->entities[i]);
		if(vamp->vampireType != 1 && vamp->functionAI != NULL) {
			(vamp->functionAI)(mapa, vamp);
		}
	}
}

void deadRising(map *mapa, ptr_vampire vamp)
{
	if(vamp->currentHP == mapa->ticksTillComeBack) {
		vamp->xStart = vamp->x;
		vamp->yStart = vamp->y;
		vamp->named = 1;
		startVampire(vamp);
		vamp->x = vamp->xStart;
		vamp->y = vamp->yStart;
		vamp->level = mapa->roomList[(mapa->mapTiles[vamp->y * mapa->width + vamp->x] & 0xff00) >> 8].level;
		updateEnemyAttr(vamp);
	} else {
		vamp->currentHP++;
	}
}

void randomDirection(map *mapa, ptr_vampire vamp)
{
	moveToSelectedOption(rand() % 4, vamp);
}

/*
* Retorna o movimento que o computador realizara. Na dificuldade facil ele executa um moviento aleatorio. Na dificuldade medio ele executa um movimento com base nas condicoes da luta no momento.
* No dificil o computador sabe qual sera o movimento do jogador e pode sempre realizar um movimento em resposta ao jogador.
*
* 		Variaveis:
* enemy 			Ponteiro do vampiro inimigo 
* player 			Ponteiro do vampiro do jogador
* difficulty 		inteiro que define a dificuldade do jogo e portanto a dificuldade da AI
*
*		Retorna:
* Um inteiro correspondente ao movimento que sera realizado pelo vampiro inimigo
*/
int getEnemyMove(ptr_vampire enemy, ptr_vampire player, int difficulty)
{
	int weights[4] = {3, 3, 2, 1};

	if(enemy->currentHP > enemy->maxHP / 2)
		weights[3] = 0;

	int move = -1;

	if(difficulty == 1)
	{
		if(player->stunned == 1)
		{
			if(enemy->currentHP <= enemy->maxHP * 0.1)
			{
				if(enemy->maxHP * 0.1 < getPotionHeal(enemy) && enemy->potAmount > 0)
				{
					move = POTMOVE;
				} else {
					move = DEFMOVE;
				}
			} else {
				if(enemy->currentHP <= enemy->maxHP * 0.9)
				{
					weights[0] = 1;
					weights[1] = 2;
					weights[2] = 0;
					weights[3] = 0;
				} else {
					move = AFMOVE;
				}
			}
		}

		if(player->currentHP < enemy->atkDamage)
		{
			move = ARMOVE;
		} else if(player->currentHP < enemy->atkDamage * 2) {
			move = AFMOVE;
		}

	} else if(difficulty == 2) {
		if(player->stunned == 1)
		{
			if(enemy->currentHP <= enemy->maxHP * 0.1)
			{
				if(enemy->maxHP * 0.1 < getPotionHeal(enemy) && enemy->potAmount > 0)
				{
					move = POTMOVE;
				} else {
					move = DEFMOVE;
				}

			} else if(enemy->currentHP <= enemy->maxHP * 0.8) {
				weights[0] = 1;
				weights[1] = 2;
				weights[2] = 0;
				weights[3] = 0;
			} else {
				move = AFMOVE;
			}
		} else if(player->move == DEFMOVE)
		{

			if(player->currentHP <= enemy->atkDamage / 2)
				move = ARMOVE;
			else if(player->currentHP <= enemy->atkDamage)
				move = AFMOVE;
			else if(getPotionHeal(enemy) > enemy->maxHP * 0.1 && enemy->potAmount > 0 && enemy->currentHP <= enemy->maxHP * 0.1)
				move = POTMOVE;
			else 
				move = DEFMOVE;

		} else if(player->move == POTMOVE) {

			if(enemy->currentHP <= enemy->maxHP * 0.1 && enemy->potAmount > 0)
				move = POTMOVE;
			else 
				move = AFMOVE;

		} else if(player->move == ARMOVE) {

			if(player->atkDamage >= enemy->currentHP)
			{

				if(enemy->atkDamage >= player->currentHP)
					move = ARMOVE;
				else if(enemy->atkDamage * 2 >= player->currentHP)
					move = AFMOVE;
				else if(player->atkDamage / 2 < enemy->currentHP)
					move = DEFMOVE;
				else
					move = POTMOVE;

			} else {

				if(enemy->atkDamage >= player->currentHP)
					move = ARMOVE;
				else if(enemy->atkDamage * 2 >= player->currentHP)
					move = AFMOVE;
				else {
					weights[0] = 1;
					weights[1] = 2;
					weights[2] = 1;
					weights[3] = 0;
				}

			}

		} else {
			if(player->atkDamage * 2 >= enemy->currentHP) 
			{

				if(enemy->atkDamage >= player->currentHP)
					move = ARMOVE;
				else if(enemy->atkDamage * 2 >= player->currentHP)
					move = AFMOVE;
				else if(player->atkDamage < enemy->currentHP)
					move = DEFMOVE;
				else
					move = POTMOVE;

			} else {

				if(enemy->atkDamage >= player->currentHP)
					move = ARMOVE;
				else if(enemy->atkDamage * 2 >= player->currentHP)
					move = AFMOVE;
				else {
					weights[0] = 1;
					weights[1] = 1;
					weights[2] = 3;
					weights[3] = 0;
				}

			}
		}
	}
	
	if(enemy->potAmount == 0) weights[3] = 0;

	if(move == -1 || (enemy->potAmount == 0 && move == 3))
		move = randomWeightedAvarage(sizeof(weights)/sizeof(weights[0]), weights);

	return move;
}

/**
* Distribui aleatoriamente os pontos de incremento do inimigo para cada atributo, com a quantidade de pontos sendo igual a 3 vezes o level do vampiro.
*/
void updateEnemyAttr(ptr_vampire enemy)
{
	int attrPoints = (enemy->level) * 3 - enemy->spentPoints;

	int extraHP = 0;
	float extraAtkDamage = 0;
	int extraPrecision = 0;
	int extraLifeSteal = 0;
	int attrToIncrease = 0;

	int weights[4] = {3, 2, 3, 2};

	while(attrPoints > 0)
	{
		attrToIncrease = randomWeightedAvarage(sizeof(weights)/sizeof(weights[0]) ,weights);
		switch(attrToIncrease)
		{
			case 0:
				extraHP += HPATTRSCALE;
				break;
			case 1:
				extraAtkDamage += ADATTRSCALE;
				break;
			case 2:
				extraPrecision += PRECISATTRSCALE;
				weights[2] = 0;
				break;
			case 3:
				extraLifeSteal += LIFESTEALATTRSCALE;
				break;
			default:
				attrPoints++;
				break;
		}
		attrPoints--;
	}
	enemy->maxHP += extraHP;
	enemy->currentHP += extraHP;
	enemy->lifeSteal += extraLifeSteal;
	enemy->precision += extraPrecision;
	if(enemy->precision > 100) enemy->precision = 100;
	enemy->atkDamage += extraAtkDamage;
	enemy->spentPoints = enemy->level * 3;
	enemy->experience = 0;
	enemy->nextLevel = 0;

	return;
}

/*************************************************************************************************************************************************
																	MAP FUNCTIONS
*************************************************************************************************************************************************/

void startMap(map *mapa)
{
	mapa->width = MAPWIDTH;
	mapa->height = MAPHEIGHT;
	mapa->offsetRight = 0;
	mapa->offsetTop = 0;
	mapa->_roomAmount = 0;
	mapa->_doorAmount = 0;
	mapa->minRoomTiles = 150;
	mapa->deltaRoomTiles = 120;
	mapa->maxRoomWidth = 9;
	mapa->maxRoomHeight = 9;
	mapa->minCorridorLenght = 3;
	mapa->deltaCorridorLenght = 5;
	mapa->maxRooms = 50;
	mapa->maxItems = 5;
	mapa->currentRoom = 1;
	mapa->ticksTillComeBack = 5;
	mapa->maxItemsOffset = 0;
	mapa->maxTurnsWithFew = 10;
	mapa->turnsWithFew = 0;
	mapa->_usablesAmount = 0;
	mapa->_entitiesAmount = 0;

	vampire nullVamp = {};

	int i = 0;
	for(;i < ENTITYLISTSIZE;i++) {
		mapa->entities[i] = nullVamp;
	}

	usable nullItem = {};
	for(i = 0;i < ITEMLISTSIZE;i++) {
		mapa->itemList[i] = nullItem;
	}

	for(i = 0;i < 8;i++)
		mapa->roundStrings[i][0] = '\0';
}

void clearMap(map* mapa)
{
	mapa->width = MAPWIDTH;
	mapa->height = MAPHEIGHT;
	mapa->offsetRight = 0;
	mapa->offsetTop = 0;
	mapa->maxRoomWidth = 9;
	mapa->maxRoomHeight = 9;
	mapa->_entitiesAmount = 0;
	mapa->turnsWithFew = 0;
	mapa->_usablesAmount = 0;

	memset(mapa->entities, 0, sizeof(mapa->entities));
	memset(mapa->itemList, 0, sizeof(mapa->itemList));
	memset(mapa->doorList, 0, 600 * sizeof(door));

	mapa->_roomAmount = 0;
	mapa->_doorAmount = 0;
}

void populateMap(map *mapa)
{
	mapa->mapTiles = malloc((mapa->width * mapa->height) * 2);

	int i = 1, j = 1, k = 0;
	for(i = 0;i < 2;i++) {
		for(j = 0;j < mapa->width;j++) {
			(mapa->mapTiles)[(i * (mapa->height - 1) * mapa->width) + j] = (WALLTILE | 0xff00);
		}
		for(j = 1;j < mapa->height - 1;j++) {
			(mapa->mapTiles)[(j * mapa->width) + (i * (mapa->width - 1))] = (WALLTILE | 0xff00);
		}
	}

	for(i = 1;i < mapa->height - 1; i++) {
		for(j = 1;j < mapa->width - 1; j++) {
			(mapa->mapTiles)[i * mapa->width + j] = (WALLTILE | 0x0080);
		}
	}

	ptr_vampire player = &(mapa->entities[0]);
	int tilesAmount = mapa->minRoomTiles + (rand() % mapa->deltaRoomTiles);
	int weights[4] = {1, 1, 1, 1};
	mapa->currentRoom = 1;
	mapa->maxDoors = 3;
	mapa->mapTiles[player->y * mapa->width + player->x] = EMPTYTILE | 0x0100;
	generateRoom(mapa, player->x, player->y, tilesAmount, randomWeightedAvarage(4, weights), 1);

	int genResult = 0;
	door *hold = NULL;
	for(mapa->workingDoorIndex = 0;mapa->workingDoorIndex < mapa->_doorAmount; mapa->workingDoorIndex++)
	{
		if(mapa->currentRoom >= mapa->maxRooms) {
			mapa->doorList[mapa->workingDoorIndex].state = 2;
			continue;
		}
		hold = &mapa->doorList[mapa->workingDoorIndex];

		if(hold->state == 0) {
			(mapa->mapTiles)[hold->y * mapa->width + hold->x] = CLOSEDDOOR | (hold->prevRoom << 8);
			(mapa->mapTiles)[hold->y * mapa->width + hold->x] &= 0xff7f;

			mapa->maxRoomWidth = (rand() % 5) + 5;
			mapa->maxRoomHeight = (rand() % 5) + 5;
			tilesAmount = 1.6 * (mapa->minRoomTiles + (rand() % mapa->deltaRoomTiles)) / sqrt(power(((13.0 + (rand() % 3)/2) / mapa->maxRoomWidth), 2) + power(((13.0 + (rand() % 3)/2) / mapa->maxRoomHeight), 2));
			mapa->currentRoom++;
			
			genResult = generateRoom(mapa, hold->x, hold->y, tilesAmount, hold->doorDirection, 1);

			if(genResult == 0)
			{
				addDoorToRoom(mapa->doorList[mapa->workingDoorIndex], &(mapa->roomList[mapa->currentRoom]) );
			}

			if(hold->state != 2)
				hold->state = 1;
		}
	}


	int holdInt = 0;
	for(i = 0;i < mapa->_doorAmount;i++) {
		if(mapa->doorList[i].state == 2  && mapa->doorList[i].prevRoom > mapa->doorList[i].nextRoom) {
			holdInt = mapa->doorList[i].prevRoom;
			mapa->doorList[i].prevRoom = mapa->doorList[i].nextRoom;
			mapa->doorList[i].nextRoom = holdInt;
		}
	}
	mapa->roomList[1].roomsToStart = 0;

	room *sala = NULL;
	door *porta = NULL;
	for(j = 0;j < mapa->_roomAmount/12;j++) {
		for(i = 1;i <= mapa->_roomAmount;i++) {
			sala = &(mapa->roomList[i]);
			if(sala == NULL) continue;
			if(sala->state == 2) continue;
			for(k = 0;k < sala->_doorAmount;k++) {
				porta = &(mapa->doorList[sala->doorListIDs[k]]);
				if(porta == NULL) continue;
				if(porta->state == 2) continue;
				int rTS = mapa->roomList[(porta->prevRoom == sala->id) ? porta->nextRoom : porta->prevRoom].roomsToStart;
				if(rTS < sala->roomsToStart)
				{
					sala->roomsToStart = rTS + 1;
				}
				if(rTS > sala->roomsToStart + 1)
				{
					mapa->roomList[(porta->prevRoom == sala->id) ? porta->nextRoom : porta->prevRoom].roomsToStart = sala->roomsToStart + 1;
				}
			}
			sala->level = sala->roomsToStart;
		}
	}

	changeRoomState(mapa, 1, 1);

	int highestRoomsToStart = 0, lastRoomPos = 0;
	for (i = 1; i <= mapa->_roomAmount; i++)
	{
		sala = &mapa->roomList[i];
		if(sala == NULL) continue;
		if(sala->state == 2) continue;
		if(sala->roomsToStart >= highestRoomsToStart) {
			highestRoomsToStart = sala->roomsToStart;
			lastRoomPos = i;
		}
	}
	
	sala = &(mapa->roomList[lastRoomPos]);
	k = sala->level;
	sala->level *= 2.5;
	j = sala->level;
	for(i = 1; i <= mapa->_roomAmount; i++) {
		sala = &(mapa->roomList[i]);
		if(sala == NULL) continue;
		if(sala->state == 2 || sala->level == 1) continue;
		sala->level = (((float)sala->level / k) * j) * 0.856;
	}

	vampire vamp = {};
	/* GENERATES ENEMIES */
	int xd = 0, yd = 0;
	for(i = 2;i <= mapa->_roomAmount;i++) {
		sala = &(mapa->roomList[i]);
		if(sala == NULL) continue;
		if(sala->state == 2) continue;
		for(j = 0;j < sala->_doorAmount;j++) {
			porta = &(mapa->doorList[sala->doorListIDs[j]]);
			if(porta == NULL) continue;
			if(porta->state == 2) continue;
			yd = porta->doorDirection == UP ? -1 : porta->doorDirection == DOWN ? 1 : 0;
			xd = porta->doorDirection == LEFT ? -1 : porta->doorDirection == RIGHT ? 1 : 0;
			int turn = ((mapa->mapTiles[ (porta->y + yd) * mapa->width + porta->x + xd ] & 0xff00) >> 8 ) == sala->id ? 1 : -1;
			xd *= turn;
			yd *= turn;
			if(((mapa->mapTiles[ (porta->y + yd) * mapa->width + porta->x + xd ] & 0xff00) >> 8 ) == sala->id) {
				vamp.named = 0;
				vamp.vampireType = lastRoomPos == i ? 2 : 0;
				startVampire(&vamp);
				vamp.level = sala->level;
				updateEnemyAttr(&vamp);
				vamp.xStart = vamp.x = porta->x + xd;
				vamp.yStart = vamp.y = porta->y + yd;
				addVampToMap(vamp, mapa);
				break;
			}
		}
	}
	
	distributeItemsToVampsAndMap(mapa);
}

int generateRoom(map *mapa, int x, int y, int roomSize, int roomDirection, int seen)
{
	unsigned short hold, roomNumberMask = mapa->currentRoom << 8;

	int xd = 0, yd = 0, startX = x, startY = y;
	int weights[4], baseWeights[] = {1, 1, 1, 1};
	int doorInfo[3][3], chanceOfDoor = 100, nextDoor = 0;
	int iterations = 0, changedTiles = 0, random;

	int leftLimit = -mapa->maxRoomWidth, rightLimit = mapa->maxRoomWidth, topLimit = -mapa->maxRoomHeight, botLimit = mapa->maxRoomHeight;

	int exitCause = 0;

	int corridorLenght = mapa->minCorridorLenght +  (rand() % mapa->deltaCorridorLenght);
	int doingCorridor = 0;

	if(mapa->currentRoom > 1) {
		int i = 0;
		for(;i < 4; i++)
			baseWeights[i] = i == roomDirection;
		leftLimit = 1000, rightLimit = 1000, topLimit = 1000, botLimit = 1000;
		doingCorridor = 1;
	} else 
		corridorLenght = 0;



	while(changedTiles < roomSize && iterations < roomSize * 11) {
		if(corridorLenght == iterations) {
			baseWeights[0] = 3;
			baseWeights[1] = 3;
			baseWeights[2] = 3;
			baseWeights[3] = 3;
			baseWeights[roomDirection == UP ? DOWN : roomDirection == DOWN ? UP : roomDirection == LEFT ? RIGHT : LEFT] = 1;
			x += xd;
			y += yd;
			xd = 0;
			yd = 0;
			leftLimit = -mapa->maxRoomWidth, rightLimit = mapa->maxRoomWidth, topLimit = -mapa->maxRoomHeight, botLimit = mapa->maxRoomHeight;
			doingCorridor = 0;
		}
		weights[0] = baseWeights[0];
		weights[1] = baseWeights[1];
		weights[2] = baseWeights[2];
		weights[3] = baseWeights[3];

		weights[roomDirection == UP ? DOWN : roomDirection == DOWN ? UP : roomDirection == LEFT ? RIGHT : LEFT] = 0;

		yd += roomDirection == UP ? -1 : roomDirection == DOWN ? 1 : 0;
		xd += roomDirection == LEFT ? -1 : roomDirection == RIGHT ? 1 : 0;

		hold = (mapa->mapTiles)[(y + yd) * mapa->width + (x + xd)];

		if( !((hold & 0xff00) == 0 || (hold & 0xff00) == roomNumberMask) ) {
			yd -= roomDirection == UP ? -1 : roomDirection == DOWN ? 1 : 0;
			xd -= roomDirection == LEFT ? -1 : roomDirection == RIGHT ? 1 : 0;
			weights[roomDirection] = 0;
			if(iterations == 0 && mapa->currentRoom != 1)
			{
				if((hold & 0x007f) != EMPTYTILE) {
					unsigned short c = WALLTILE | (hold & 0xff00);
					if(seen == 0)
						c |= 0x0080;
					mapa->currentRoom--;
					(mapa->mapTiles)[(y + yd) * mapa->width + (x + xd)] = c;
					mapa->doorList[mapa->workingDoorIndex].state = 2;
					exitCause = 2;
				} else if((hold & 0x007f) == EMPTYTILE) {

					mapa->currentRoom--;

					mapa->doorList[mapa->workingDoorIndex].nextRoom = ((hold & 0xff00) >> 8);
					addDoorToRoom( mapa->doorList[mapa->workingDoorIndex], &mapa->roomList[(hold & 0xff00) >> 8]);
					exitCause = 1;
				}
				break;
			}
		}

		if(!doingCorridor && (xd > rightLimit || xd < leftLimit || yd > botLimit || yd < topLimit)) {
			random = rand() % 100;

			if(random < chanceOfDoor && selectDoorTile(mapa, x + xd, y + yd, doorInfo[0], nextDoor) == 1 && nextDoor < mapa->maxDoors) {
				doorInfo[nextDoor][0] = x + xd;
				doorInfo[nextDoor][1] = y + yd;
				doorInfo[nextDoor][2] = roomDirection;
				nextDoor++;
				chanceOfDoor /= 7.0;
			}
			xd = 0;
			yd = 0;
			iterations++;
			continue;
		} else {
			if((hold & 0x007f) == WALLTILE && ((hold & 0xff00) == roomNumberMask || (hold & 0xff00) == 0)) {
				(mapa->mapTiles)[(y + yd) * mapa->width + (x + xd)] = EMPTYTILE;

				int j = -1, k = -1;
				changedTiles++;
				for(;j < 2;j++) {
					for(k = -1;k < 2;k++) {
						hold = (mapa->mapTiles)[(y + yd + j) * mapa->width + (x + xd + k)];
						if((hold & 0xff00) == 0) {
							if(seen == 1) {
								(mapa->mapTiles)[(y + yd + j) * mapa->width + (x + xd + k)] &= 0xff7f;
							}
							else if(seen == 0) {
								(mapa->mapTiles)[(y + yd + j) * mapa->width + (x + xd + k)] |= 0x0080;
							}
							(mapa->mapTiles)[(y + yd + j) * mapa->width + (x + xd + k)] |= roomNumberMask;
						}
					}
				}
			}
		}

		if((x + xd) == 1)
			weights[LEFT] = 0;
		else if((x + xd) > (mapa->width/4)*3 && weights[LEFT] != 0)
			weights[LEFT] += 1;

		if((x + xd) == mapa->width - 1)
			weights[RIGHT] = 0;
		else if((x + xd) < mapa->width/4 && weights[RIGHT] != 0)
			weights[RIGHT] += 1;

		if((y + yd) == 1) 
			weights[UP] = 0;
		else if((y + yd) > (mapa->height/4)*3 && weights[UP] != 0)
			weights[UP] += 1;

		if((y + yd) == mapa->height - 1)
			weights[DOWN] = 0;
		else if((y + yd) < mapa->height/4 && weights[DOWN] != 0)
			weights[DOWN] += 1;


		weights[roomDirection] += 3;
		roomDirection = randomWeightedAvarage(4, weights);
		iterations++;

	}

	if(changedTiles == 1) {
		(mapa->mapTiles)[(y + yd) * mapa->width + (x + xd)] = WALLTILE | (seen ? 0x0000 : 0x0080);
		(mapa->mapTiles)[(startY) * mapa->width + (startX)] = WALLTILE | (seen ? 0x0000 : 0x0080);
		mapa->doorList[mapa->workingDoorIndex].state = 2;
		mapa->currentRoom--;
		return 2;
	}

	if(exitCause == 0) {
		mapa->doorList[mapa->workingDoorIndex].nextRoom = mapa->currentRoom;
		room *sala = &(mapa->roomList[mapa->currentRoom]);
		sala->id = mapa->currentRoom;
		sala->_doorAmount = 0;
		sala->doorListSize = 0;
		sala->roomsToStart = 10000;
		sala->doorListIDs = NULL;
		sala->state = 0;
		mapa->_roomAmount++;

		int i;
		for(i = 0;i < nextDoor;i++) {
			door curDoor;
			curDoor.x = doorInfo[i][0];
			curDoor.y = doorInfo[i][1];
			curDoor.doorDirection = doorInfo[i][2];
			curDoor.state = 0;
			curDoor.prevRoom = sala->id;
			curDoor.nextRoom = -1;
			addDoorToMap(&curDoor, mapa);
			addDoorToRoom(curDoor, sala);
		}
	}

	return exitCause;
}

int selectDoorTile(map *mapa, int x, int y, int *prevdoorInfo, int doorAmount)
{
	unsigned short hold;
	int wallsSurround = 0, emptySurrond = 0;

	int prevX, prevY, i = 0;
	for(;i < doorAmount;i++) {
		prevX = prevdoorInfo[3 * i];
		prevY = prevdoorInfo[(3 * i) + 1];
		if((prevY == y + 1 && prevX == x) || (prevY == y - 1 && prevX == x) || (prevX == x + 1 && prevY == y) || (prevX == x - 1 && prevY == y)) {
			return 0;
		}
	}

	hold = (mapa->mapTiles)[(y + 1) * mapa->width + (x    )] & 0x007f;
	if(hold == EMPTYTILE) emptySurrond++;
	else if(hold == WALLTILE) wallsSurround++;

	hold = (mapa->mapTiles)[(y - 1) * mapa->width + (x    )] & 0x007f;
	if(hold == EMPTYTILE) emptySurrond++;
	else if(hold == WALLTILE) wallsSurround++;

	hold = (mapa->mapTiles)[(y    ) * mapa->width + (x + 1)] & 0x007f;
	if(hold == EMPTYTILE) emptySurrond++;
	else if(hold == WALLTILE) wallsSurround++;

	hold = (mapa->mapTiles)[(y    ) * mapa->width + (x - 1)] & 0x007f;
	if(hold == EMPTYTILE) emptySurrond++;
	else if(hold == WALLTILE) wallsSurround++;

	if(wallsSurround == 3 && emptySurrond == 1) return 1;
	return 0;
}

void changeRoomState(map* mapa, int roomID, int state)
{
	int i = 0, roomMask;
	room *sala = &(mapa->roomList[roomID]);
	door *porta = NULL;
	for(; i < sala->_doorAmount;i++)
	{
		porta = &(mapa->doorList[sala->doorListIDs[i]]);
		if(porta == NULL) continue;
		if(porta->state == 2) continue;
		roomMask = mapa->mapTiles[porta->y * mapa->width + porta->x] & 0xff00;
		mapa->mapTiles[porta->y * mapa->width + porta->x] = (state == 0 ? CLOSEDDOOR : OPENDOOR) | roomMask;
		porta->state = state;
	}
	sala->state = state;
}

int isPassable(short tile, char type)
{
	if(type == 3) {
		return 1;
	}
	switch(tile & 0x0007f) {
		case WALLTILE:
			return 0;

		case CLOSEDDOOR:
			return 0;

		case OPENDOOR:
			return type & 0x1;
	}

	return 1;
}

door getDoorAtRoom(map *mapa, int x, int y, int roomID)
{
	int i = 0;
	door porta = {};
	for(;i < mapa->roomList[roomID]._doorAmount;i++) {
		porta = mapa->doorList[ mapa->roomList[roomID].doorListIDs[i] ];
		if(porta.x == x && porta.y == y)
			return porta;
	}
	return mapa->doorList[0];
}

/*************************************************************************************************************************************************
																MAP I/O FUNCTIONS
*************************************************************************************************************************************************/

void readMap(map *mapa)
{
	FILE *mapFile = fopen("mapa.txt", "r");
	if(mapFile == NULL)
		return;

	fscanf(mapFile ," %d %d", &mapa->width, &mapa->height);
	if(mapa->width < 0) {
		mapa->width = mapa->height;
		fscanf(mapFile, " %d", &mapa->height);
		mapa->mapTiles = malloc(mapa->height * mapa->width * sizeof(short));
		unsigned int a = 0, i = 0, j = 0;
		char b = 0;
		for(i = 0;i < mapa->height;i++) {
			for(j = 0;j < mapa->width;j++) {
				fscanf(mapFile, " %d%c", &a, &b);
				mapa->mapTiles[i * mapa->width + j] = b;
				mapa->mapTiles[i * mapa->width + j] |= (a << 8);
			}
			fgetc(mapFile);
		}

		fscanf(mapFile, " %d %d", &mapa->entities[0].xStart, &mapa->entities[0].yStart);
		fgetc(mapFile);
		mapa->entities[0].level = 1;
		fscanf(mapFile, " %d", &mapa->_entitiesAmount);
		for(i = 1;i < mapa->_entitiesAmount;i++) {
			fscanf(mapFile, " %d %d %d", &mapa->entities[i].xStart, &mapa->entities[i].yStart, &mapa->entities[i].level);
			mapa->entities[i].x = mapa->entities[i].xStart;
			mapa->entities[i].y = mapa->entities[i].yStart;
			mapa->roomList[ (mapa->mapTiles[mapa->entities[i].y * mapa->width + mapa->entities[i].x] & 0xff00) >> 8 ].level = mapa->entities[i].level;
		}

	} else {
		mapa->mapTiles = malloc(mapa->width * mapa->height * sizeof(short));
		int i = 0, j = 0;
		char c = 0;
		while(i < mapa->width * mapa->height) {
			c = fgetc(mapFile);
			if(c == '\n') {
				continue;
			}
			mapa->mapTiles[i] = c;
			i++;
		}
		
		for(i = 0; i < 2;i++) {
			for(j = 0;j < mapa->width;j++) {
				mapa->mapTiles[i * (mapa->height - 1) * mapa->width + j] |= 0xff00;
			}
			for(j = 1;j < mapa->width - 1;j++) {
				mapa->mapTiles[j * mapa->width + (i * (mapa->width - 1))] |= 0xff00;
			}
		}

		fgetc(mapFile);
		fscanf(mapFile, " %d %d", &(mapa->entities[0].x), &(mapa->entities[0].y));
		mapa->entities[0].xStart = mapa->entities[0].x;
		mapa->entities[0].yStart = mapa->entities[0].y;
		fillMap(mapa, mapa->entities[0].x, mapa->entities[0].y);
		fscanf(mapFile, " %d", &i);
		int x, y, level;
		int highestLevel = 0, highLevelIndex = 0;
		for(j = 0;j <= i;j++) {
			mapa->entities[ENTITYLISTSIZE - 3].vampireType = 0;
			mapa->entities[ENTITYLISTSIZE - 3].named = 0;
			startVampire(&(mapa->entities[ENTITYLISTSIZE - 3]));
			mapa->entities[ENTITYLISTSIZE - 3].alive = 1;

			fscanf(mapFile, " %d %d %d", &x, &y, &level);

			mapa->entities[ENTITYLISTSIZE - 3].x = mapa->entities[ENTITYLISTSIZE - 3].xStart = x;
			mapa->entities[ENTITYLISTSIZE - 3].y = mapa->entities[ENTITYLISTSIZE - 3].yStart = y;
			mapa->entities[ENTITYLISTSIZE - 3].level = level;
			mapa->roomList[ (mapa->mapTiles[y * mapa->width + x] & 0xff00) >> 8 ].level = level;
			if(level > highestLevel) {
				highestLevel = level;
				highLevelIndex = mapa->_entitiesAmount;
			}
			updateEnemyAttr(&(mapa->entities[ENTITYLISTSIZE - 3]));

			addVampToMap(mapa->entities[ENTITYLISTSIZE - 3], mapa);
		}
		mapa->entities[highLevelIndex].vampireType = 2;
	}

	fclose(mapFile);
}

void saveMap(map *mapa)
{
	FILE *mapFile = fopen("mapa.txt", "w");
	if(mapFile == NULL)
		return;
	fprintf(mapFile, "-1 %d %d\n", mapa->width, mapa->height);
	int i = 0, j = 0;
	for(;i < mapa->height;i++) {
		for(j = 0;j < mapa->width;j++) {
			fprintf(mapFile, " %d%c", (mapa->mapTiles[i * mapa->width + j] & 0xff00) >> 8, mapa->mapTiles[i * mapa->width + j] & 0x00ff);
		}
		fputc('\n', mapFile);
	}
	fprintf(mapFile, "%d %d\n%d\n", mapa->entities[0].xStart, mapa->entities[0].yStart, mapa->_entitiesAmount);
	for(i = 1;i < mapa->_entitiesAmount;i++) {
		fprintf(mapFile, "%d %d %d\n", mapa->entities[i].xStart, mapa->entities[i].yStart, mapa->roomList[ (mapa->mapTiles[mapa->entities[i].y * mapa->width + mapa->entities[i].x] & 0xff00) >> 8 ].level);
	}
	fclose(mapFile);
}

void readItemList(usable **itemList, int *size)
{
	if(*itemList != NULL) {
		free(*itemList);
		*itemList = NULL;
	}

	if(!existsValidFile("itens.txt")) {
		*itemList = malloc(sizeof(usable) * 15);
		*size = 15;
		(*itemList)[0] = getItem(0, 0, 14, 15, 0, 0, "Pocao fraca", 1);
		(*itemList)[1] = getItem(1, 0, 10, 30, 0, 0, "Pocao media", 1);
		(*itemList)[2] = getItem(2, 0, 8, 60, 0, 0, "Pocao forte", 1);
		(*itemList)[3] = getItem(3, 0, 2, 0, 0, 0, "Super pocao", 1);

		(*itemList)[4] = getItem(4, 1, 20, 0, 8, 0, "Machado", 1);
		(*itemList)[5] = getItem(5, 1, 1, 0, 17, 0, "Machado das Trevas", 1);

		(*itemList)[6] = getItem(6, 1, 20, 0, 5, 4, "Espada", 1);
		(*itemList)[7] = getItem(7, 1, 1, 0, 8, 5, "Espada Longa", 1);

		(*itemList)[8] = getItem(8, 1, 20, 0, 2, 7, "Adaga", 1);
		(*itemList)[9] = getItem(9, 1, 1, 0, 4, 15, "Adaga Vampirica", 1);

		(*itemList)[10] = getItem(10, 2, 10, 10, 0, 0, "de papel", 1);
		(*itemList)[11] = getItem(11, 2, 6, 25, 0, 0, "normal", 1);
		(*itemList)[12] = getItem(12, 2, 4, 40, 0, 0, "de aco", 1);
		(*itemList)[13] = getItem(13, 2, 2, 100, 0, 0, "runica", 1);
		(*itemList)[14] = getItem(14, 2, 1, 70, 10, 0, "Mech Armor", 1);
	} else {
		FILE *file = fopen("itens.txt", "r");
		if(file == NULL)
			return;

		int hp = 0, damage = 0, lifeSteal = 0;

		*size = 1;
		int i = 0, j = 0;
		fscanf(file, " %d", &hp);
		fscanf(file, " %d", &i);
		*size += i;
		while(i) {
			fscanf(file, " %*[^\n]");
			i--;
		}
		fscanf(file, " %d", &i);
		*size += i;
		*itemList = malloc(sizeof(usable) * (*size));
		
		fseek(file, 0, SEEK_SET);
		int id = 0;
		(*itemList)[id] = getItem(id, 0, 1, hp, 0, 0, "Pocao", 1);
		id++;
		


		fscanf(file, " %*d %d", &j);
		char string[15];
		i = 0;
		while(i - j) {
			sprintf(string, "Arma %d", i + 1);
			fscanf(file, " %d %d", &damage, &lifeSteal);
			(*itemList)[id] = getItem(id, 1, 1, 0, damage, lifeSteal, string, 1);
			id++;
			i++;
		}

		fscanf(file, " %d", &j);
		i = 0;
		while(i - j) {
			sprintf(string, "%d", i + 1);
			fscanf(file, " %d", &hp);
			(*itemList)[id] = getItem(id, 2, 1, hp, 0, 0, string, 1);
			id++;
			i++;
		}
		fclose(file);
	}
}

void fillMap(map *mapa, int x, int y)
{
	room *sala = &(mapa->roomList[mapa->currentRoom]);
	sala->id = mapa->currentRoom;
	sala->_doorAmount = 0;
	sala->doorListSize = 0;
	sala->roomsToStart = 0;
	sala->doorListIDs = NULL;
	sala->state = 0;
	mapa->_roomAmount++;
	fillRoom(mapa, x, y, 1);
	door *porta = NULL;
	mapa->currentRoom++;
	int i = 0, xd = 0, yd = 0;
	for(i = 0;i < mapa->_doorAmount;i++) {
		porta = &(mapa->doorList[i]);
		if(porta->doorDirection == RIGHT) {
			porta->doorDirection = ((mapa->mapTiles[porta->y * mapa->width + porta->x - 1] & 0xff00) >> 8) == porta->prevRoom ? RIGHT : LEFT;
		} else {
			porta->doorDirection = ((mapa->mapTiles[(porta->y - 1) * mapa->width + porta->x] & 0xff00) >> 8) == porta->prevRoom ? DOWN : UP;
		}
		sala = &(mapa->roomList[mapa->currentRoom]);
		sala->id = mapa->currentRoom;
		sala->_doorAmount = 0;
		sala->doorListSize = 0;
		sala->doorListIDs = NULL;
		sala->state = 0;
		addDoorToRoom(*porta, sala);
		xd = porta->doorDirection == RIGHT ? 1 : porta->doorDirection == LEFT ? -1 : 0;
		yd = porta->doorDirection == DOWN ? 1 : porta->doorDirection == UP ? -1 : 0;
		if((mapa->mapTiles[(porta->y + yd) * mapa->width + porta->x + xd] & 0xff00) != 0) {
			porta->nextRoom = (mapa->mapTiles[(porta->y + yd) * mapa->width + porta->x + xd] & 0xff00) >> 8;
			addDoorToRoom(*porta, &(mapa->roomList[ (mapa->mapTiles[(porta->y + yd) * mapa->width + porta->x + xd] & 0xff00) >> 8 ]) );
			continue;
		}
		porta->nextRoom = mapa->currentRoom;
		fillRoom(mapa, porta->x + xd, porta->y + yd, mapa->currentRoom);
		mapa->_roomAmount++;
		mapa->currentRoom++;
	}
	changeRoomState(mapa, 1, 1);
}

void fillRoom(map *mapa, int x, int y, int level)
{
	/* Tirar dos comentarios se quiser acompanhar o preenchimento do mapa */
	#if(CLEAR)
		system("clear");
	#endif
	int k = 0, l = 0;
	for(;k < mapa->height;k++) {
		for(l = 0;l < mapa->width;l++) {
			if(x == l && y == k)
				printf("\x1B[7m%c\x1B[0m", mapa->mapTiles[k * mapa->width + l]);
			else {
				if((mapa->mapTiles[k * mapa->width + l] & 0xff00) == 0)
					printf("%c", mapa->mapTiles[k * mapa->width + l]);
				else {
					printf("\x1B[%dm%c\x1B[0m", 41 + (((mapa->mapTiles[k * mapa->width + l] & 0xff00) >> 8) % 7), mapa->mapTiles[k * mapa->width + l]);
				}
			}
		}
		putchar('\n');
	}
	

	mapa->mapTiles[y * mapa->width + x] |= (level << 8);
	short tile = mapa->mapTiles[y * mapa->width + x];
	if((tile & 0x007f) == OPENDOOR || (tile & 0x007f) == CLOSEDDOOR) {
		door porta = {};
		porta.x = x;
		porta.y = y;
		porta.doorDirection = (mapa->mapTiles[y * mapa->width + x - 1] & 0x007f) == EMPTYTILE ? RIGHT : UP;
		porta.state = 0;
		if((tile & 0x007f) == OPENDOOR)
			mapa->mapTiles[y * mapa->width + x] = (tile & 0xff00) | CLOSEDDOOR;
		porta.prevRoom = level;
		porta.nextRoom = -1;
		addDoorToMap(&porta, mapa);
		addDoorToRoom(porta, &(mapa->roomList[mapa->currentRoom]));
		return;
	} else if((tile & 0x007f) == WALLTILE) {
		return;
	}

	int i = -1, j = -1;
	for(;i < 2;i++) {
		for(j = -1;j < 2;j++) {
			if(!(j == 0 && i == 0) && x + j >= 0 && x + j < mapa->width && y + i >= 0 && y + i < mapa->height && (mapa->mapTiles[(y + i) * mapa->width + x + j] & 0xff00) >> 8 == 0) {
				if(abs(i) == 1 && abs(j) == 1 && (mapa->mapTiles[(y + i) * mapa->width + x] & 0x007f) == WALLTILE && (mapa->mapTiles[(y) * mapa->width + x + j] & 0x007f) == WALLTILE && (mapa->mapTiles[(y + i) * mapa->width + x + j] & 0x007f) == EMPTYTILE)
					continue;
				fillRoom(mapa, x + j, y + i, level);
			}
		}
	}
}

/*************************************************************************************************************************************************
																ARRAY MANAGMENT FUNCTIONS
*************************************************************************************************************************************************/

void addVampToMap(vampire vamp, map *mapa)
{
	vamp.id = mapa->_entitiesAmount;
	(mapa->entities)[mapa->_entitiesAmount] = vamp;
	mapa->_entitiesAmount++;
}

void addItemToMap(usable item, map *mapa)
{
	if(mapa->itemList[mapa->_usablesAmount].state == 0) {
		mapa->itemList[mapa->_usablesAmount] = item;
		mapa->_usablesAmount++;
	} else {
		int i = 0;
		for(;i < mapa->maxItems + mapa->maxItemsOffset;i++) {
			if(mapa->itemList[i].state == 0) {
				mapa->itemList[i] = item;
				mapa->_usablesAmount++;
				break;
			}
		}
	}
}

void addDoorToMap(door *porta, map *mapa)
{
	if(mapa->_doorAmount == 600) {
		return;
	}
	porta->id = mapa->_doorAmount;
	mapa->doorList[mapa->_doorAmount] = *porta;
	mapa->_doorAmount++;
}

void addDoorToRoom(door porta, room *sala)
{
	if(sala->_doorAmount == sala->doorListSize) {
		int *newDoorList = malloc(sizeof(int) * (sala->doorListSize + 5));
		int i = 0;
		for(;i < sala->doorListSize;i++)
			newDoorList[i] = sala->doorListIDs[i];
		sala->doorListSize += 5;
		free(sala->doorListIDs);

		sala->doorListIDs = newDoorList;

		for(;i < sala->doorListSize;i++)
			sala->doorListIDs[i] = -1;
	}
	sala->doorListIDs[sala->_doorAmount] = porta.id;
	sala->_doorAmount++;
}

void clearPointerArray(void **array, int size)
{
	int i = 0;
	for(;i < size;i++)
		array[i] = NULL;
}

/*
* Preenche uma array com null characters. Usado para eviar bugs devido ao re-uso de uma array
*
*		Variaveis:
* array 		Array que tera seu conteudo apagado
* length 		Tamanho da array que sera apagada
*/
void clearArray(char* array, int length, int stopAtFirstNull)
{
	int i = 0;
	for(;i<length;i++)
	{
		if(array[i] == '\0' && stopAtFirstNull) break;
		array[i] = '\0';
	}
}

void freeAll(map *mapa, screen *tela)
{
	int i = 1;
	for(;i <= mapa->_roomAmount; i++) {
		free(mapa->roomList[i].doorListIDs);
	}
	free(mapa->mapTiles);

	int j = 0;
	for(i = 0;i < tela->helpScroll.maxPage;i++) {
		for(j = 0;j < tela->helpScroll.maxLine[i]; j++)
			free((tela->helpScroll.pageList[i])[j]);
		free(tela->helpScroll.pageList[i]);
	}

	getItemAt(0xff00ff01);

}

/*************************************************************************************************************************************************
																  MENU FUNCTIONS
*************************************************************************************************************************************************/

void startMenu(arrowMenu *menu, int state, int x, int y, char *arrow) {
	menu->state = 0;
	menu->curLine = 0;
	memset(menu->changeState, -1, MENUSAMOUNT * sizeof(int));
	menu->x = x;
	menu->y = y;
	menu->width = 0;
	menu->height = 0;
	menu->changeState[0] = -2;
	menu->maxLine = 0;
	setArrow(menu, arrow);
}

void addLine(arrowMenu *menu, char *line, int lineState) /* 0 - Not an option; 1 - Enabled; 2 - Not Shown */
{
	menu->lineList[menu->maxLine] = line;
	menu->lineState[menu->maxLine] = lineState;
	menu->maxLine++;
	if(strlen(line) > menu->width)
		menu->width = strlen(line);
	menu->height = menu->maxLine;
}

void setArrow(arrowMenu *menu, char *arrow)
{
	menu->arrowSize = strlen(arrow);
	menu->arrow = arrow;
}

void changeMenuState(screen *tela, int menuIndex)
{
	if(menuIndex >= 0 && menuIndex < MENUSAMOUNT)
	{
		arrowMenu *menu = &(tela->menuList[menuIndex]);

		tela->currentMenu = menuIndex;

		int i = 0;
		for(;i < MENUSAMOUNT;i++)
			if(menu->changeState[i] == -2) {
				for(i = 0;i < MENUSAMOUNT;i++)
					tela->menuList[i].state = 0;
				break;
			} else if(menu->changeState[i] >= 0) {
				tela->menuList[menu->changeState[i]].state = 0;
			}
		menu->state = 1;
	}
}

int doMenuInput(screen *tela, int option, int rightArrowSelect, int resetAfterDone)
{
	int result = -1;
	arrowMenu *menu = &(tela->menuList[tela->currentMenu]);

	if(rightArrowSelect)
	{
		if(option == LEFT) option = X;
		if(option == RIGHT) option = Z;
	}

	if(option == X) {
		result = -2;
		if(resetAfterDone)
			resetMenuValues(tela, tela->currentMenu);
	} else if(option == Z) {
		result = menu->curLine;
		if(resetAfterDone)
			resetMenuValues(tela, tela->currentMenu);
	} else if(option == UP || option == DOWN) {
		option *= 2;
		option--;
		if(nextLineTo(tela, tela->currentMenu, option) == -1) {
			resetMenuValues(tela, tela->currentMenu);
			result = -2;
		}
	}

	return result;
}

int nextLineTo(screen *tela, int menuIndex, int downOrUp)
{
	arrowMenu *menu = &(tela->menuList[menuIndex]);

	if(downOrUp == 0) {
		if(menu->lineState[menu->curLine] == 1) {
			return menu->curLine;
		} else {
			downOrUp = 1;
		}
	}
	int i = 0;
	for(;i < MAXLINESONMENU;i++)
	{
		menu->curLine += downOrUp;
		if(menu->curLine < 0)  {
			menu->curLine %= menu->maxLine;
			menu->curLine += menu->maxLine;
		}
		if(menu->curLine >= menu->maxLine) menu->curLine %= menu->maxLine;
		if(menu->lineState[menu->curLine] == 1) break;
	}
	return i == MAXLINESONMENU ? -1 : menu->curLine;
}

void resetMenuValues(screen *tela, int menuIndex)
{
	arrowMenu *menu = &(tela->menuList[menuIndex]);
	menu->curLine = 0;
}

/*************************************************************************************************************************************************
																SCROLL FUNCTIONS
*************************************************************************************************************************************************/

void startScroll(scroll *papiro)
{
	int i = 0;
	for(;i < MAXPAGES;i++) {
		papiro->maxLine[i] = 0;
		papiro->curLine[i] = 0;
		papiro->pageList[i] = NULL;
	}
}

void addPage(scroll *papiro, char *page)
{
	papiro->curLine[papiro->maxPage] = 0;
	papiro->maxLine[papiro->maxPage] = 1;
	papiro->totalCharsOnPage[papiro->maxPage] = strlen(page);

	char *lineStart = page;
	char *word = strchr(page, ' ');
	char *strHold = NULL;

 	int i = 0;

	while(word != NULL)
	{
		strHold = strchr((word + 1), ' ');

		if((strHold - lineStart >= papiro->width)) {
			*(word) = '\0';
			lineStart = word + 1;
			papiro->maxLine[papiro->maxPage]++;
		} else if(strHold - word == 2 && *(word + 1) == '\n') {
			*(word + 1) = ' ';
			*(word + 2) = '\0';
			lineStart = word + 2;
			papiro->maxLine[papiro->maxPage]++;
		}

		word = strHold;
		i++;
	}

	papiro->pageList[papiro->maxPage] = malloc(papiro->maxLine[papiro->maxPage] * sizeof(void*));
	

	int j = 0;
	for(i = 0;i < papiro->maxLine[papiro->maxPage]; i++) {
		(papiro->pageList[papiro->maxPage])[i] = malloc( strlen(page) + 1 );
		for(j = 0;j < strlen(page);j++) {
			( ( papiro->pageList[papiro->maxPage] )[i] )[j] = page[j];
		}
		page += strlen(page) + 1;
	}

	papiro->maxPage++;
}

int doScrollInput(scroll *papiro, int option)
{
	if(option == UP && papiro->curLine[papiro->curPage] > 0) {
		papiro->curLine[papiro->curPage]--;
		return -1;
	} else if(option == DOWN && papiro->curLine[papiro->curPage] < papiro->maxLine[papiro->curPage] - 1 - papiro->height) {
		papiro->curLine[papiro->curPage]++;
		return -1;
	} else if(option == LEFT && papiro->curPage > 0) {
		papiro->curPage--;
		return -1;
	} else if(option == RIGHT && papiro->curPage < papiro->maxPage - 1) {
		papiro->curPage++;
		return -1;
	}

	return option;
}

/*************************************************************************************************************************************************
																SCREEN FUNCTIONS
*************************************************************************************************************************************************/

void startScreen(screen *tela)
{
	tela->width = MINSCREENWIDTH;
	tela->height = MINSCREENHEIGHT;
	tela->shownWidth = MINSCREENWIDTH;
	tela->shownHeight = MINSCREENHEIGHT;
	tela->offsetRight = 0;
	tela->offsetTop = 0;
	tela->columnWidth = MINCOLUMNWIDTH;
	tela->mapWidth = MINMAPWIDTH;
	tela->drawColored = 0;

	clearScreen(tela);
}

void clearScreen(screen *tela)
{
	int i = 0, j = 0;

	for(;i < sizeof(tela->screenPixels); i++) {
		tela->screenPixels[i] = ' ';
	}

	for(i = 0;i < 2;i++)
	{
		memset((tela->screenPixels + (i * tela->shownWidth * (tela->shownHeight - 1)) + 1), 0x84, tela->shownWidth - 2);
		for(j = 1; j < tela->shownHeight - 1;j++)
		{
			(tela->screenPixels)[j * tela->shownWidth + (i * (tela->shownWidth - 1))] = 0x85;
		}
		for(j = 1; j < tela->shownHeight - 1;j++)
		{
			(tela->screenPixels)[(j * tela->shownWidth) + (i * tela->mapWidth) + (tela->columnWidth) + 1 + i] = 0x85;
		}
	}

	for(i = 0;i < 2;i++)
	{
		(tela->screenPixels)[0 + (i * tela->mapWidth) + tela->columnWidth + 1 + i] = 0x87;
		(tela->screenPixels)[(tela->shownWidth * (tela->shownHeight - 1)) + (i * tela->mapWidth) + tela->columnWidth + 1 + i] = 0x86;
	}

	(tela->screenPixels)[0] = 0x80;
	(tela->screenPixels)[tela->shownWidth - 1] = 0x81;
	(tela->screenPixels)[(tela->shownHeight - 1) * tela->shownWidth] = 0x82;
	(tela->screenPixels)[(tela->shownHeight * tela->shownWidth) - 1] = 0x83;

	arrowMenu *menu = &(tela->menuList[0]);
	startMenu(menu, 0, -2, 1, "");
	addLine(menu, "Setas: Movimentar", 1);
	addLine(menu, "X:Acessar menu", 1);
	addLine(menu, "ESC:Menu principal", 1);

	menu = &(tela->menuList[1]);
	startMenu(menu, 0, -2, 1, "->");
	menu->changeState[0] = 0;
	menu->changeState[1] = 4;
	addLine(menu, "Jogar arma no chao", 1);
	addLine(menu, "Jogar armadura no chao", 1);
	addLine(menu, "Jogar pocoes no chao", 1);
	addLine(menu, "Distribuir pontos", 1);
	addLine(menu, "Vasculhar", 1);
	addLine(menu, "Voltar ao jogo", 1);
	addLine(menu, "Salvar", 1);
	addLine(menu, "Menu principal", 1);

	menu = &(tela->menuList[2]);
	startMenu(menu, 0, -2, 1, "->");
	menu->changeState[0] = 0;
	menu->changeState[1] = 1;
	menu->changeState[2] = 3;
	addLine(menu, "Ataque rapido!", 1);
	addLine(menu, "Ataque forte!", 1);
	addLine(menu, "Defender!", 1);
	addLine(menu, "Usar pocao", 1);
	addLine(menu, "Fugir", 1);
	addLine(menu, "Menu principal", 1);

	menu = &(tela->menuList[3]);
	startMenu(menu, 0, -2, 1, "->");
	addLine(menu, "Continuar", 1);
	addLine(menu, "Menu principal", 1);

	menu = &(tela->menuList[4]);
	startMenu(menu, 0, -2, 1, "->");
	addLine(menu, "Aumentar vida maxima", 1);
	addLine(menu, "Aumentar ataque", 1);
	addLine(menu, "Aumentar life-steal", 1);
	addLine(menu, "Aumentar precisao", 1);
	addLine(menu, "Redistribuir", 1);
	addLine(menu, "Continuar", 1);
	menu->x = tela->columnWidth + 1 + ( (tela->mapWidth - menu->width) / 2 );
	menu->y = (tela->shownHeight - menu->height - 2) / 2;

	menu = &(tela->menuList[5]);
	startMenu(menu, 0, -2, 1, "->");
	addLine(menu, "Jogar novamente!", 1);
	addLine(menu, "Criar novo mapa", 1);
	addLine(menu, "Menu principal", 1);
	addLine(menu, "Sair do jogo", 1);
	menu->x = tela->columnWidth + 2 + ( (tela->mapWidth - menu->width) / 2 );
	menu->y = (tela->shownHeight - menu->height - 2) / 2;

	menu = &(tela->menuList[6]);
	startMenu(menu, 0, -2, 1, "->");
	addLine(menu, "Continuar jogo!", 1);
	addLine(menu, "Novo jogo!", 1);
	addLine(menu, "Como jogar", 1);
	addLine(menu, "Opcoes", 1);
	addLine(menu, "Sair", 1);
	menu->x = tela->columnWidth + 1 + ( (tela->mapWidth - menu->width) / 2 );
	menu->y = (tela->shownHeight - menu->height - 2) / 2;

	menu = &(tela->menuList[7]);
	startMenu(menu, 0, -1, 1, "");
	menu->width = tela->columnWidth;
	menu->changeState[0] = 6;
	addLine(menu, "Setas vert.: Rolagem", 1);
	addLine(menu, "Setas hor.: Troca de pag.", 1);
	addLine(menu, "Qualquer outra: Voltar", 1);
	menu->y = (tela->shownHeight - menu->height - 2) / 2;

	menu = &(tela->menuList[8]);
	startMenu(menu, 0, -1, 1, "->");
	addLine(menu, "Dificuldade:", 1);
	addLine(menu, "Configurar o tamanho da tela:", 1);
	addLine(menu, "Alterar nome:", 1);
	addLine(menu, "Seta p/direita confirma selecao:", 1);
	addLine(menu, "", 2);
	addLine(menu, "", 2);
	addLine(menu, "Voltar ao menu", 1);
	menu->width += 7 ;
	menu->x = tela->columnWidth + 1 + ( (tela->mapWidth - menu->width) / 2 );
	menu->y = (tela->shownHeight - menu->height - 2) / 2;

	menu = &(tela->menuList[9]);
	startMenu(menu, 0, -2, 1, "->");
	addLine(menu, "Digitar o tamanho:", 1);
	addLine(menu, "Medir a tela:", 1);
	addLine(menu, "Voltar ao menu", 1);
	menu->x = tela->columnWidth + 1 + ( (tela->mapWidth - menu->width) / 2);
	menu->y = (tela->shownHeight - menu->height - 2) / 2;

	menu = &(tela->menuList[10]);
	startMenu(menu, 0, -2, 1, "");
	addLine(menu, "Setas para mover", 1);
	addLine(menu, "Voltar para o jogo", 1);

	tela->helpScroll.state = 1;
	tela->helpScroll.width = tela->mapWidth - 6;
	tela->helpScroll.height = tela->shownHeight - 2;
	tela->helpScroll.x = tela->columnWidth + 2 + 4;
	tela->helpScroll.y = 1;
	tela->helpScroll.curPage = 0;
	tela->helpScroll.maxPage = 0;
	startScroll(&(tela->helpScroll));
	char page1[] = "     Controles: \n As teclas usadas nesse jogo sao X, Z, as setas direcionais e a tecla ESC. \n As setas podem ser usadas para se mover no mapa e para controlar o seletor em menus; dentro de menus a seta para a esquerda possui funcao analoga a tecla X e a seta para a direita e analoga a tecla Z; ao se movimentar no mapa a tecla X abre o menue a tecla Z pega itens no chao. \n A tecla Z confirma a selecao no menu. A tecla X retorna, assim como a tecla ESC, quando for possivel retornar.";
	addPage(&(tela->helpScroll), page1);
	char page2[] = "     Sobre o jogo: \n Voce comeca com um vampiro de nivel 1 em uma caverna com vampiros inimigos. Voce pode derrotar os inimigos para subir de nivel e continuar explorando a caverna encontrando inimigos cada vez mais fortes e mais armados. \n A cada nivel que o jogador sobe ele ganha 3 pontos para distribuir entre seus atributos, A quantidade de pontos e mostrada na extremidade superior direita da tela, e o numero precedido por 'PTS:' \n \n Tambem e possivel pegar itens e pocoes que lhe deixam mais forte, para isso basta se posicionar em cima do item e pressinar 'Z', se vampiros inimigos passarem por cima do item, eles o pegaram automaticamente";
	addPage(&(tela->helpScroll), page2);
	char page3[] = "     Sobre mapas: \n Ao selecionar a opcao 'Continuar jogo' do menu principal, voce continua o jogo a partir do estado do jogo salvo na memoria, se e a primeira vez que joga, um mapa novo ja tera sido criado nesse ponto. \n Se ha um mapa salvo, mas nao jogou nele ainda, ele sera incializado como descrito no mapa. \n A opcao 'Novo jogo' gera um novo mapa, sobreescrevendo o antigo e apagando o estado anterior do jogo. \n Durante o jogo, no menu do mapa, existe a opcao de salvar o jogo, mas ao sair pelo menu o jogo tambem e salvo.";
	addPage(&(tela->helpScroll), page3);


	changeMenuState(tela, 6);
}

void drawScreen(screen* tela, map *mapa, config bits)
{
	ptr_vampire theWatcher = &(mapa->entities[ENTITYLISTSIZE - 2]);
	#if(CLEAR)
		system("clear");
	#endif
	int i, j = 1, yLeft = 1, yRight = 1;
	for(;j < tela->shownHeight - 1; j++)
	{
		for(i = 1;i < tela->shownWidth - 1;i++)
		{
			if( i != tela->columnWidth + 1 && i != tela->columnWidth + 2 + tela->mapWidth )
				(tela->screenPixels)[(j * tela->shownWidth) + i] = ' ';
		}
	}

	if(bits.mode == MOVINMAP || bits.mode == MAPMENU || bits.mode == BATTLING || bits.mode == WATCHING)
		yLeft = printVampireAt(tela, 1, yLeft, tela->columnWidth, mapa->entities[0], 1);

	if(bits.mode == WATCHING) {
		char string[26];
		door port = {};
		short tile = mapa->mapTiles[theWatcher->y * mapa->width + theWatcher->x];
		sprintf(string, "x:%d, y:%d. roomId:%d", theWatcher->x, theWatcher->y, (tile & 0xff00) >> 8);
		drawLine(tela, tela->columnWidth + tela->mapWidth + 3, yRight, string);
		yRight++;
		for(i = 0;i < mapa->roomList[(tile & 0xff00) >> 8]._doorAmount;i++) {
			port = mapa->doorList[ mapa->roomList[(tile & 0xff00) >> 8].doorListIDs[i] ];
			sprintf(string, "x:%d,y:%d,p:%d,n:%d", port.x, port.y, port.prevRoom, port.nextRoom);
			drawLine(tela, tela->columnWidth + tela->mapWidth + 3, yRight, string);
			yRight++;
		}
		if((tile & 0x007f) == CLOSEDDOOR || (tile & 0x007f) == OPENDOOR) {
			port = getDoorAtRoom(mapa, theWatcher->x, theWatcher->y, ((tile & 0xff00) >> 8));
			sprintf(string, "state:%d,dir:%d,prev:%d", port.state, port.doorDirection, port.prevRoom);
			drawLine(tela, tela->columnWidth + tela->mapWidth + 3, yRight, string);
			yRight++;
			sprintf(string, "next:%d", port.nextRoom);
			drawLine(tela, tela->columnWidth + tela->mapWidth + 3, yRight, string);
			yRight++;
		}
	}

	if(bits.mode == BATTLING) {
		vampire vamp = mapa->entities[0];
		if(vamp.battling != -1) {
			drawLine(tela, 1, yLeft, &(mapa->roundStrings[0][0]));
			yLeft++;
			drawLine(tela, 1, yLeft, &(mapa->roundStrings[2][0]));
			yLeft++;
			drawLine(tela, 1, yLeft, &(mapa->roundStrings[3][0]));
			yLeft++;
			drawLine(tela, 1, yLeft, &(mapa->roundStrings[4][0]));
			yLeft++;

			yRight = printVampireAt(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, tela->columnWidth, mapa->entities[vamp.battling], 1);

			drawLine(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, &(mapa->roundStrings[5][0]));
			yRight++;
			drawLine(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, &(mapa->roundStrings[7][0]));
			yRight++;
			drawLine(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, &(mapa->roundStrings[8][0]));
			yRight++;
			drawLine(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, &(mapa->roundStrings[9][0]));
			yRight++;
		}
	}

	if(bits.mode == MOVINMAP || bits.mode == MAPMENU || bits.mode == BATTLING || bits.mode == WATCHING)
	{
		char c;
		for(j = 0; j < tela->shownHeight - 2;j++)
		{
			for(i = 0;i < tela->mapWidth;i++)
			{
				if(i >= mapa->width) {
					tela->screenPixels[(j + 1) * tela->shownWidth + i + tela->columnWidth + 2] = ' ';
					continue;
				}

				if(j >= mapa->height) {
					tela->screenPixels[(j + 1) * tela->shownWidth + i + tela->columnWidth + 2] = ' ';
					continue;
				}

				c = mapa->mapTiles[(j + mapa->offsetTop) * mapa->width + i + mapa->offsetRight];

				if(!(c & 0x80))
					tela->screenPixels[(j + 1) * tela->shownWidth + i + tela->columnWidth + 2] = c;

			}
		}

		ptr_vampire _vamp = &(mapa->entities[0]);

		usable item;
		for(i = 0; i < mapa->maxItems + mapa->maxItemsOffset;i++) {
			item = mapa->itemList[i];
			if(item.state == 1 && item.y > mapa->offsetTop && item.y < mapa->offsetTop + tela->shownHeight - 2 && item.x > mapa->offsetRight && item.x < mapa->offsetRight + tela->mapWidth) {
				tela->screenPixels[(item.y + 1 - mapa->offsetTop) * tela->shownWidth + (item.x + tela->columnWidth + 2 - mapa->offsetRight)] = item.type == 0 ? 'P' : item.type == 1 ? 'W' : 'A';
			}
			if((bits.mode == WATCHING && item.x == theWatcher->x && item.y == theWatcher->y && item.state == 1) || (bits.mode != BATTLING && bits.mode != WATCHING && item.x == _vamp->x && item.y == _vamp->y && item.state == 1) )
				yRight = printItemAt(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, tela->columnWidth, item);
		}
		for(i = 1;i < mapa->_entitiesAmount;i++) {
			_vamp = &(mapa->entities[i]);
			if(_vamp != NULL) {
				if(_vamp->y >= mapa->offsetTop && _vamp->y < mapa->offsetTop + tela->shownHeight - 2 && _vamp->x >= mapa->offsetRight && _vamp->x < mapa->offsetRight + tela->mapWidth)
					tela->screenPixels[(_vamp->y + 1 - mapa->offsetTop) * tela->shownWidth + (_vamp->x + tela->columnWidth + 2 - mapa->offsetRight)] = _vamp->alive == 0 ? 'M' : _vamp->vampireType == 0 ? 'V' : _vamp->vampireType == 1 ? '@' : 'D';
				if(bits.mode == WATCHING && _vamp->x == theWatcher->x && _vamp->y == theWatcher->y && i != 0) {
					yRight = printVampireAt(tela, tela->columnWidth + 3 + tela->mapWidth, yRight, tela->columnWidth, *(_vamp), 1);
				}
			}
		}
		_vamp = &(mapa->entities[0]);
		if(_vamp->y >= mapa->offsetTop && _vamp->y < mapa->offsetTop + tela->shownHeight - 2 && _vamp->x >= mapa->offsetRight && _vamp->x < mapa->offsetRight + tela->mapWidth)
			tela->screenPixels[(_vamp->y + 1 - mapa->offsetTop) * tela->shownWidth + (_vamp->x + tela->columnWidth + 2 - mapa->offsetRight)] = '@';

		if(bits.mode == WATCHING) {
			i = mapa->entities[ENTITYLISTSIZE - 2].y + 1 - mapa->offsetTop;
			j = mapa->entities[ENTITYLISTSIZE - 2].x + tela->columnWidth + 2 - mapa->offsetRight;
			if(i != 1) {
				tela->screenPixels[(i - 1) * tela->shownWidth + j] = 0x90;
			}
			if(i != tela->shownHeight - 2) {
				tela->screenPixels[(i + 1) * tela->shownWidth + j] = 0x8f;
			}
			if(j != tela->columnWidth + 2) {
				tela->screenPixels[i * tela->shownWidth + j - 1] = 0x91;
			}
			if(j != tela->columnWidth + 1 + tela->mapWidth) {
				tela->screenPixels[i * tela->shownWidth + j + 1] = 0x92;
			}
		}
	}

	if(bits.mode == UPDATING) {
		int yMiddle = tela->shownHeight / 4;
		yMiddle = printVampireAt(tela, tela->columnWidth + 2 + (tela->mapWidth / 4), yMiddle, tela->mapWidth / 2, mapa->entities[ENTITYLISTSIZE - 1], 0);
		tela->menuList[4].y = yMiddle + 1;
	} else if(bits.mode == WONSCREEN || bits.mode == LOSTSCREEN) {
		int yMiddle = tela->shownHeight / 4;
		yMiddle -= 3;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - 60) / 2, yMiddle, "  ________                                                  ");
		yMiddle++;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - 60) / 2, yMiddle, " /  _____/_____    _____   ____     _______  __ ___________ ");
		yMiddle++;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - 60) / 2, yMiddle, "/   \\  ___\\__  \\  /     \\_/ __ \\   /  _ \\  \\/ // __ \\_  __ \\");
		yMiddle++;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - 60) / 2, yMiddle, "\\    \\_\\  \\/ __ \\|  Y Y  \\  ___/  (  <_> )   /\\  ___/|  | \\/");
		yMiddle++;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - 60) / 2, yMiddle, " \\______  (____  /__|_|  /\\___  >  \\____/ \\_/  \\___  >__|   ");
		yMiddle++;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - 60) / 2, yMiddle, "        \\/     \\/      \\/     \\/                   \\/       ");
		yMiddle += 2;
		drawLine(tela, tela->columnWidth + 2 + (tela->mapWidth - (bits.mode == WONSCREEN ? 21 : 7)) / 2, yMiddle, bits.mode == WONSCREEN ? "Parabens! Voce ganhou" : "Perdeu!");
		yMiddle += 5;

		tela->menuList[5].y = yMiddle + 1;
	} else if(bits.mode == HOWTOPLAY) {
		printScrollAt(tela, tela->helpScroll.x, tela->helpScroll.y, tela->helpScroll.width, tela->helpScroll.height, &(tela->helpScroll));
	} else if(bits.mode == CONFIGUR) {
		drawLine(tela, tela->menuList[8].x + tela->menuList[8].width, tela->menuList[8].y, bits.difficulty == 0 ? "Facil" : bits.difficulty == 1 ? "Medio" : "Dificil");
		drawLine(tela, tela->menuList[8].x + tela->menuList[8].width, tela->menuList[8].y + 3, bits.rightArrowSelect ? "Sim" : "Nao");
	} else if(bits.mode == CONFIGURSCREEN) {
		char string[35];
		i = sprintf(string, "Tamanho atual: %d/%d", tela->width, tela->height);
		drawLine(tela, tela->columnWidth + 2 + ( (tela->mapWidth - i) / 2), -2 + (tela->shownHeight - tela->menuList[9].height - 2) / 2, string);
	} else if(bits.mode == NAMING) {
		char string[] = "Digite seu nome (max 20 digitos):";
		drawLine(tela, tela->columnWidth + 2 + ( (tela->mapWidth - sizeof(string)) / 2), -4 + (tela->shownHeight / 2 ), string);
		drawBox(tela, tela->columnWidth + 2 + ( (tela->mapWidth - sizeof(mapa->entities[0].name)) / 2), -3 + (tela->shownHeight / 2), 22, 3);
		j =  drawLine(tela, tela->columnWidth + 3 + ( (tela->mapWidth - sizeof(mapa->entities[0].name) ) / 2), -2 + (tela->shownHeight / 2), mapa->entities[0].name);
		if(j < 20)
		drawLine(tela, tela->columnWidth + 3 + ( (tela->mapWidth - sizeof(mapa->entities[0].name) ) / 2) + j, -2 + (tela->shownHeight / 2), "_");
		drawLine(tela, tela->columnWidth + 3 + ( (tela->mapWidth - sizeof(mapa->entities[0].name) ) / 2), (tela->shownHeight / 2), "Enter para continuar");
	}

	arrowMenu *menu;
	for(i = 0;i < MENUSAMOUNT; i++) {
		menu = &(tela->menuList[i]);
		if(menu->state != 0 && nextLineTo(tela, i, 0) != -1) {
			j = printMenuAt(tela,
				menu->x == -2 ? 1 : menu->x == -1 ? tela->columnWidth + 3 + tela->mapWidth : menu->x,
				menu->x == -2 ? yLeft + menu->y : menu->x == -1 ? yRight + menu->y : menu->y,
				(menu->x == -2 || menu->x == -1) ? tela->columnWidth : menu->width,
				menu->x == -2 ? tela->shownHeight - yLeft - 2 : menu->x == -1 ? tela->shownHeight - yRight - 2 : menu->height,
				*menu);
			if(menu->x == -2)
				yLeft = j;
			else if(menu->x == -1)
				yRight = j;
		}
	}
}

void showScreen(screen *tela)
{
	int i = 0, j = 0;
	char c;
	for(;j < tela->offsetTop;j++) {
		putchar('\n');
	}

	for(j = 0;j < tela->shownHeight;j++)
	{
		printf("%*s", tela->offsetRight, "");
		for(i = 0;i < tela->shownWidth;i++)
		{
			c = tela->screenPixels[(j * tela->shownWidth) + i];
			if((c & 0x80)) {
				if((c & 0x40)) {
					char color = (c & 0x70) >> 4;
					if(color == 0x4)		printf("\x1B[91m");
					else if(color == 0x5)	printf("\x1B[92m");
					else if(color == 0x6)	printf("\x1B[96m");
					else					printf("\x1B[101m");

					if((c & 0x0F) == 0x0A) 	printf(".\x1B[0m");
					else 					printf("%d\x1B[0m", c & 0x0F);
					continue;
				}
				switch((c & 0x7f))
				{
					case 0x00:
						printf("╔");
						break;

					case 0x01:
						printf("╗");
						break;
						
					case 0x02:
						printf("╚");
						break;
						
					case 0x03:
						printf("╝");
						break;
						
					case 0x04:
						printf("═");
						break;
						
					case 0x05:
						printf("║");
						break;
						
					case 0x06:
						printf("╩");
						break;

					case 0x07:
						printf("╦");
						break;

					case 0x08:
						printf("╣");
						break;

					case 0x09:
						printf("╠");
						break;

					case 0x0a:
						printf("╬");
						break;

					case 0x0b:
						printf("\x1B[47m \x1B[0m");
						break;

					case 0x0c:
						printf("\x1B[90m ");
						break;

					case 0x0d:
						printf(" \x1B[0m");
						break;

					case 0x0e:
						printf("\x1B[91m♥\x1B[0m");
						break;

					case 0x0f:
						printf("\x1B[36m▲\x1B[0m");
						break;

					case 0x10:
						printf("\x1B[36m▼\x1B[0m");
						break;

					case 0x11:
						printf("\x1B[36m▶\x1B[0m");
						break;

					case 0x12:
						printf("\x1B[36m◀\x1B[0m");
						break;
				}
				continue;
			}
			if(tela->drawColored && j > 0 && j < tela->shownHeight - 1 && i > tela->columnWidth + 1 && i < tela->columnWidth + 2 + tela->mapWidth) {
				if (c == '@') {
					printf("\x1B[92m@\x1B[0m");
				} else if (c == 'V') {
					printf("\x1B[91mV\x1B[0m");
				} else if (c == 'D') {
					printf("\x1B[101mD\x1B[0m");
				} else if (c == 'P' || c == 'W' || c == 'A') {
					printf("\x1B[46m%c\x1B[0m", c);
				} else if (c == '#') {
					printf("\x1B[7m \x1B[0m");
				} else if (c == '+') {
					printf("\x1B[7m+\x1B[0m");
				} else {
					putchar(c);
				}
			} else {
					putchar(c);
			}

		}
		if(j < tela->shownHeight - 1)
			putchar('\n');
	}
}

void updateOffsets(map *mapa, screen *tela, int vampireToCenter)
{
	vampire player = mapa->entities[vampireToCenter];
	
	if(mapa->width <= tela->mapWidth)
		mapa->offsetRight = 0;
	else {
		mapa->offsetRight = player.x - (tela->mapWidth/2);
		if(mapa->offsetRight < 0) mapa->offsetRight = 0;
		else if(mapa->offsetRight > mapa->width - tela->mapWidth) mapa->offsetRight = mapa->width - tela->mapWidth;
	}

	if(mapa->height <= tela->shownHeight - 2)
		mapa->offsetTop = 0;
	else {
		mapa->offsetTop = player.y - (tela->shownHeight/2) - 1;
		if(mapa->offsetTop < 0) mapa->offsetTop = 0;
		else if(mapa->offsetTop > mapa->height - tela->shownHeight + 2) mapa->offsetTop = mapa->height - tela->shownHeight + 2;
	}
}

int drawLine(screen *tela, int x, int y, char *string)
{
	int i = 0;
	while(string[i] != '\0') {
		tela->screenPixels[y * tela->shownWidth + x + i] = string[i];
		i++;
	}
	return i;
}

void drawBox(screen *tela, int x, int y, int width, int height)
{
	if(width < 2 || height < 2)
		return;
	int i = 0, j = 0;
	for(i = 0;i < 2;i++) {
		for(j = 1;j < width - 1;j++) {
			tela->screenPixels[ (y + (i * (height - 1)) ) * tela->shownWidth + x + j] = 0x84;
		}
		for(j = 1;j < height - 1;j++) {
			tela->screenPixels[ (y + j) * tela->shownWidth + x + (i * (width - 1) ) ] = 0x85;
		}

		tela->screenPixels[ y * tela->shownWidth + x + ((width - 1) * i) ] = 0x80 + i;
		tela->screenPixels[ (y + (height - 1)) * tela->shownWidth + x + ((width - 1) * i) ] = 0x82 + i;
	}
}

int printVampireAt(screen *tela, int x, int y, int width, vampire vamp, int printItems)
{
	int i = 0;
	char string[46];
	char holdString[6];
	usable item;

	sprintf(string, "Nome:%*s", width - 5, vamp.name);
	drawLine(tela, x, y + i, string);
	i++;

	sprintf(string, "Nivel:%*d|Exp:%*d|PTS:%*d", (int)((width - 16) * 0.4), vamp.level, (int)((width - 16) * 0.4), vamp.experience, width - 16 - ((int)((width - 16) * 0.4)) * 2, vamp.level * 3 - vamp.spentPoints);
	drawLine(tela, x, y + i, string);
	i++;

	/* cur/maxHP */
	{
		int deltaHP = 0;
		if(vamp.itemListIDS[2] != -1) {
			item = getItemAt(vamp.itemListIDS[2]);
			deltaHP += item.hp;
		}

		if(deltaHP == 0) {
			sprintf(string, "Vida:%*.2f/%d", width - calculateDigitsAmount(vamp.maxHP) - 6, vamp.currentHP, vamp.maxHP);
			drawLine(tela, x, y + i, string);
			i++;
		} else {
			sprintf(string, "Vida:%*.2f/", width - calculateDigitsAmount(vamp.maxHP) - 6, vamp.currentHP);
			int size = sprintf(holdString, "%d", vamp.maxHP);
			int j = 0;
			for(;j < size;j++)
			{
				holdString[j] = (deltaHP > 0 ? 0xD0 : 0xC0) | (holdString[j] - '0');
			}
			sprintf(string, "%s%s", string, holdString);
			drawLine(tela, x, y + i, string);
			i++;
		}

		int lifeLenght = round( (((float)vamp.currentHP) / vamp.maxHP) * width);
		if(lifeLenght < 0) lifeLenght = 0;
		else if(lifeLenght > width) lifeLenght = width;
		memset(string, 0x8b, lifeLenght);
		memset((string + lifeLenght), '_', width - lifeLenght);
		string[width + 1] = '\0';
		drawLine(tela, x, y + i, string);
		i++;
	}

	/* atkDamage */
	{
		float deltaDamage = 0;
		if(vamp.itemListIDS[1] != -1) {
			item = getItemAt(vamp.itemListIDS[1]);
			deltaDamage += item.damage;
		}

		if(vamp.itemListIDS[2] != -1) {
			item = getItemAt(vamp.itemListIDS[2]);
			deltaDamage += item.damage;
		}

		if(deltaDamage == 0) {
			sprintf(string, "Ataque:%*.2f", width - 7, vamp.atkDamage);
			drawLine(tela, x, y + i, string);
			i++;
		} else {
			sprintf(string, "Ataque:");
			int size = sprintf(holdString, "%.2f", vamp.atkDamage);
			int j = 0;
			for(;j < size;j++)
			{
				holdString[j] = (deltaDamage > 0 ? 0xD0 : 0xC0) | (holdString[j] == '.' ? 0xA : holdString[j] - '0');
			}
			sprintf(string, "%s%*s", string, width - 7, holdString);
			drawLine(tela, x, y + i, string);
			i++;
		}
	}

	/* Life-steal */
	{
		int lifeSteal = 0;

		if(vamp.itemListIDS[1] != -1) {
			item = getItemAt(vamp.itemListIDS[1]);
			lifeSteal += item.lifeSteal;
		}

		if(lifeSteal == 0) {
			sprintf(string, "Life-steal:%*d%%", width - 12, vamp.lifeSteal);
			drawLine(tela, x, y + i, string);
			i++;
		} else {
			sprintf(string, "Life-steal:");
			int size = sprintf(holdString, "%d", vamp.lifeSteal);
			int j = 0;
			for(;j < size;j++)
			{
				holdString[j] = (lifeSteal > 0 ? 0xD0 : 0xC0) | (holdString[j] - '0');
			}
			sprintf(string, "%s%*s%%", string, width - 12, holdString);
			drawLine(tela, x, y + i, string);
			i++;
		}
	}

	sprintf(string, "Precisao:%*d%%", width - 10, vamp.precision);
	drawLine(tela, x, y + i, string);
	i++;

	if(printItems == 0)
		return i + y;

	if(vamp.potAmount > 0) {
		int nameSize = sprintf(string, "%s", &(getItemAt(vamp.itemListIDS[0]).name[0]));
		if(getItemAt(vamp.itemListIDS[0]).id == 3) {
			sprintf(string, "%s:%*d%%", string, width - 2 - nameSize, 100);
		} else {
			sprintf(string, "%s:%*d", string, width - 1 - nameSize, vamp.potAmount);
		}
	} else {
		sprintf(string, "Pocoes:%*d", width - 7, 0);
	}
	drawLine(tela, x, y + i, string);
	i++;

	if(vamp.itemListIDS[1] != -1) {
		item = getItemAt(vamp.itemListIDS[1]);
		sprintf(string, "Arma: %*s", width - 6, &(item.name[0]));
		drawLine(tela, x, y + i, string);

		if(item.lifeSteal > 0)
			sprintf(string, "  Dano:%*d|Life-steal:%*d%%", (int)((width - 20) * 0.6), item.damage, width - 20 - (int)((width - 20) * 0.6), item.lifeSteal);
		else
			sprintf(string, "  Dano:%*d", width - 7, item.damage);
		drawLine(tela, x, y + i + 1, string);
	}
	i += 2;
	if(vamp.itemListIDS[2] != -1) {
		item = getItemAt(vamp.itemListIDS[2]);
		if(item.id == 14) {
			sprintf(string, "%s:", &(item.name[0]));
			drawLine(tela, x, y + i, string);

			sprintf(string, "  Vida:%*d|Dano:%*d", (int)((width - 13) * 0.50), item.hp, width - 13 - (int)((width - 13) * 0.50), item.damage);
			drawLine(tela, x, y + i + 1, string);
		}
		else {
			sprintf(string, "Armadura %s:", &(item.name[0]));
			drawLine(tela, x, y + i, string);

			sprintf(string, "  Vida:%*d", width - 7, item.hp);
			drawLine(tela, x, y + i + 1, string);
		}
	}
	i += 2;
	return i + y;
}

int printItemAt(screen *tela, int x, int y, int width, usable item)
{
	int i = 0;
	char string[46];
	string[0] = '\0';
	if(item.type == 2) {
		sprintf(string, "Armadura ");
	}

	int size  = sprintf(string, "%s%s:", string, item.name);
	sprintf(string, "%s%*d", string, width - size, item.id);
	drawLine(tela, x, y + i, string);
	i++;

	if(item.type == 0) {
		if(item.hp != 0) {
			sprintf(string, "  Vida:%*d", width - 7, item.hp);
			drawLine(tela, x, y + i, string);
			i++;
		} else {
			drawLine(tela, x, y + i, "  Vida: 100%%");
			i++;
		}
		
		sprintf(string, "  Quantidade:%*d", width - 13, item.amount);
		drawLine(tela, x, y + i, string);
		i++;
	} else if(item.type == 1) {
		sprintf(string, "  Dano:%*d", width - 7, item.damage);
		drawLine(tela, x, y + i, string);
		i++;

		if(item.lifeSteal != 0) {
			sprintf(string, "  Life-steal:%*d", width - 13, item.lifeSteal);
			drawLine(tela, x, y + i, string);
			i++;
		}
	} else if(item.type == 2) {
		sprintf(string, "  Vida:%*d", width - 7, item.hp);
		drawLine(tela, x, y + i, string);
		i++;

		if(item.damage != 0) {
			sprintf(string, "  Dano:%*d", width - 7, item.damage);
			drawLine(tela, x, y + i, string);
			i++;
		}
	}
	return i + y;
}

int printMenuAt(screen *tela, int x, int y, int width, int height, arrowMenu menu)
{
	int i = 0, drawingLine = 0;
	int j = 0;
	for(;drawingLine < menu.maxLine && i < height;drawingLine++) {
		if(menu.lineState[drawingLine] == 2) {
			i++;
			continue;
		} else if(menu.lineState[drawingLine] == 0 && menu.arrowSize > 0) {
			tela->screenPixels[(y + i) * tela->shownWidth + x] = 0x8C;
			j = drawLine(tela, x + menu.arrowSize, y + i, menu.lineList[drawingLine]);
			tela->screenPixels[(y + i) * tela->shownWidth + x + menu.arrowSize + j] = 0x8D;

			i++;
			continue;
		}

		if(drawingLine == menu.curLine && menu.state == 1)
			drawLine(tela, x, y + i, menu.arrow);
		drawLine(tela, x + menu.arrowSize, y + i, menu.lineList[drawingLine]);
		i++;
	}

	return y + i;
}

int printLives(screen *tela, int lives)
{
	char heartString[10];
	int i = 0;

	for(;i < 9;i++) {
		heartString[i] = ' ';
	}

	heartString[i] = '\0';
	for(i = 0;i < lives;i++)
		heartString[i * 2] = 0x8e;

	drawLine(tela, tela->columnWidth + tela->mapWidth + 3 + ((tela->columnWidth - 9) / 2), tela->shownHeight - 2, heartString);
	return 0;
}

int printScrollAt(screen *tela, int x, int y, int width, int height, scroll *papiro)
{
	int i = 0;
	for(;i < height && i < papiro->maxLine[papiro->curPage];i++) {
		drawLine(tela, x, y + i, papiro->pageList[papiro->curPage][i + papiro->curLine[papiro->curPage]]);
	}

	char string[15];
	sprintf(string, "Pagina: %d/%d", papiro->curPage + 1, papiro->maxPage);
	drawLine(tela, tela->columnWidth + 3 + tela->mapWidth, 1, string);

	return y + i;
}

/*************************************************************************************************************************************************
																	UTIL
*************************************************************************************************************************************************/

int existsValidFile(char *fileName)
{
	FILE *file = fopen(fileName, "r");
	if(file == NULL) {
		return 0;
	}
	fclose(file);
	return 1;
}

double power(double a, int b)
{
	double result = a;
	for(;b > 1;b--) {
		result *= a;
	}
	return result;
}

int round(double f)
{
	int i = (int)(f + 0.5);
	return i;
}

int calculateDigitsAmount(int a)
{
	a *= a < 0 ? -1 : 1;
	int amount = 0;
	do {
		amount++;
		a /= 10;
	} while(a > 0);
	return amount;
}

/**
* Sorteia um dos participantes e retorna qual foi o escolhido. Cada participante tem seu peso, sua chance de ganhar, definido na array weights
*
*		Variaveis:
* weightsSize 	Quantidade de elementos na array weights e de participantes no sorteio
* weights 		Pesos associados a cada participante. Se o valor de determinado participante for 0 ele nunca ser escolhido
*/
int randomWeightedAvarage(int weightsSize, int* weights)
{
	int i = 0, sum = 0;
	for(;i < weightsSize;i++)
	{
		sum += weights[i];
	}

	int result = rand() % sum;
	for(i = 0;i < weightsSize;i++)
	{
		result -= weights[i];
		if(result < 0) break;
	}

	if(i == weightsSize)
		return -1;

	return i;
}

/*************************************************************************************************************************************************
																VAMPIRE FUNCTIONS
*************************************************************************************************************************************************/

/*
* Reseta o vampiros passado. Gera um novo nome se nao for um jogador.
*
*		Variaveis:
* vamp 				Vampiro que sera redefinido
*/
void startVampire(ptr_vampire vamp)
{
	clearVampState(vamp);
	vamp->itemListIDS[0] = -1;
	vamp->itemListIDS[1] = -1;
	vamp->itemListIDS[2] = -1;
	vamp->experience = 0;
	vamp->alive = 1;
	vamp->battling = -1;
	vamp->maxHP = BASEMAXHP;
	vamp->currentHP = vamp->maxHP;
	vamp->lifeSteal = BASELIFESTEAL;
	vamp->precision = BASEPRECISION;
	vamp->level = BASELEVEL;
	vamp->nextLevel = vamp->level * 2;
	vamp->atkDamage = BASEATKDAMAGE;
	vamp->spentPoints = 3;
	vamp->x = MAPWIDTH / 2;
	vamp->y = MAPHEIGHT / 2;
	vamp->xMotion = 0;
	vamp->yMotion = 0;
	vamp->functionAI = randomDirection;
	if(vamp->vampireType != 1 && !vamp->named) {
		clearArray(vamp->name, 20,  1);
		nameVampire(vamp);
	}

	return;
}

/*
* Da um nome aleatorio ao vampiro inimigo.
*
* 		Variaveis:
* vamp 				Vampiro que sera nomeado.
*/
void nameVampire(ptr_vampire vamp)
{
	char name[21]; /* entre 10 e 21 letras */
	int i = 0;
	int sizeName = (rand() % (22-MINNAMELENGTH)) + MINNAMELENGTH; /* entre 10 e 21 letras */
	CONSONANTS
	VOGALS
	for(;i < sizeName;i++)
	{
		name[i] = ' ';
	}
	name[i-1] = '\0';
	sizeName -= 2; /* entre 8 e 19 */
	for(i = sizeName;i >= 0;i--)
	{
		if((sizeName - i) % 2 == 0)
			name[i] = vogais[rand() % sizeof(vogais)];
		else
			name[i] = consoantes[rand() % sizeof(consoantes)];
	}

	strcpy(vamp->name, name);
	vamp->named = 1;
	return;
}

/*
* Altera vida do vampiro, verifica se esta dentro dos limites.
*
*		Variaveis:
* vamp 				Ponteiro para o vampiro que sera modificado
* HP 				Valor para aplicar como alteracao a vida atual.
*
*		Retorna:
* float igual a variacao total da vida. Pode ser igual ou menor ao valor passado em HP dependendo da vida atual do vampiro antes da alteracao.
*/
float addCurrentHP(ptr_vampire vamp, float HP)
{
	if(HP == 0) return 0;
	float lastCurHP = vamp->currentHP;
	return setCurrentHP(vamp, vamp->currentHP + HP) - lastCurHP;
}

/*
* Define a vida do vampiro, verifica se esta dentro dos limites
*
*		Variaveis:
* vamp 				Ponteiro para o vampiro que sera modificado
* HP 				Vida do vampiro. Se menor que 0 sera igual a 0; se maior que a vida maxima sera igual a vida maxima
*
*		Retorna:
* float igual a vida final do vampiro, entre 0 e a vida maxima inclusive.
*/
float setCurrentHP(ptr_vampire vamp, float HP)
{
	if(HP < 0) 
		vamp->currentHP = 0;
	else if(HP > vamp->maxHP)
		vamp->currentHP = vamp->maxHP;
	else 
		vamp->currentHP = HP;

	return vamp->currentHP;
}

/*
* Reseta as variaveis usadas em combate dos vampiros
*
*		Variaveis:
* vamp 				Vampiro que sera alterado
*/
void clearVampState(ptr_vampire vamp)
{
	vamp->potAmount = 0;
	vamp->stunned = 0;
	vamp->move = BASEMOVE;
	vamp->lostFirst = 0;
	return;
}

/*************************************************************************************************************************************************
																INPUT
*************************************************************************************************************************************************/

int readInt()
{
	char read[3], c = 'a';
	int i = 0;
	while(c != '\n') {
		c = getchar();
		if(i < 3) {
			if(c >= '0' && c <= '9') {
				read[i] = c;
				i++;
			} else {
				c = '\0';
				break;
			}
		}
	}
	if(i == 0) return -1;
	return atoi(read);
}

int readKeyPress(keyboard teclado)
{
	int i = 1, j = 0;
	char c[3];
	c[0] = toupper(getch());

	do {
		if(kbhit())
			c[i] = toupper(getch());
		else
			c[i] = 0xff;
		i++;
	} while(i < 3 && kbhit());

	while(kbhit()) getch();

	for(i = 0;i < KEYSAMOUNT;i++)
	{
		for(j = 0; teclado.keyEnabled[i] && j < teclado.keyParts[i] && teclado.keys[i][j] == c[j]; j++) ;
		if(teclado.keyParts[i] == j) return i;
	}

	return -1;
}

char getch()
{
    char ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int kbhit()
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

/*************************************************************************************************************************************************
																ITEM FUNCTIONS
*************************************************************************************************************************************************/

void checkAndCreateItem(map* mapa, int doALL)
{
	if(mapa->_usablesAmount < mapa->maxItems) {
		mapa->turnsWithFew++;
		if(mapa->turnsWithFew > mapa->maxTurnsWithFew || doALL) {
			int xd = 0, yd = 0;
			do {
				while(1) {
					xd = (rand() % (mapa->width - 2)) + 1;
					yd = (rand() % (mapa->height - 2)) + 1;
					short holdShort = mapa->mapTiles[yd * mapa->width + xd];
					if((holdShort & 0xff00) == 0 || (holdShort & 0xff00) == 0xff00 || mapa->roomList[(holdShort & 0xff00) >> 8].level == 1) continue;
					if((holdShort & 0x007f) == EMPTYTILE) {
						int weights[3] = {2, 1, 1};
						usable item = getRandomItemOfType( randomWeightedAvarage(3, weights) );
						item.state = 1;
						item.x = xd;
						item.y = yd;
						addItemToMap(item, mapa);
						break;
					}
				}
			} while(doALL && mapa->_usablesAmount < mapa->maxItems);
			mapa->turnsWithFew = 0;
		}
	} else {
		mapa->turnsWithFew = 0;
	}
}

void distributeItemsToVampsAndMap(map *mapa)
{
	checkAndCreateItem(mapa, 1);
	int i = 1;
	for(;i < mapa->_entitiesAmount;i++) {
		if(mapa->entities[i].level > 1) {
			giveRandomItems(&(mapa->entities[i]));
		}
	}
}

int getPotionHeal(ptr_vampire vamp)
{
	if(vamp->itemListIDS[0] == -1)
		return getItemAt(0).hp;
	if(getItemAt(vamp->itemListIDS[0]).hp == 0)
		return vamp->maxHP - vamp->currentHP;
	return getItemAt(vamp->itemListIDS[0]).hp;
}

void dropItem(map *mapa, ptr_vampire vamp, int type, int dropUnder)
{
	usable item = getItemAt(vamp->itemListIDS[type]);

	if(type == 0)
	{
		if(vamp->potAmount == 0) return;
		item.amount = vamp->potAmount;
		vamp->potAmount = 0;
	} else {
		vamp->maxHP -= item.hp;
		vamp->currentHP -= item.hp;
		vamp->atkDamage -= item.damage;
		vamp->lifeSteal -= item.lifeSteal;
	}
	vamp->itemListIDS[type] = -1;
	item.state = 1;
	item.x = vamp->x;
	item.y = vamp->y;
	if(!dropUnder) {
		int weights[4] = {1, 1, 1, 1};
		int dir = 0;
		while(1) {
			dir = randomWeightedAvarage(4, weights);
			if(dir == -1) {
				item.x = vamp->x;
				item.y = vamp->y;
				break;
			}
			item.x = vamp->x + (dir == UP ? -1 : dir == DOWN ? 1 : 0);
			item.y = vamp->y + (dir == LEFT ? -1 : dir == RIGHT ? 1 : 0);
			if( (mapa->mapTiles[item.y * mapa->width + item.x] & 0x007f) == EMPTYTILE)
				break;
			else {
				weights[dir] = 0;
			}
		}
	}

	if(mapa->_usablesAmount >= mapa->maxItems)
		mapa->maxItemsOffset++;
	addItemToMap(item, mapa);
}

void pickItem(map *mapa, ptr_vampire vamp)
{
	int i = 0;
	usable item;
	for(;i < mapa->maxItems + mapa->maxItemsOffset;i++)
	{
		item = mapa->itemList[i];
		if(item.state && vamp->x == item.x && vamp->y == item.y)
		{
			if(item.type == 0) {
				if(vamp->itemListIDS[0] == item.id || vamp->itemListIDS[0] == -1 || vamp->potAmount == 0) {
					vamp->potAmount += item.amount;
					mapa->itemList[i].state = 0;
					vamp->itemListIDS[0] = item.id;
					if(mapa->maxItemsOffset > 0 && i == mapa->maxItems + mapa->maxItemsOffset) 
						mapa->maxItemsOffset--;
					mapa->_usablesAmount--;
				} else {
					usable holdItem = getItemAt(vamp->itemListIDS[0]);

					holdItem.amount = vamp->potAmount;
					holdItem.x = item.x;
					holdItem.y = item.y;
					holdItem.state = 1;

					mapa->itemList[i] = holdItem;

					vamp->itemListIDS[0] = item.id;
					vamp->potAmount = item.amount;
				}
			} else {
				if(vamp->itemListIDS[item.type] == -1) {
					vamp->itemListIDS[item.type] = item.id;
					mapa->itemList[i].state = 0;
					if(i == mapa->maxItems + mapa->maxItemsOffset && mapa->maxItemsOffset > 0)
						mapa->maxItemsOffset--; 
					mapa->_usablesAmount--;
				} else {
					usable holdItem = getItemAt(vamp->itemListIDS[ item.type ]);
					vamp->atkDamage -= holdItem.damage;
					vamp->maxHP -= holdItem.hp;
					vamp->currentHP -= holdItem.hp;
					vamp->lifeSteal -= holdItem.lifeSteal;
					holdItem.x = item.x;
					holdItem.y = item.y;
					holdItem.state = 1;

					mapa->itemList[i] = holdItem;

					vamp->itemListIDS[item.type] = item.id;
				}

				vamp->atkDamage += item.damage;
				vamp->maxHP += item.hp;
				vamp->currentHP += item.hp;
				vamp->lifeSteal += item.lifeSteal;
			}
			item = mapa->itemList[i];
			return;
		}
	}
}

void giveRandomItems(ptr_vampire vamp)
{
	int weights[3] = {10, 8, 9};
	usable item;
	int i = 0, type = -1;
	while(i < 3 && (rand() % 30 <= vamp->level)) {
		type = randomWeightedAvarage(3, weights);
		item = getRandomItemOfType(type);
		if(type == 0) {
			vamp->itemListIDS[0] = item.id;
			vamp->potAmount = rand() % 4;
		} else {
			vamp->itemListIDS[type] = item.id;
			vamp->atkDamage += item.damage;
			vamp->maxHP += item.hp;
			vamp->currentHP += item.hp;
			vamp->lifeSteal += item.lifeSteal;
		}
		weights[type] = 0;
		i++;
	}
}

usable getRandomItemOfType(int type)
{
	
	int i = 0, j = 0, k = -1;
	int weights[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	for(;i < getItemAt(-1).id + 1;i++)
	{
		if(getItemAt(i).type == type) {
			if(k == -1) k = i;
			weights[j] = getItemAt(i).rarity;
			j++;
		} else if(k != -1) {
			break;
		}
	}
	i = randomWeightedAvarage(j, weights);

	return getItemAt(i + k);
	/*
	int x, y;
	int type;
	int rarity;
	int hp,
		damage,
		lifeSteal;
	char name[20];*/
}

usable getItemAt(int index)
{
	static int SIZE = 0;
	static usable *ITEMLIST = NULL;

	if(index == 0xff00ff00) {
		readItemList(&ITEMLIST, &SIZE);
		index = 0;
	} else if(index == 0xff00ff01) {
		free(ITEMLIST);
		SIZE = 0;
		ITEMLIST = NULL;
		usable kappa;
		return kappa;
	}

	if(index < 0 || index > SIZE)
		index = SIZE - 1;

	return ITEMLIST[index];
}

usable getItem(int id, int type, int rarity, int hp, int damage, int lifeSteal, char *name, int amount)
{

	usable item = {.id = id, .type = type, .rarity = rarity, .hp = hp, .damage = damage, .lifeSteal = lifeSteal, .amount = amount};
	int i = 0;
	while(1) {
		if(name[i] == '\0')
			break;
		item.name[i] = name[i];
		i++;
	}
	return item;
}