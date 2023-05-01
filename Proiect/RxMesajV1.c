#include <c8051F040.h>	// declaratii SFR
#include <uart1.h>
#include <Protocol.h>
#include <UserIO.h>

extern nod retea[];						// reteaua Master-Slave, cu 5 noduri

extern unsigned char STARE_NOD;		// starea initiala a nodului curent
extern unsigned char TIP_NOD;			// tip nod
extern unsigned char ADR_MASTER;	// adresa nodului master

extern unsigned char timeout;		// variabila globala care indica expirare timp de asteptare eveniment
//***********************************************************************************************************
unsigned char RxMesaj(unsigned char i);				// primire mesaj de la nodul i
unsigned char ascii2bin(unsigned char *ptr);			// functie de conversie 2 caractere ASCII HEX in binar

//***********************************************************************************************************
unsigned char RxMesaj(unsigned char i){					// receptie mesaj															   
	// unsigned char j, ch, sc, adresa_hw_dest, adresa_hw_src, screc, src, dest, lng, tipmes, *ptr;
										if(TIP_NOD == MASTER){						// Daca nodul este master...
												ch = UART1_Getch_TMO(WAIT);			// M: asteapta cu timeout primul caracter al raspunsului de la slave
												if(timeout)				        // M: timeout, terminare receptie
													return TMO;	
												else{													
													retea[i].full=0;                         // M: raspunsul de la slave vine, considera ca mesajul anterior a fost transmis cu succes	
													if(ch != ':')
													{
														do{ 												// M: ignora restul mesajului
															ch = UART1_Getch_TMO(5);															
														}while(timeout)
														return ERI;											
													}
													else{					  // M: adresa HW ASCII gresita, terminare receptie
														ptr = retea[i].bufasc + 1;	  // M: initializare pointer in bufferul ASCII
														*ptr++ = UART1_Getch_TMO(5);      // M: asteapta cu timeout primul caracter ASCII al adresei HW
														if(timeout)											// M: timeout, terminare receptie
															return CAN;
														*ptr-- = UART1_Getch_TMO(5);	  // M: asteapta cu timeout al doilea caracter al adresei HW
														if(timeout)											// M: timeout, terminare receptie
															return CAN;
														adresa_hw_dest = ascii2bin(ptr);
														if(adresa_hw_dest != ADR_NOD)
														{															
															do{ 												// M: ignora restul mesajului
																ch = UART1_Getch_TMO(5);															
															}while(timeout)
															return ERA;		 // M: adresa HW ASCII gresita, terminare receptie
														}
													}	
												}
											}
											else{  // Daca nodul este slave...
												INTOARCERE:
												ch = UART1_Getch_TMO(2*WAIT + ADR_NOD*WAIT);    // S: asteapta cu timeout primirea primului caracter al unui mesaj de la master
													if(timeout)				// S: timeout, terminare receptie
														return TMO;	
													if(ch !=':') goto INTOARCERE;           // S: asteapta sincronizarea cu inceputul mesajului
													ptr = retea[i].bufasc + 1;		// S: initializeaza pointerul in bufferul ASCII
													*ptr++ = UART1_Getch_TMO(5); 	        // S: asteapta cu timeout primul caracter ASCII al adresei HW
													
													if(timeout)										// S: timeout, terminare receptie								
														return CAN;													
													*ptr-- = UART1_Getch_TMO(5);		 // S: asteapta cu timeout al doilea caracter ASCII al adresei HW													
													
													if(timeout)				 // S: timeout, terminare receptie		
														return CAN;
													
													adresa_hw_dest = ascii2bin(ptr);  	// S: determina adresa HW destinatie
														
													if(adresa_hw_dest != ADR_NOD) goto INTOARCERE;  // S: iese doar cand mesajul era adresat acestui slave														
											}												
													ptr++;
												 do{
													 *(++ptr) = UART1_Getch_TMO(5);  	// M+S: pune in bufasc restul mesajului ASCII HEX
													if(timeout)				// M+S: timeout, terminare receptie								
														return CAN;	
												 }while(*ptr != 0x0A)													 
												
												ptr = retea[i].bufasc + 3;  			// M+S: reinitializare pointer in bufferul ASCII
												screc = adresa_hw_dest; 			// M+S: initializeaza screc cu adresa HW dest
												adresa_hw_src = ascii2bin(ptr); 		// M+S: determina adresa HW src
												ptr+=2;
												screc += adresa_hw_src;  			// M+S: aduna adresa HW src
												if(TIP_NOD == SLAVE)
													ADR_MASTER = adresa_hw_src;		// Slave actualizeaza adresa Master
												tipmes = ascii2bin(ptr);			// M+S: determina tipul mesajului
												ptr+=2;
	
												if(tipmes>1) return TIP;			// M+S: cod functie eronat, terminare receptie
														
												screc += tipmes; 				// M+S: ia in calcul in screc codul functiei
	
												if(tipmes != USER_MES) 
												{
													sc = ascii2bin(ptr); 			 //M+S: determina suma de control
													if( sc == screc)
														return ROK;
														return ESC;
													}
													else{					// M+S: Daca mesajul este unul de date
														src = ascii2bin(ptr);		// M+S: determina sursa mesajului
														ptr += 2;
														screc += src; 			// M+S: ia in calcul in screc adresa src
														dest = ascii2bin(ptr); 		// M+S: determina destinatia mesajului
														ptr += 2;
														screc += dest; 			// M+S: ia in calcul in screc adresa dest
														
														if(TIP_NOD==MASTER) 		// Daca nodul este master...
															if(retea[dest].full == 1) 
																return OVR; 	// M: bufferul destinatie este deja plin, terminare receptie
														lng = ascii2bin(ptr); 		// M+S: determina lng
														ptr += 2; 
														screc += lng;    		// M+S: ia in calcul in screc lungimea datelor
														
														if(TIP_NOD==MASTER){  		// Daca nodul este master...
															retea[dest].bufbin.adresa_hw_src = ADR_NOD; 	// M: stocheaza in bufbin adresa HW src	
															retea[dest].bufbin.tipmes = tipmes; 		// M: stocheaza in bufbin tipul mesajului	
															retea[dest].bufbin.src = src; 			// M: stocheaza in bufbin adresa nodului sursa al mesajului	
															retea[dest].bufbin.dest = dest; 		// M: stocheaza in bufbin adresa nodului destinatie al mesajului	
															retea[dest].bufbin.lng = lng; 			// M: stocheaza lng
															
															for(j=0; j<retea[dest].bufbin.lng-1; j++) 	// M: determina un octet de date
															{ 
																temp_data = ascii2bin(ptr);
																retea[dest].bufbin.date[j] = temp_data;																	
																ptr += 2;
																screc += temp_data;          		// M: ia in calcul in screc octetul de date
															}
																sc = ascii2bin(ptr); // M: determina suma de control
																retea[dest].bufbin.sc = sc;			// M: pune sc in bufbin
																if(sc==screc)
																{
																	retea[dest].full = 1 ; 			// M: mesaj corect, marcare buffer plin
																	return ROK;
																}		
																else
																{
																	return ESC;  			 	// M: eroare SC, terminare receptie
																}
														}
														else{ 								// Daca nodul este slave ...
															retea[ADR_NOD].bufbin.src = src;
															retea[ADR_NOD].bufbin.lng = lng;
															for(j=0; j<retea[ADR_NOD].bufbin.lng-1; j++) 		// M: determina un octet de date
															{ 
																temp_data = ascii2bin(ptr);
																retea[ADR_NOD].bufbin.date[j] = temp_data;																	
																ptr += 2;
																screc += temp_data;          			// M: ia in calcul in screc octetul de date
															}
																						// S: stocheaza la destsrc codul nodului sursa al mesajului	
																				        	// S: stocheaza lng
																	
																						// S: determina un octet de date
																		
																						// S: ia in calcul in screc octetul de date																	
															
															sc = ascii2bin(ptr);					// S: determina suma de control
																		
															if(sc==screc)				
															{
																retea[ADR_NOD].full = 1 ;			// S: mesaj corect, marcare buffer plin
																return ROK;
															}
															else
															{																	
																return ESC;					// S: eroare SC, terminare receptie
															}	
															

														}
														sc = ascii2bin(ptr);						// M+S: determina suma de control
														if(sc!=screc)
															return ESC;							// M+S: eroare SC, terminare receptie
												}			
																
														
			return TMO;													
}															


//***********************************************************************************************************
unsigned char ascii2bin(unsigned char *ptr){			// converteste doua caractere ASCII HEX de la adresa ptr
	unsigned char bin;
	
	if(*ptr > '9') bin = (*ptr++ - 'A' + 10) << 4;	// contributia primului caracter daca este litera
	else bin = (*ptr++ - '0') << 4;									// contributia primului caracter daca este cifra
	if(*ptr > '9') bin  += (*ptr++ - 'A' + 10);			// contributia celui de-al doilea caracter daca este litera
	else bin += (*ptr++ - '0');											// contributia celui de-al doilea caracter daca este cifra
	return bin;
}


