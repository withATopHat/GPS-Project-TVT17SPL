#include "mbed.h"
#include <string.h>
//#include "GSM.h"

Serial gsm(D8, D2);
Serial gps(D14, D15);
Serial pc(USBTX, USBRX);
DigitalOut led(LED1);
Thread gpsthread;
Thread gsmthread;
DigitalIn button(USER_BUTTON);
//DigitalIn button(USER_BUTTON);

Timer ajastin;

//Alku käskyt

char *alku[][2][200] = {
	{{"AT"},			{"OK"}},		//Varmistetaan että modeemi on päällä
	{{"AT+CMEE=2"},		{"OK"}},		//Virhee kirjallisiksi
	{{"AT+CPIN=0000"},	{"Call Ready"}},//Annetaan PIN koodi
	{{"AT+CGREG?"},		{"+CGREG: 0,1"}},//Varmistetaan että on rekisteröitynyt koti verkkoon.
	//{{"AT+CGATT=1"},	{"OK"}},		//Kiinnittää GPRS:n
	//{{"AT+QIMODE=1"},	{"OK"}},		//Aktivoidaan PDP konteksti
	{{"AT+CGACT=1,1"},	{"OK"}},		//Aktivoidaan PDP konteksti
	{{"AT+QIREGAPP"},	{"OK"}},		//Käynnistetään TCP/IP protocolla
	{{"AT+QIACT"},		{"OK"}},		//Aktivoidaan TCP/IP
	{{"AT+QILOCIP"},	{"."}}			//Luetaan oma IP
};

char *lahetys[][2][200] = {
	{{"AT+QIOPEN=\"TCP\",\"193.167.100.74\",80"}, {"CONNECT OK"}}, //Koulun IP
	{{"AT+QISEND"}, {">"}}
};

//Globaaleja muuttujia tiedon siirtämiseksi GPS:ltä GSM:lle.
float Lat = 0.0, Lon = 0.0, oikeaHDOP = 0.0;
int Aika = 0, ID = 0;
char viesti[1000];// = {"GET /~t6heja02/vastaanotto.php?testi=%d HTTP/1.1\r\nHost: www.students.oamk.fi\r\nConnection: close\r\n\r\n\032"};

void GSM_Thread();
void GPS_Thread();
void ledi();
void lahetaLueLoop(bool alku);
void lue(int aika);
void laheta(char *kasky);
void lahetaJaLue(char *kasky, int aika);
int lahetaJaOdota(char *kasky, char *vastaus, int aika);


int main(){
	pc.printf("Aloitus. GSM GPS yhdistelmä.\nAloitetaan säikeet.\n");
	//gsmthread.start(callback(GSM_Thread));
	gpsthread.start(callback(GPS_Thread));
	
	while(true){
		//led = !led;
	}
}

void GSM_Thread(){
	int l = sizeof(alku)/sizeof(alku[0]);
	
	//Käydään läpi alku komentosarja
	for(int i = 0; i < l; i++){
		ledi();
		int j = lahetaJaOdota(alku[i][0][0], alku[i][1][0], 5);
		wait(1);
		if(j != 0 ){
			//pc.printf("Väärä vastaus.");
			i--;
		}
	}
	
	while(true){
		//Odotetaan kunnes GPS saa dataa.
	}
	
	//Yhdistetään TCP/IP yhteys.
	for(int i = 0; i < l; i++){
		ledi();
		int j = lahetaJaOdota(lahetys[i][0][0], lahetys[i][1][0], 5);
		lue(1);
		if(j != 0 ){
			pc.printf("Väärä vastaus.");
			i--;
		}
	}
	
	//Kysytään ID
	
	
	while(true){
		//Lähetetään tietyin väliajoin
	}
}

