//***********************************************************************************************************
// PROTOCOL: MS / JETON

// TEMA 1:	UART1, RS232(modem radio), ASCII HEX, 9600,8,E,1
// TEMA 2:	UART1, RS232(modem radio), BINAR, 9600,8,E,1
// TEMA 3:	UART1, RS485, ASCII HEX, 115200,9,1
// TEMA 4:	UART1, RS485, BINAR, 115200,9,1
//***********************************************************************************************************

#define TEMA				1				// 1 sau 2 sau 3 sau 4

//***********************************************************************************************************

#define ADR_NOD 			0		// adresa nodului curent - se moodifica dupa caz

#define	MS					0
#define	JT					1
#define PROTOCOL			MS 			// MS sau JT - trebuie ales
#if(PROTOCOL == MS)
	#define MASTER			1
	#define	SLAVE			0
	#define POLL_MES 		0				// mesaj de interogare / raspuns implicit
#elif(PROTOCOL == JT)
	#define JETON			1
	#define NOJET			0
	#define JET_MES 		0				// mesaj de tip jeton / raspuns implicit
#endif

#define USER_MES 			1				// mesaj ASCII - de afisat

#define	TMO	0
#define	ROK	1
#define	ERI	2
#define	ERA	3
#define	CAN	4
#define	TIP	5
#define	OVR	6
#define	ESC	7

//***********************************************************************************************************
#define	RADIO_MODEM		0
#define	CABLU_RS485		1
#if(TEMA == 1 || TEMA == 2)
	#define PHY		RADIO_MODEM
#elif(TEMA == 3 || TEMA == 4)
	#define PHY		CABLU_RS485
#endif

#if(PHY == RADIO_MODEM)
	#define BAUDRATE_COM 	9600		
#elif(PHY == CABLU_RS485)
	#define BAUDRATE_COM 	115200
#endif

#define BAUDRATE_IO 	9600		

#define WAIT	200

#define NR_NODURI		5				// 5 noduri în retea
#define NR_CHAR_MAX	10

typedef struct {
	unsigned char adresa_hw_dest;			// adresa HW nod care primeste mesajul
	unsigned char adresa_hw_src;			// adresa HW nod care transmite mesajul
	unsigned char tipmes;							// tip mesaj: POLL_MES | JET_MES | USER_MES
	unsigned char src;								// sursa mesajului
	unsigned char dest;								// destinatia mesajului
	unsigned char lng;								// numarul de octeti din campul de date
	unsigned char date[NR_CHAR_MAX];				// campul de date
	unsigned char sc;									// suma de control
} mesaj;													// structura de tip mesaj binar

typedef struct {
	mesaj bufbin;							// mesaj in format binar
	unsigned char bufasc[1+2*(NR_CHAR_MAX + 8)];	// mesaj in format ASCII HEX
	unsigned char full;				// buffer binar plin
}nod;										// structura de date de tip nod
