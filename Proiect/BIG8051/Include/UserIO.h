

#define KEYBLCD	0
#define TERMINAL	1

#define USERIO		TERMINAL

#if(USERIO == KEYBLCD)
	#include <lcd.h>
	#include <keyb.h>
#elif(USERIO == TERMINAL)
	#include <uart0.h>
#endif

void Afisare_meniu(void);								// afisare meniu initial
void Afisare_mesaj(void);								// afisare mesaj
void Timer0_Init(void);									// initializare timer 0
void Delay(int ms);											// intarziere programabila
void Error(char *ptr);									// afisare mesaj de eroare
void Keyblcd(void);											// interfata cu utilizatorul
void UserIO(void);										// interfata cu utilizatorul