void GPS_Thread(){
		while(true) {
		//pc.printf("LOOPIN ALKU!\n\n\n");
		char buffer[460];
		for(int i = 0; i< 450; i++)
		{
		
		char c = gps.getc();
		
		buffer[i] = c;
		
		
		}
		
		//pc.puts(buffer);		//tulostaa bufferin raakadataa
	
		//Haluttu input = "$GPGGA,xxxxx.xx,xxxx.xxxxxx,x,xxxxxx.xxxxx,x,x,xx,x.xx,xx.x,x,xx.x,x,,*xx";
		
		char *token;		
		token = strtok(buffer, "$");
		
		//pc.printf("TYHJÄ = %s\n", token);
			
		char* paikannusTieto = {"GPGGA"};
	
		while (true){
			
		bool loyty = false;
		
		token = strtok(NULL, "$");
						
		for(int i = 0; i < 5; i++)
		{
			if (paikannusTieto[i] != token[i])
			{
				//pc.printf("Väärä merkki!\n");
				loyty = true;
				break;
			}
		}
		if (!loyty){
			
		break;				//Takaisin loopin alkuun etsimään "GPGGA"
		}

		}
		while (gps.readable())
		{
		char c = gps.getc();
		}
		token = strtok(token, ",.");
		//pc.printf(" = %s\n", token);
			
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		int fix = atoi(token);
		int UTC = fix + 20000;
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		int nolla = atoi(token);
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		int kulma = (atoi(token));					//JAA 100
		float todellinenKulma = kulma/100;			// 65 !!
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		float minuutit = atoi(token);
		float todellisetMinuutit = minuutit/100000;	//0.0232000
		
		float latitude = todellinenKulma + (todellisetMinuutit / 60);
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		int Korkeus = atoi(token);
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		float a6 = atoi(token);						//2530.000000				
		float LeveysKulma = a6/100;
		LeveysKulma = floor(LeveysKulma);			//Pyöristää kahden desimaalin kokonaisluvuksi (floor=alaspäin pyöristys)
		
		float valivaihe = a6 - (LeveysKulma * 100);	// = 30 MINUUTIT
		
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);
		float a7 = atoi(token);
		float leveydenAika= a7/100000;				// 0,55534
		
		float longitude = LeveysKulma + (valivaihe + leveydenAika) / 60;
		
		token = strtok(NULL, ",.");
		//pc.printf("token = %s\n", token);			//EAST
	//	int leveysSuunta = atoi(token);
		
		token = strtok(NULL, ",.");
	//	pc.printf("token = %s\n", token);
	//	int a9 = atoi(token);						//fix quality
		
		token = strtok(NULL, ",.");
	//	pc.printf("token = %s\n", token);
	//	int a10 = atoi(token);						//number of satellites
		
		token = strtok(NULL, ",.");
	//	pc.printf("token = %s\n", token);
		float HDOP = atoi(token);					//HDOP
		
		token = strtok(NULL, ",.");
	//	pc.printf("token = %s\n", token);			//Altitude from sea level
	//	int a12 = atoi(token);
		
		pc.printf("aika = %d\n longitude = %f\n Latitude = %f HDOP= %f\n\n\n\n",UTC, longitude, latitude, HDOP);
		
		Lat = latitude;
		Aika = UTC;
		Lon = longitude;
		oikeaHDOP = HDOP;
	}
}

void ledi(){
	//
}

//Kaikki mitä pc:lle kirjoitetaan lähetetään GSM:lle ja päinvastoin.
void lahetaLueLoop(bool alku){
	while(true){
		if(pc.readable()){
			ledi();
			while(pc.readable()){
				gsm.putc(pc.getc());
			}
		}
		if(gsm.readable()){
			ledi();
			while(gsm.readable()){
				pc.putc(gsm.getc());
			}
		}
		if(alku && !button){
			ledi();
			wait(0.5);
			if(button)break;
		}
	}
}

void lue(int aika){
	ajastin.start();
	while(true){
		if(gsm.readable()){
			char c = gsm.getc();
			pc.putc(c);
		}
		if(aika <= ajastin.read()){
			ajastin.stop();
			ajastin.reset();
			break;
		}
	}
}

void laheta(char *kasky){
	gsm.printf(kasky);
	gsm.puts("\r");
	//pc.puts("\n");
}

void lahetaJaLue(char *kasky, int aika){
	laheta(kasky);
	ajastin.start();
	while(true){
		if(gsm.readable()){
			char c = gsm.getc();
			pc.putc(c);
		}
		if(aika < ajastin.read()){
			ajastin.stop();
			ajastin.reset();
			break;
		}
	}
}

int lahetaJaOdota(char *kasky, char *vastaus, int aika){
	laheta(kasky);
	int indeksi = 0, pituus = strlen(vastaus);
	ajastin.start();
	
	while(true){
		if(gsm.readable()){
			char c = gsm.getc();
			pc.putc(c);
			if(c == vastaus[indeksi]){
				indeksi++;
			}
			else{
				indeksi = 0;
			}
			if(pituus == indeksi){
				return 0;
			}
			
		}
		if(aika < ajastin.read()){
			ajastin.stop();
			ajastin.reset();
			return -1;
		}
	}
}
