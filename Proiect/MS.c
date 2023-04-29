#include <c8051F040.h>	// declaratii SFR
#include <wdt.h>
#include <osc.h>
#include <port.h>
#include <uart1.h>
#include <lcd.h>
#include <keyb.h>
#include <Protocol.h>
#include <UserIO.h>

nod retea[NR_NODURI];					// reteaua Master-Slave, cu 5 noduri

unsigned char STARE_NOD = 0;		// starea initiala a nodului curent
unsigned char TIP_NOD	= 0;		// tip nod initial: Slave sau No JET
unsigned char STARE_IO 	= 0;		// stare interfata IO - asteptare comenzi
unsigned char ADR_MASTER;				// adresa nod master - numai MS
extern unsigned char AFISARE;

//***********************************************************************************************************
void TxMesaj(unsigned char i);	// transmisie mesaj destinat nodului i
unsigned char RxMesaj(unsigned char i);		// primire mesaj de la nodul i

//***********************************************************************************************************
void main (void) {
	unsigned char i, found;	// variabile locale
	
	WDT_Disable();												// dezactiveaza WDT
	SYSCLK_Init();												// initializeaza si selecteaza oscilatorul ales in osc.h
	UART1_Init(NINE_BIT, BAUDRATE_COM);		// initilizare UART1  - conectata la RS232-B (P1.0 si P1.1)
 	PORT_Init ();													// conecteaza perifericele la pini (UART0, UART1) si stabileste tipul pinilor

	LCD_Init();    												// 2 linii, display ON, cursor OFF, pozitia initiala (0,0)
	KEYB_Init();
	UART0_Init(EIGHT_BIT, BAUDRATE_IO);		// initializare UART0  - conectata la USB-UART (P0.0 si P0.1)
	UART1_TxRxEN(1, 1);

	Timer0_Init();  											// initializare Timer 0

 	EA = 1;                         			// valideaza intreruperile
 	SFRPAGE = LEGACY_PAGE;          			// selecteaza pagina 0 SFR
	
	for(i = 0; i < NR_NODURI; i++){				// initializare structuri de date
		retea[i].full = 0;									// initializeaza buffer gol pentru toate nodurile
		retea[i].bufasc[0] = ':';						// pune primul caracter in buffer-ele ASCII ":"
	}

	Afisare_meniu();			   							// Afiseaza meniul de comenzi
	
  	while(1){														// bucla infinita
																
		switch(STARE_NOD){
			case 0:																// nodul este slave, asteapta mesaj de la master	
				switch(RxMesaj(ADR_NOD)){						// asteapta un mesaj de la master
					case TMO:	Error("\n\r Nod SLAVE -> MASTER!");							// anunta ca nodul curent devine master
										TIP_NOD = MASTER;																// nodul curent devine master
										STARE_NOD = 2;																	// trece in starea 2
										i = ADR_NOD;																		// primul slave va fi cel care urmeaza dupa noul master
					
									break;

					case ROK: Afisare_mesaj(); STARE_NOD = 1;				break;	// a primit un mesaj de la master, il afiseaza si trebuie sa raspunda
					case CAN:	Error("\n\r Mesaj incomplet!");				break;	// afiseaza eroare Mesaj incomplet
					case TIP:	Error("\n\r Mesaj nec!");							break;	// afiseaza eroare Tip mesaj necunoscut
					case ESC:	Error("\n\r Eroare SC!")	;						break;	// afiseaza Eroare SC
					default:	Error("\n\r Cod err nec!")	;					break;	// afiseaza cod eroare necunoscut
				}
				
				break;

			case 1:																// nodul este slave, transmite mesaj catre master
					found = 0;												// se initializeaza variabila de gasire a unui nod cu mesaj cu 0
					for(i = 0; i < NR_NODURI; i++){		// se verifica fiecare dintre noduri
						if(retea[i].full == 1){					// cauta sa gaseasca un mesaj util de transmis
							found = 1;
							break;												// daca este gasit, verificarea celorlalte este intrerupta
						}
					}			
					if(found == 1){										//daca gaseste un nod i
						retea[i].bufbin.adresa_hw_dest = ADR_MASTER;					// adresa HW dest este ADR_MASTER
						TxMesaj(i);																						// transmite mesajul catre nodul i
					}
					else{															// daca nu gaseste, construieste un mesaj in bufferul ADR_MASTER
						retea[ADR_MASTER].bufbin.adresa_hw_dest = ADR_MASTER;	// adresa HW dest este ADR_MASTER
						retea[ADR_MASTER].bufbin.adresa_hw_src = ADR_NOD;			// adresa HW src este ADR_NOD
						retea[ADR_MASTER].bufbin.tipmes = POLL_MES;						// tip mesaj = POLL_MES
						TxMesaj(ADR_MASTER);																	// transmite mesajul din bufferul ADR_MASTER
					}
					
					STARE_NOD = 0;										// trece in starea 0, sa astepte un nou mesaj de la master
					
				break;
	
			case 2:																// tratare stare 2 si eventual comutare stare
					do{
							i++;																		// selecteaza urmatorul slave (incrementeaza i)
							if(i == NR_NODURI) i = 0;								// daca se ajunge capat, se reia de la 0
					}
					while(i == ADR_NOD);
					
					retea[i].bufbin.adresa_hw_dest = i;					// adresa HW dest este i
					
					if(retea[i].full == 1) {										// daca in bufferul i se afla un mesaj util, il transmite
						TxMesaj(i);
					}
					else{																				// altfel, construieste un mesaj de interogare in bufferul i
						retea[i].bufbin.adresa_hw_src = ADR_NOD;  // adresa HW src este ADR_NOD
						retea[i].bufbin.tipmes = POLL_MES;				// tip mesaj = POLL_MES
						TxMesaj(i);																// transmite mesajul din bufferul i
					}
															
					STARE_NOD = 3;				 					// trece in starea 3, sa astepte raspunsul de la slave-ul i
					
				break;

			case 3:										 					// nodul este slave, asteapta mesaj de la master	
				switch(RxMesaj(i)){																			// asteapta un raspuns de la slave i
					case TMO:	Error("\n\r Timeout nod ");									// afiseaza eroare Timeout nod i
										if(AFISARE)	UART0_Putch(i + '0');
										break;
					
					case ROK:	Afisare_mesaj();								break;	// a primit un mesaj de la master, il afiseaza si trebuie sa raspunda
					case ERI:	Error("\n\r Err incadrare!");		break;	// afiseaza Eroare incadrare
					case ERA:	Error("\n\r Err adresa!");			break;	// afiseaza Eroare adresa
					case TIP:	Error("\n\r Mesaj nec!!");			break;	// afiseaza Tip mesaj necunoscut
					case OVR:	Error("\n\r Err suprap!");			break;	// afiseaza Eroare suprapunere
					case ESC:	Error("\n\r Eroare SC!");				break;	// afiseaza Eroare SC
					default:	Error("\n\r Cod err nec!");			break;	// afiseaza Eroare necunoscuta
				}
				
				STARE_NOD = 2;										// revine in starea 2 (a primit sau nu un raspuns de la slave, trece la urmatorul slave)
				
				break;
		}
		UserIO();							// apel functie interfata
	}
}
