#ifndef _STABLA_H_
#define _STABLA_H_ 0

#define BACKLOG 5
#define BUFFER_SIZE 8000
#define MAX_DESCRIPTOR 20

char niz_parametara[10][30];

/* struktura koja sadrzi informacije o cvoru */
typedef struct Cvor{
	char prva[20];
	char druga[50];
	char treca[500];
	char cetvrta[500];
	struct Cvor* levo;
	struct Cvor* desno;

}cvor;


typedef struct CvorH{
	char id[25];
	char imsi[16];
	char msc[20];
	char dest[30];
	char pdu[500];
	int poslato;
	int br_paketa;
	int min;
	struct tm* vreme;
	struct CvorH* levo;
	struct CvorH* desno;

}cvorH;


/* struktura cvor sadrzi buffer i pokazivac na sledeci cvor */
typedef struct cvor_liste{
	unsigned char buffer[1000];
	struct cvor_liste* sledeci_cvor;
	int velicinapaketa;
}Cvor_liste;


cvor* kreiraj_cvor(char* prva, char *druga, char* treca, char* cetvrta);
cvor* dodaj_cvor(cvor* stablo, char* prva,char* druga,char* treca, char* cetvrta);
cvor* pronadji_cvor(cvor* stablo, char* prva, char* druga);
cvor* pronadji_cvor_po_vrednosti(cvor* stablo, char* vrednost);
void oslobodi_memoriju(cvor* stablo);
void ispisi_stablo(cvor* stablo);

void error_fatal (char *format, ...);
int hctoi(const char h);
void funkcija_konverzija (int c, char *br,int sirina);
int konverzija_u_dekadno(char* broj);
int koliko_ima_parametara (char* parametri);
int indeks_karaktera(char* parametri, char c, int poc_poz);
void rasparcaj (char* parametri);
void izvuci_parametre(cvor* stablo,char* ime_paketa,char* parametri);
void obradi_broj (char *broj);
int indeks_a0(unsigned char* buffer, int count);
cvorH* dodajIDuHash(cvorH* pocetak, char* id,  char* imsi, char* msc, char* dest, char* pdu);
cvorH* kreiraj_cvorHM(char* id, char* imsi, char* msc, char* dest, char* pdu);
void oslobodi_memorijuHM(cvorH* stablo);
void ispisiHM(cvorH* stablo);
cvorH* min_cvor(cvorH* koren);
cvorH* nadji_cvor_za_hlr (cvorH* pocetak);
cvorH* nadji_cvor_za_forwardSM (cvorH* pocetak);
Cvor_liste* napravi_cvor_liste(unsigned char* paket, int velicina);
void dodaj_u_red(Cvor_liste** pocetak_reda, Cvor_liste** kraj_reda,unsigned char* paket, int velicina);
int uzmi_iz_reda(Cvor_liste** pocetak_reda, Cvor_liste** kraj_reda,unsigned char* paket);
int procitaj_prvi_u_redu (Cvor_liste** pocetak_reda, Cvor_liste** kraj_reda,unsigned char* paket);
void ispisi_red(Cvor_liste* pocetak_reda);
void oslobodi_red(Cvor_liste** pocetak_reda, Cvor_liste** kraj_reda);
cvorH* nadji_poslato_za_insert(cvorH* pocetak);
cvorH* nadji_cvor_br_paketa(cvorH* pocetakHM, int pot_id);
int br_elemenata_uHM (cvorH* pocetak);
void obradi_imsi (char *broj);
#endif