#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "stabla.h"
#include <mysql/mysql.h>
#include <time.h>



static pthread_mutex_t mutex;	/* Mutex kojim se sinhronizuje pristup
char poruka[150];
/* stabla za implementaciju hash mapa*/
cvor* messageClassValues=NULL;
cvor* ParametarValue=NULL;
cvor* MessageTypeValue=NULL;
cvor* ParametarOrder=NULL;
cvorH* pocetakHM=NULL;
static char *program;		/* Ime programa. */

int ind1, ind2;
int brzanit;
int stid;
int invokeID;
int SLS;


Cvor_liste* pocetak_reda=NULL;
Cvor_liste* kraj_reda=NULL;
/*    poc_msc=i+1;
    duzina_msc=buffer[i-1]-1;
 * Funkcija error_fatal() ispisuje poruku o gresci i potom prekida program.
 */

void obradi (char* buffer, int count, char* ime_paketa)
{
  
  char class[20];
  char type[20];
  int i;
          printf("Primljen je paket velicine %d\n", count);
          sprintf(class, "%d", buffer[2]);
	  class[1]='\0';
	  sprintf(type, "%d", buffer[3]);
	  type[1]='\0';	  
	  cvor* pomocni = pronadji_cvor (MessageTypeValue, class , type);
	  strcpy(ime_paketa, pomocni->treca);
          printf("Tip paketa je  %s\n", ime_paketa);
}
void salje_paket (char* ime_paketa, int server, int PORT)
{
  
    char cdata[80];
    char br[30];
    char parametri[500];
    cvor *pomocni = pronadji_cvor_po_vrednosti(MessageTypeValue, ime_paketa);  
   
    /*Version*/
    funkcija_konverzija(1, br, 2);
    strcpy(cdata, br);
    /*Reserved*/
    funkcija_konverzija(0, br, 2);
    strcat(cdata, br);     
    /*Message class*/
    funkcija_konverzija(atoi(pomocni->prva), br, 2);
    strcat(cdata, br);
     /*Message Type*/
    funkcija_konverzija(atoi(pomocni->druga), br, 2);
    strcat(cdata, br);     
    /*Message length*/
    if (strcmp(ime_paketa, "ASPUP")==0)
    {
    funkcija_konverzija(36, br,8);
    }
    if (strcmp(ime_paketa, "ASPAC")==0)
    {
    funkcija_konverzija(24, br,8);
    }
    if (strcmp(ime_paketa, "DAUD")==0)
    {
    funkcija_konverzija(16, br,8);
    }
    strcat(cdata, br);
    
       /* zaglavlje za parametre*/    
     izvuci_parametre(ParametarOrder,ime_paketa, parametri);
     rasparcaj (parametri);

     int pom;
    for (pom=0; pom<koliko_ima_parametara(parametri); pom++)
     {
       /*Parametar tag*/
    pomocni = pronadji_cvor_po_vrednosti(ParametarValue, niz_parametara[pom]); 
    funkcija_konverzija(konverzija_u_dekadno(pomocni->prva), br, 4);
    strcat(cdata, br);
    /*Parametar length*/
    if (strcmp(niz_parametara[pom], "InfoString")==0)
    {
    funkcija_konverzija(18, br,4);
    strcat(cdata, br);
        /*Info String*/
    strcat(cdata, "537175697265277320415350210a");
    }
      else
      {
    funkcija_konverzija(8, br,4);
    strcat(cdata, br);
      }
      
/* treci deo zaglavlja za parametar*/

if (strcmp(niz_parametara[pom], "ASPIdentifier")==0)
{
    funkcija_konverzija(0, br,8);
    strcat(cdata, br);
}
    
     if (strcmp( niz_parametara[pom], "TrafficModeType")==0)
     {
    funkcija_konverzija(1, br,8);
    strcat(cdata, br);
     }
    
    
    /* ovde treba voditi racuna, na jednu adresu malte se salje RC=100,  a na drugu (194) se salje RC=101*/
    if (strcmp( niz_parametara[pom], "RoutingContext")==0)
    {
      if(PORT == 2905)
	  funkcija_konverzija(100, br,8);
      else if (PORT == 2906)
	  funkcija_konverzija(101, br,8);
      else
	  funkcija_konverzija(100, br,8);
    strcat(cdata, br);
    }
    
       if (strcmp( niz_parametara[pom], "AffectedPointCode")==0)
       {
    funkcija_konverzija(0, br,2);
    strcat(cdata, br);
        /* ovde treba voditi racuna, na jednu adresu malte se salje APC=8460,  a na drugu se salje APC=8461*/
      if(PORT == 2905)
	  funkcija_konverzija(8460, br,6);
      else if (PORT == 2906)
	  funkcija_konverzija(8461, br,6);
      else
	  funkcija_konverzija(8460, br,6);
    strcat(cdata, br);
       }
 } 
    if (strcmp(ime_paketa, "ASPUP")==0)
    {
    /*Padding*/
    funkcija_konverzija(0, br,6);
    strcat(cdata, br);
    }
    else
        {
    /*Padding*/
    funkcija_konverzija(0, br,2);
    strcat(cdata, br);
    }    
    
    unsigned char udata[(strlen(cdata)-1)/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      int count = sizeof (udata);
      int i;
      if (send(server, udata, count, 0) != count)
	error_fatal ("%s send() error\n", "client"); 
}
void  izgradi_stek(int server, int PORT)
{
    int count;
    char buffer[BUFFER_SIZE];
    char ime_paketa[15];
    
    salje_paket("ASPUP", server, PORT);
  
    printf("Poslat prvi paket\n");
    if ((count = recv (server, buffer, 200, 0)) < 0)
	error_fatal ("%s readline() error\n", program);
      buffer[count] = 0;
    printf("Primljen prvi paket\n");
    obradi(buffer, count, ime_paketa);
    
    if (strcmp(ime_paketa, "ASPUP_ACK")==0)
    {
	printf("primili smo ASPUP_ACK paket\n");
    }

      else
      {
	  printf("Ocekivan je ASPUP_ACK paket\n"); 
	  exit(1);
      }
       salje_paket("ASPAC", server, PORT);

 if ((count = recv (server, buffer, 200, 0)) < 0)
	error_fatal ("%s readline() error\n", program);
      buffer[count] = 0;    
    obradi(buffer, count, ime_paketa);
 
    if (strcmp(ime_paketa, "NTFY")==0)
	printf("primili smo NTFY paket\n");
      else
      {
	  printf("Ocekivan je NTFY paket\n"); 
	  exit(1);
      }
       if ((count = recv (server, buffer, 200, 0)) < 0)
	error_fatal ("%s readline() error\n", program);
      buffer[count] = 0;    
    obradi(buffer, count, ime_paketa);
    if (strcmp(ime_paketa, "ASPAC_ACK")==0)
	printf("primili smo ASPAC_ACK paket\n");
      else
      {
	  printf("Ocekivan je ASPAC_ACK paket\n"); 
	  exit(1);
      }
       if ((count = recv (server, buffer, 200, 0)) < 0)
	error_fatal ("%s readline() error\n", program);
      buffer[count] = 0;
      printf("primili paket\n");
    
    obradi(buffer, count, ime_paketa);
    if (strcmp(ime_paketa, "NTFY")==0)
	printf("primili smo NTFY paket\n");
      else
      {
	  printf("Ocekivan je NTFY paket\n"); 
	  exit(1);
      }
      /*
      salje_paket("DAUD", server, PORT);
       if ((count = recv (server, buffer, 200, 0)) < 0)
	error_fatal ("%s readline() error\n", program);
      buffer[count] = 0;
      printf("primili paket\n");
    
    obradi(buffer, count, ime_paketa);
    if (strcmp(ime_paketa, "DAVA")==0)
	printf("primili smo DAVA paket\n");
      else
      {
	  printf("Ocekivan je DAVA paket\n"); 
	  exit(1);
      }   
     */

      return;
}

int pocetak_ID (unsigned char* buffer, int count, int indikator)
{
  
  // indikator 1 -> paket od clienta
  //indikator 2 -> paket od operatora
  
 int poz=-1;
 int i;
 char br[5];
 for(i=0; i<count; i++)
 {
     if((indikator==1) && (buffer[i+1]==04) && (buffer[i]==72))
   break;
     if((indikator==2) && (buffer[i+1]==04) && (buffer[i]==73))
   break;
 }
 
 return i;
  
}

void uzmiIDpaketa (unsigned char* buffer, int count, char *dtid, int indikator)
{
  
  int poz=pocetak_ID(buffer, count, indikator);
  int i;
  char br[5];
  for(i=poz+2; i<poz+6; i++)
  {
	funkcija_konverzija(buffer[i], br,2);
	if (i==poz+2)
	  strcpy(dtid, br);
	else
	  strcat(dtid, br);
     }
  
     printf("dtid: %s\n", dtid); 
   
   
  
}

void DATA (char* broj, char* br_servera, int server, int PORT)
{
  
    char cdata[250];
    char br[20];
  
    cvor *pomocni = pronadji_cvor_po_vrednosti(MessageTypeValue, "DATA");  
    /*Version*/
    funkcija_konverzija(1, br, 2);
    strcpy(cdata, br);
    
    /*Reserved*/
    funkcija_konverzija(0, br, 2);
    strcat(cdata, br);
    /*Message class*/
    funkcija_konverzija(atoi(pomocni->prva), br, 2);
    strcat(cdata, br);
    /*Message Type*/
    funkcija_konverzija(atoi(pomocni->druga), br, 2);
    strcat(cdata, br);
    
// polje koje nema konstantnu vrednost
    /*Message length*/
    funkcija_konverzija(128, br,8);
    strcat(cdata, br);
    
     /*Parametar tag*/
    funkcija_konverzija(528, br, 4);
    strcat(cdata, br);
// polje koje nema konstantnu vrednost   
    /*Parametar length*/
    funkcija_konverzija(119, br,4);
        strcat(cdata, br);
	/* OPC */
    funkcija_konverzija(5050, br,8);
        strcat(cdata, br);
	/* DPC */
	if (PORT == 2905)
	  funkcija_konverzija(8080, br, 8);
      else
	  funkcija_konverzija(8080, br, 8);

        strcat(cdata, br);
	/* SI */
    funkcija_konverzija(3, br,2);
        strcat(cdata, br);
	/* NI */
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/*MP */
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/* SLS*/
     funkcija_konverzija(SLS, br,2);
     SLS++;
        strcat(cdata, br);
	

	
    /*Message Type*/
    funkcija_konverzija(9, br,2);
    strcat(cdata, br);
     /*Message class*/
    funkcija_konverzija(128, br,2);
    strcat(cdata, br);
//polja koja oznacavaju pocetak sledece sekcije
     /*Parameter to First Mandatoty var*/
     //ovo bi trebalo da je const
    funkcija_konverzija(3, br,2);
    strcat(cdata, br);
      /*Parameter to Second Mandatoty var*/
      // nastaje kada se na gornju 3 doda 11 bajtova sto j duzina zaglavlja za podatke o broju za koji se HLR radi
    funkcija_konverzija(14, br,2);
    strcat(cdata, br);
      /*Parameter to third Mandatoty var*/
      // i ova vrednost nastaje dodavanjem 11 ali to je uvek Malta pa je const
    funkcija_konverzija(25, br,2);
    strcat(cdata, br);
    
    
    /*duzina zaglavlja za Called */
    funkcija_konverzija(11, br, 2);
    strcat(cdata, br);
    
    /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* HLR 6*/
    funkcija_konverzija(6, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
      obradi_broj(broj);
      strcat(cdata, broj);
      
      /*duzina zaglavlja za Calling */

      funkcija_konverzija(11, br,2);
      strcat(cdata, br);
    
     /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* MSC 8*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
    funkcija_konverzija(17, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
       obradi_broj(br_servera);
      strcat(cdata, br_servera);
      obradi_broj(br_servera);

      
      // duzina TCAP-a i GSM MAP-a zajedno
// ovo nije const cifra
       funkcija_konverzija(73, br,2);
	 strcat(cdata, br);
	  
	  
    strcat(cdata, "6247");
    /* Source transaction ID*/
    strcat(cdata, "4804");
     funkcija_konverzija(stid, br,8);
	 strcat(cdata, br);
	 stid++;
	 printf("dtid: %d\n", stid);
  
    
    // je ne sais pas sta je ovo :/
     strcat(cdata, "6b1e281c0607");
     /* oid */
     strcat(cdata, "00118605010101");
     
     strcat (cdata, "a011600f");
     
     /* dialogue request*/
     strcat(cdata, "80020780a109060704000001001402");
      strcat (cdata, "6c");
      
      
  // duzina    GSM MAP-a
       strcat (cdata, "1f");
      /*GSM Mobile Application */
      
     strcat(cdata, "a11d0201");
     
     /* invokeID*/
      funkcija_konverzija(invokeID, br,2);
      invokeID++;
      strcat(cdata, br);
      
      /* localValue */
      strcat(cdata, "0201");
      
      /* send routing info for sm*/
       funkcija_konverzija(45, br,2);
    strcat(cdata, br);
      
    strcat(cdata, "3015800791");
    /*adress digits*/
    strcat(cdata, broj);
    strcat(cdata, "8101ff8207");
    
    /*service centre adress*/
    funkcija_konverzija(145, br,2);
    strcat(cdata, br);
   strcat(cdata, "1444950611f0");
     
   funkcija_konverzija(0, br,2);
      strcat(cdata, br);
    
    unsigned char udata[(strlen(cdata)-1)/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
      int count = sizeof (udata);
      if (send(server, udata, count, 0) != count)
	error_fatal ("%s send() error\n", "client"); 
      
          
     
}



void gsm_pdu(unsigned char* cdata, char* dest, char* gt_number)
{
  
    char br[10];

    /*GSM Mobile Application */ 
     strcpy (cdata, "a1");
     funkcija_konverzija(17+strlen(dest)/2+strlen(gt_number)/2+strlen(dest)%2+strlen(gt_number)%2, br,2);
     strcat (cdata, br);
     strcat(cdata, "0201");
     /* invokeID*/
      funkcija_konverzija(invokeID, br,2);
      invokeID++;
      strcat(cdata, br);
      /* localValue */
      strcat(cdata, "0201");
      /* send routing info for sm*/
      funkcija_konverzija(45, br,2);
      strcat(cdata, br); 
      strcat(cdata, "30");
      funkcija_konverzija(9+strlen(dest)/2+strlen(gt_number)/2+strlen(dest)%2+strlen(gt_number)%2, br,2);
      strcat (cdata, br);
      strcat(cdata, "80");
      funkcija_konverzija(1+strlen(dest)/2+strlen(dest)%2, br,2);
      strcat (cdata, br);
      strcat(cdata, "91");
      /*adress digits*/
      char dest2[15];
      strcpy(dest2,dest);
      obradi_broj(dest2);
      strcat(cdata,dest2);
      strcat(cdata, "81010082");
      funkcija_konverzija(1+strlen(gt_number)/2+strlen(gt_number)%2, br,2);
      strcat (cdata, br); 
      /*service centre adress*/
      funkcija_konverzija(145, br,2);
      strcat(cdata, br);
      char gt_num2[15];
      strcpy(gt_num2, gt_number);
      obradi_broj(gt_num2);
      cdata=strcat(cdata, gt_num2);
     /* printf(" gsm pdu:\n");
       unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
      int i;
      for(i=0; i<(strlen(cdata))/2; i++)
	printf("%x ", udata[i]);
           printf(" gsm pdu:\n");*/

      }
    
void tcap_pdu (unsigned char* cdata,char* dest, char* gt_number)
{
  unsigned char temp_buffer[100];
  gsm_pdu(temp_buffer, dest, gt_number);
  char br[10];
  
    strcpy(cdata, "62");
    funkcija_konverzija(9+(strlen(temp_buffer)-1)/2, br,2);
    strcat(cdata, br);
    /* Source transaction ID*/
    strcat(cdata, "4804");
    funkcija_konverzija(stid, br,8);
    strcat(cdata, br);
    stid++;
    printf("dtid: %d\n", stid);
    strcat (cdata, "6c");   
  // duzina    GSM MAP-a
    funkcija_konverzija(1+(strlen(temp_buffer)-1)/2, br,2);
    strcat (cdata, br);
    strcat(cdata, temp_buffer); 
    
  /*  printf(" tcap pdu:\n");
       unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
      int i;
      for(i=0; i<(strlen(cdata))/2; i++)
	printf("%x ", udata[i]);
          printf(" tcap pdu:\n");*/

  
}

void sccp_pdu (unsigned char* cdata, char* dest, char* gt_number)
{
  
  unsigned char temp_buffer[200];
  tcap_pdu(temp_buffer, dest, gt_number);
  
  char br[10];

  
    /*Message Type*/
    funkcija_konverzija(9, br,2);
    strcpy(cdata, br);
     /*Message class*/
    funkcija_konverzija(129, br,2);
    strcat(cdata, br);
    //polja koja oznacavaju pocetak sledece sekcije
     /*Parameter to First Mandatoty var*/
     //ovo bi trebalo da je const
    funkcija_konverzija(3, br,2);
    strcat(cdata, br);
      /*Parameter to Second Mandatoty var*/
// nastaje kada se na gornju 3 doda 11 bajtova sto j duzina zaglavlja za podatke o broju za koji se HLR radi
funkcija_konverzija(3+5+(strlen(dest)+1)/2, br,2);
strcat(cdata, br);
/*Parameter to third Mandatoty var*/
// i ova vrednost nastaje dodavanjem 11 ali to je uvek Malta pa je const
funkcija_konverzija(3+10+(strlen(dest)+1)/2+(strlen(gt_number)+1)/2, br,2);
strcat(cdata, br);
    /*duzina zaglavlja za Called */
funkcija_konverzija(5+(strlen(dest)+1)/2, br, 2);
strcat(cdata, br);
    
    /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* HLR 6*/
    funkcija_konverzija(6, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
     if(strlen(dest)%2)
    funkcija_konverzija(17, br,2);
    else
      funkcija_konverzija(18, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
     char dest2[15];
      strcpy(dest2,dest);
      obradi_broj_bez_f(dest2);
      strcat(cdata,dest2);      
      /*duzina zaglavlja za Calling */

funkcija_konverzija(5+(strlen(gt_number)+1)/2, br,2);
strcat(cdata, br);
    
     /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* MSC 8*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
    funkcija_konverzija(17, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
      char gt_num2[15];
      strcpy(gt_num2, gt_number);
      obradi_broj_bez_f(gt_num2);
      strcat(cdata, gt_num2);
      // duzina TCAP-a i GSM MAP-a zajedno
// ovo nije const cifra
   funkcija_konverzija(strlen(temp_buffer)/2, br,2);
   strcat (cdata, br);
   strcat(cdata, temp_buffer); 
	  
 /*  printf(" sccp pdu:\n");
    unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
      int i;
      for(i=0; i<(strlen(cdata))/2; i++)
	printf("%x ", udata[i]);
      printf(" sccp pdu:\n");*/
}

void m3ua_pdu(char* dest, char* gt_number, int server, int RC, int OPC, int DPC, int NI)
{
   char br[20];
    unsigned char temp_buffer[200];
    unsigned char cdata[1000];

    sccp_pdu(temp_buffer, dest, gt_number);
    cvor *pomocni = pronadji_cvor_po_vrednosti(MessageTypeValue, "DATA");  
    /*Version*/
    funkcija_konverzija(1, br, 2);
    strcpy(cdata, br);
    
    /*Reserved*/
    funkcija_konverzija(0, br, 2);
    strcat(cdata, br);
    /*Message class*/
    funkcija_konverzija(atoi(pomocni->prva), br, 2);
    strcat(cdata, br);
    /*Message Type*/
    funkcija_konverzija(atoi(pomocni->druga), br, 2);
    strcat(cdata, br);
    
// polje koje nema konstantnu vrednost
    /*Message length*/
funkcija_konverzija(104, br,8);
strcat(cdata, br);
    
    /*Parametar tag for RoutingContext*/
    funkcija_konverzija(6, br, 4);
    strcat(cdata, br);
    /*Parametar length*/
    funkcija_konverzija(8, br, 4);
    strcat(cdata, br);
    funkcija_konverzija(RC, br, 8);
    strcat(cdata, br);
    
    
     /*Parametar tag*/
    funkcija_konverzija(528, br, 4);
    strcat(cdata, br);
// polje koje nema konstantnu vrednost   
    /*Parametar length*/
funkcija_konverzija(16+strlen(temp_buffer)/2, br,4);
strcat(cdata, br);
	/* OPC */
    funkcija_konverzija(OPC, br,8);
        strcat(cdata, br);
	/* DPC */
    funkcija_konverzija(DPC, br, 8);
        strcat(cdata, br);
	/* SI */
    funkcija_konverzija(3, br,2);
        strcat(cdata, br);
	/* NI */
     funkcija_konverzija(NI, br,2);
        strcat(cdata, br);
	/*MP */
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/* SLS*/
     funkcija_konverzija(SLS, br,2);
     strcat(cdata, br);
     SLS++;
     if(SLS==9)
       SLS=1;
     
    strcat(cdata, temp_buffer); 
    
    /* padding bytes*/
    int sirina=104-32-strlen(temp_buffer)/2;
    funkcija_konverzija(0, br,sirina*2);
     strcat(cdata, br);
    
	  
  // printf(" m3ua pdu:\n");
    unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
    /*  int i;
      for(i=0; i<(strlen(cdata))/2; i++)
	printf("%x ", udata[i]);
      printf(" m3ua pdu:\n");*/
        int count = sizeof (udata);
      if (send(server, udata, count, 0) != count)
	error_fatal ("%s send() error\n", "client"); 
	
}

void posalji_sm(int server,char* imsi,char* msc,char* pdu, char* br_malte)
   {
    	  
    char cdata[400];
    char br[30];
    int OPC=5050;
    int DPC=8080;
    
    cvor *pomocni = pronadji_cvor_po_vrednosti(MessageTypeValue, "DATA");  
    /*Version*/
    funkcija_konverzija(1, br, 2);
    strcpy(cdata, br);
    /*Reserved*/
    funkcija_konverzija(0, br, 2);
    strcat(cdata, br);
    /*Message class*/
    funkcija_konverzija(atoi(pomocni->prva), br, 2);
    strcat(cdata, br);
    /*Message Type*/
    funkcija_konverzija(atoi(pomocni->druga), br, 2);
    strcat(cdata, br);
    /*Message length*/
    
 // nije ovo fiksna duzina, zavisi od PDU-a
 int parLen=131+strlen(pdu)/2;
    funkcija_konverzija(156, br,8);
    strcat(cdata, br);

     /*Parametar tag*/
    funkcija_konverzija(528, br, 4);
    strcat(cdata, br);
 //nije fiksne duzine, zavisi od PDU-a   
    /*Parametar length*/
    funkcija_konverzija(147, br,4);
        strcat(cdata, br);
	/* OPC */
    funkcija_konverzija(OPC, br,8);
        strcat(cdata, br);
	/* DPC */
    funkcija_konverzija(DPC, br, 8);
        strcat(cdata, br);
	/* SI */
    funkcija_konverzija(3, br,2);
        strcat(cdata, br);
	/* NI */
	//Malta 2
	//Fink 0
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/*MP */
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/* SLS*/
     funkcija_konverzija(SLS, br,2);
     SLS++;
     if (SLS==15)
       SLS=2;
        strcat(cdata, br);
	

    /*Message Type*/
    funkcija_konverzija(9, br,2);
    strcat(cdata, br);
     /*Message class*/
    funkcija_konverzija(128, br,2);
    strcat(cdata, br);
     /*Parameter to First Mandatoty var*/
    funkcija_konverzija(3, br,2);
    strcat(cdata, br);
      /*Parameter to Second Mandatoty var*/
    funkcija_konverzija(14, br,2);
    strcat(cdata, br);
      /*Parameter to third Mandatoty var*/
    funkcija_konverzija(25, br,2);
    strcat(cdata, br);
    
    /* je ne sais pas */
    funkcija_konverzija(11, br,2);
    strcat(cdata, br);
    
    /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* MSC 8*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
    funkcija_konverzija(17, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
       
     obradi_broj(msc);
  // MSC mozda baguje!!!!!!!!!!!!!!
      strcat(cdata, msc);
           obradi_broj(msc);


          /* je ne sais pas */
      funkcija_konverzija(11, br,2);
      strcat(cdata, br);
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* MSC 8*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
      

	 
     /*Numbering plan */
    funkcija_konverzija(17, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
   // funkcija_konverzija(381641499068, br,12);
      obradi_broj(br_malte);
  strcat(cdata, "836100169400");
      //strcat(cdata, br_malte);
    
   strcat(cdata, "65");
   
   //TCAP
   
   strcat(cdata, "62");
  // nije const
      strcat(cdata, "63");
    
    /* Source transaction ID*/
    strcat(cdata, "4804");
     funkcija_konverzija(stid, br,8);
     // strcat(cdata, br);
      strcat(cdata, "02000000");
      stid++;
   /* //destination transaction id
     strcat(cdata, "4904041e35f9");
     
      strcat(cdata, "6c");
     //ovo nije const ili bar ne bi trebalo :) 
     // trebalo bi da stoji ovaj red
      //funkcija_konverzija(33+strlen(pdu)/2, br,2);
       strcat(cdata, "39");
     */
     //strcat(cdata, "4a");
     
    strcat(cdata, "6b1e281c060700118605010101a011600f80020780a1090607040000010019026c3b");

     
     
     /* GSM Mobile Application */
     
     strcat (cdata, "a1");
     
     
     //funkcija_konverzija(31+strlen(pdu)/2, br,2);
      strcat(cdata, "39");
     
     strcat (cdata,"0201");
     //incvoke ID
      strcat (cdata,"02");
     strcat(cdata, "02012e30");
     // pretposlednje 2e je operation code
     
     // funkcija_konverzija(23+strlen(pdu)/2, br,2);
      strcat(cdata, "31");
     
     strcat(cdata, "8008");
     // 08 je duzina imsi ja
     
     /* TBCD digits */
     //obradi_broj(imsi);
     strcat(cdata, imsi);
     
     // 07 je od duzine tog dela gde se br operatora stavlja, 91 je deo Maltinnog broja
      strcat (cdata, "840791");
      
            //obradi_broj(br_malte);

       //strcat(cdata, br_malte);
     strcat (cdata, "5396490125f4");
     strcat (cdata, "04");
     /* duzina pdu-a*/
     funkcija_konverzija(strlen(pdu)/2, br,2);
         strcat (cdata, br);
      strcat(cdata, pdu);
      
      funkcija_konverzija(5, br,2);
      strcat(cdata, br);
      
        
    funkcija_konverzija(0, br,4);
      strcat(cdata, br);
            
      unsigned char udata[(strlen(cdata)-1)/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
      int count2 = sizeof (udata);
      if (send(server, udata, count2, 0) != count2)
	error_fatal ("%s send() error\n", "client"); 
	  
	
     
   }


   
void gsm_forwardSM(unsigned char* cdata, char* gt_number, char* imsi, unsigned char* pdu)
  {
          
      char br[10];

     /* GSM Mobile Application */
       strcpy (cdata, "a1");  
     funkcija_konverzija(29+strlen(pdu)/2, br,2);
     strcat(cdata, br);
     strcat (cdata,"0201");
     //incvoke ID
      strcat (cdata,"02");
     strcat(cdata, "02012e30");
     // pretposlednje 2e je operation code
     funkcija_konverzija(21+strlen(pdu)/2, br,2);
      strcat(cdata, br);
     
     strcat(cdata, "80");
      funkcija_konverzija(8, br,2);
      strcat(cdata, br);
      

       
     // 08 je duzina imsi ja
     char imsi2[20];
    strcpy(imsi2, imsi);
    obradi_imsi(imsi2);
    printf("%s imsi 2  %d ------------------\n", imsi2, (strlen(imsi)+1)/2);
    strcat(cdata,imsi2);

     // 07 je od duzine tog dela gde se br operatora stavlja, 91 je deo Maltinnog broja
      strcat (cdata, "84");
      funkcija_konverzija(1+(strlen(gt_number)+1)/2, br,2);
      strcat(cdata, br);
      strcat(cdata,"91");
      
            //obradi_broj(br_malte);

       //strcat(cdata, br_malte);
    char gt_num2[20];
    strcpy(gt_num2, gt_number);
    obradi_broj(gt_num2);
    strcat(cdata, gt_num2);
     strcat (cdata, "04");
     /* duzina pdu-a*/
     funkcija_konverzija(strlen(pdu)/2, br,2);
         strcat (cdata, br);
      strcat(cdata, pdu);
   
      int i;
      printf("---------------------    gsm layer      %d -----------------\n", strlen(cdata));
    unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
     for(i=0; i<(strlen(cdata))/2; i++)
       printf("%x ", udata[i]);
     printf("\n");
   }
   
void gsm2_forwardSM(unsigned char* cdata, char* gt_number, char* imsi, unsigned char* pdu)
  {
          
      char br[10];
      printf("gsm2 fjan----------\n");
      
      int len1, len2, len3;
      
      len1=strlen(pdu)/2;
      len2=20+len1;
      if(len1<128)
	len2++;
      else if (len1<256)
	len2+=2;
      else
	len2+=3;
      
      len3=7+len2;
        if(len2<128)
	len3++;
      else if (len2<256)
	len3+=2;
      else
	len3+=3;      
     /* GSM Mobile Application */
       strcpy (cdata, "a1");  
      
     if(len3<128)
     {
     funkcija_konverzija(len3, br,2);
     strcat (cdata, br);
     }
     else if (len3<256)
     {
     strcat (cdata, "81");  
     funkcija_konverzija(len3, br,2);
     strcat (cdata, br);
     }
     else
     {
     strcat (cdata, "82");  
     funkcija_konverzija(len3/0xff , br,2);
     strcat (cdata, br);
     funkcija_konverzija(len3%0xff , br,2);
     strcat (cdata, br); 
     } 
     strcat (cdata,"0201");
     //incvoke ID
      strcat (cdata,"02");
     strcat(cdata, "02012e30");
     // pretposlednje 2e je operation code
     
    
     if(len2<128)
     {
     funkcija_konverzija(len2, br,2);
     strcat (cdata, br);
     }
     else if (len2<256)
     {
     strcat (cdata, "81");  
     funkcija_konverzija(len2, br,2);
     strcat (cdata, br);
     }
     else
     {
     strcat (cdata, "82");  
     funkcija_konverzija(len2/0xff , br,2);
     strcat (cdata, br);
     funkcija_konverzija(len2%0xff , br,2);
     strcat (cdata, br); 
     } 
      
      
      
     strcat(cdata, "80");
      funkcija_konverzija(8, br,2);
      strcat(cdata, br);
      

       
     // 08 je duzina imsi ja
     char imsi2[20];
    strcpy(imsi2, imsi);
    obradi_imsi(imsi2);
    printf("%s imsi 2  %d ------------------\n", imsi2, (strlen(imsi)+1)/2);
    strcat(cdata,imsi2);

     // 07 je od duzine tog dela gde se br operatora stavlja, 91 je deo Maltinnog broja
      strcat (cdata, "84");
      funkcija_konverzija(1+(strlen(gt_number)+1)/2, br,2);
      strcat(cdata, br);
      strcat(cdata,"91");
      
    char gt_num2[20];
    strcpy(gt_num2, gt_number);
    obradi_broj(gt_num2);
    strcat(cdata, gt_num2);
     
    
    strcat (cdata, "04");
     
     /* duzina pdu-a*/
     if(len1<128)
     {
     funkcija_konverzija(len1, br,2);
     strcat (cdata, br);
     }
     else if (len1<256)
     {
     strcat (cdata, "81");  
     funkcija_konverzija(len1, br,2);
     strcat (cdata, br);
     }
     else
     {
     strcat (cdata, "82");  
     funkcija_konverzija(len1/0xff , br,2);
     strcat (cdata, br);
     funkcija_konverzija(len1%0xff , br,2);
     strcat (cdata, br); 
     }
     strcat(cdata, pdu);
   }
   

   

 void reverse_str (unsigned char* str)
 {
   int i,j;
   char temp;
   
   i = 0;
   j = strlen(str) - 1;
 
   while (i < j) {
      temp = str[i];
      str[i] = str[j];
      str[j] = temp;
      i++;
      j--;
   }
 
   
 }
   
   
void gsm3_forwardSM(unsigned char* cdata, char* gt_number, char* imsi, unsigned char* pdu)
  {
          
      char br[10];
      char pomocni1[1000];
      char pomocni2[1000];
     
      
       
     strcpy(pomocni1, "80");
      funkcija_konverzija(8, br,2);
      strcat(pomocni1, br);
     // 08 je duzina imsi ja
     char imsi2[20];
    strcpy(imsi2, imsi);
    obradi_imsi(imsi2);
    printf("%s imsi 2  %d ------------------\n", imsi2, (strlen(imsi)+1)/2);

    strcat(pomocni1,imsi2);

     // 07 je od duzine tog dela gde se br operatora stavlja, 91 je deo Maltinnog broja
      strcat (pomocni1, "84");
      funkcija_konverzija(1+(strlen(gt_number)+1)/2, br,2);
      strcat(pomocni1, br);
      strcat(pomocni1,"91");
      
    char gt_num2[20];
    strcpy(gt_num2, gt_number);
    obradi_broj(gt_num2);
    strcat(pomocni1, gt_num2);
     
   strcat (pomocni1, "04");
     /* duzina pdu-a*/
     if((strlen(pdu)/2)<128)
     {
     funkcija_konverzija(strlen(pdu)/2, br,2);
     strcat (pomocni1, br);
     }
     else if ((strlen(pdu)/2)<256)
     {
     strcat (pomocni1, "81");  
     funkcija_konverzija(strlen(pdu)/2, br,2);
     strcat (pomocni1, br);
     }
     else
     {
     strcat (pomocni1, "82");  
     funkcija_konverzija((strlen(pdu)/2)/0xff , br,2);
     strcat (pomocni1, br);
     funkcija_konverzija((strlen(pdu)/2)%0xff , br,2);
     strcat (pomocni1, br); 
     }
     strcat(pomocni1, pdu);

      
 
     int duzina_podporuke=2+strlen(pomocni1)/2;
     

     if(duzina_podporuke<128)
     {
     funkcija_konverzija(duzina_podporuke, br,2);
     strcpy (pomocni2, br);
     }
     else if (duzina_podporuke<256)
     {
     strcpy (pomocni2, "81");  
     funkcija_konverzija(duzina_podporuke, br,2);
     strcat (pomocni2, br);
     }
     else
     {
     strcpy (pomocni2, "82");  
     funkcija_konverzija(duzina_podporuke/0xff , br,2);
     strcat (pomocni2, br);
     funkcija_konverzija(duzina_podporuke%0xff , br,2);
     strcat (pomocni2, br); 
     } 
     
     strcat(pomocni2, pomocni1);
    
       

     strcpy (pomocni1,"0201");
     //incvoke ID
      strcat (pomocni1,"02");
     strcat(pomocni1, "02012e30");
     // pretposlednje 2e je operation code
      strcat(pomocni1, pomocni2);
     duzina_podporuke=2+strlen(pomocni1)/2;

 
     /* GSM Mobile Application */
       strcpy (cdata, "a1");
         
     if(duzina_podporuke<128)
     {
     funkcija_konverzija(duzina_podporuke, br,2);
     strcat (cdata, br);
     }
     else if (duzina_podporuke<256)
     {
     strcat (cdata, "81");  
     funkcija_konverzija(duzina_podporuke, br,2);
     strcat (cdata, br);
     }
     else
     {
     strcat (cdata, "82");  
     funkcija_konverzija(duzina_podporuke/0xff , br,2);
     strcat (cdata, br);
     funkcija_konverzija(duzina_podporuke%0xff , br,2);
     strcat (cdata, br); 
     } 
     strcat(cdata, pomocni1);
     strcat(cdata, "050000");
     
      unsigned char udata[(strlen(cdata))/2];
    int i;
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
     for(i=0; i<(strlen(cdata))/2; i++)
       printf("%x ", udata[i]);
     printf("\n");
    
   }
   
   
void tcap_forwardSM(unsigned char* cdata,char* gt_number, char* imsi, unsigned char* pdu)
{
  unsigned char temp_buffer[1000];
  char br[10];

  gsm_forwardSM(temp_buffer, gt_number, imsi, pdu);
  
  strcpy(cdata, "62");
  // nije const
     funkcija_konverzija(8+strlen(temp_buffer)/2, br,2);
      strcat(cdata, br);
    
    /* Source transaction ID*/
    strcat(cdata, "4804");
     funkcija_konverzija(stid, br,8);
      strcat(cdata, br);
      stid++;
    strcat(cdata, "6c");
       funkcija_konverzija(strlen(temp_buffer)/2, br,2);
      strcat(cdata, br);
      strcat(cdata, temp_buffer);
   
    int i;
      printf("---------------------    tcap layer      %d -----------------\n", strlen(cdata));
    unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
     for(i=0; i<(strlen(cdata))/2; i++)
       printf("%x ", udata[i]);
     printf("\n");
      
     
}

void tcap2_forwardSM(unsigned char* cdata,char* gt_number, char* imsi, unsigned char* pdu)
{
  unsigned char temp_buffer[1000];
  char br[10];
  int len1, len2;
  gsm2_forwardSM(temp_buffer, gt_number, imsi, pdu);
  
  len2=strlen(temp_buffer)/2;
  len1=7+strlen(temp_buffer)/2;
  if(len2<128)
    len1++;
  else if (len2<256)
    len1+=2;
  else
    len1+=3;
  
  strcpy(cdata, "62");

  
 if(len1<128)
 {
     funkcija_konverzija(len1, br,2);
     strcat(cdata, br);
 }
 else if (len1<256)
 {
        strcat(cdata, "81");
      funkcija_konverzija(len1, br,2);
     strcat(cdata, br);
   
 }
 else
 {
   strcat(cdata, "82");
      funkcija_konverzija(len1/0xff, br,2);
     strcat(cdata, br);
       funkcija_konverzija(len1%0xff, br,2);
     strcat(cdata, br);
 }
  
  
  
  
  
  
  
  
  
    /* Source transaction ID*/
    strcat(cdata, "4804");
     funkcija_konverzija(stid, br,8);
      strcat(cdata, br);
      stid++;
    strcat(cdata, "6c");
    if(len2<128)
    {
   funkcija_konverzija(len2, br,2);
    strcat(cdata, br);
    }
   else if (len2<256)
   {
         strcat(cdata, "81");
         funkcija_konverzija(len2, br,2);
	 strcat(cdata, br);

   }
   
   else
   {
         strcat(cdata, "82");
         funkcija_konverzija(len2/0xff, br,2);
	 strcat(cdata, br);
	 funkcija_konverzija(len2%0xff, br,2);
	 strcat(cdata, br);     
   }
     
     strcat(cdata, temp_buffer);
       
}
void sccp_forwardSM(unsigned char* cdata, char* gt_number, char* imsi, unsigned char* pdu, char* msc)
{
 unsigned char temp_buffer[1000];
 char br[10];

 tcap2_forwardSM(temp_buffer, gt_number, imsi, pdu);
   /*Message Type*/
    funkcija_konverzija(9, br,2);
    strcpy(cdata, br);
     /*Message class*/
    funkcija_konverzija(129, br,2);
    strcat(cdata, br);
     /*Parameter to First Mandatoty var*/
    funkcija_konverzija(3, br,2);
    strcat(cdata, br);
       /*Parameter to Second Mandatoty var*/
// nastaje kada se na gornju 3 doda 11 bajtova sto j duzina zaglavlja za podatke o broju za koji se HLR radi
funkcija_konverzija(3+5+(strlen(msc)+1)/2, br,2);
strcat(cdata, br);
/*Parameter to third Mandatoty var*/
// i ova vrednost nastaje dodavanjem 11 ali to je uvek Malta pa je const
funkcija_konverzija(3+10+(strlen(msc)+1)/2+(strlen(gt_number)+1)/2, br,2);
strcat(cdata, br);

funkcija_konverzija(5+(strlen(msc)+1)/2, br,2);
strcat(cdata, br);

  /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* HLR 6*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
     if(strlen(msc)%2)
     {
    funkcija_konverzija(17, br,2);
     }
    else
    {
      funkcija_konverzija(18, br,2);
    }
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
     char msc2[15];
      strcpy(msc2,msc);
      obradi_broj_bez_f(msc2);
      strcat(cdata,msc2);      
      /*duzina zaglavlja za Calling */

funkcija_konverzija(5+(strlen(gt_number)+1)/2, br,2);
strcat(cdata, br);

      /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* HLR 6*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
     if(strlen(gt_number)%2)
    funkcija_konverzija(17, br,2);
    else
      funkcija_konverzija(18, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
     char gt2[15];
      strcpy(gt2,gt_number);
      obradi_broj_bez_f(gt2);
      strcat(cdata,gt2);       

    funkcija_konverzija(strlen(temp_buffer)/2, br,2);
      strcat(cdata, br);
      strcat(cdata, temp_buffer);
 
}

void m3ua_forwardSM(char* gt_number, char* imsi, unsigned char* pdu, char* msc, int RC, int OPC, int DPC, int NI, int server)
{
  unsigned char cdata[1000];
  char br[10];
  unsigned char temp_buffer[1000];
       int i;

  sccp_forwardSM(temp_buffer,gt_number, imsi,  pdu, msc);
  
  cvor *pomocni = pronadji_cvor_po_vrednosti(MessageTypeValue, "DATA");  
    /*Version*/
    funkcija_konverzija(1, br, 2);
    strcpy(cdata, br);
    /*Reserved*/
    funkcija_konverzija(0, br, 2);
    strcat(cdata, br);
    /*Message class*/
    funkcija_konverzija(atoi(pomocni->prva), br, 2);
    strcat(cdata, br);
    /*Message Type*/
    funkcija_konverzija(atoi(pomocni->druga), br, 2);
    strcat(cdata, br);
    /*Message length*/
    int duzina_poruke=32+strlen(temp_buffer)/2;
    
    int br_padding_bajtova=0;
    
    	switch(duzina_poruke % 4)
	{
	case 1:	/* append 3 bytes of padding */
	      br_padding_bajtova++;	
	case 2:	/* append 2 bytes of padding */
	      br_padding_bajtova++;	
	case 3: /* append 1 byte of padding */
	      br_padding_bajtova++;	
	case 0: /* no padding needed */
	break;
		}
    
    int fin_len=br_padding_bajtova+duzina_poruke;	
 // nije ovo fiksna duzina, zavisi od PDU-a
    funkcija_konverzija(fin_len, br,8);
    strcat(cdata, br);

     /*Parametar tag*/
    funkcija_konverzija(6, br, 4);
    strcat(cdata, br);
    /*Parametar length*/
    funkcija_konverzija(8, br,4);
        strcat(cdata, br);
	/* OPC */
    funkcija_konverzija(RC, br,8);
        strcat(cdata, br);
    
    
     /*Parametar tag*/
    funkcija_konverzija(528, br, 4);
    strcat(cdata, br);
 //nije fiksne duzine, zavisi od PDU-a   
    /*Parametar length*/
    funkcija_konverzija(16+strlen(temp_buffer)/2, br,4);
        strcat(cdata, br);
	/* OPC */
    funkcija_konverzija(OPC, br,8);
        strcat(cdata, br);
	/* DPC */
    funkcija_konverzija(DPC, br, 8);
        strcat(cdata, br);
	/* SI */
    funkcija_konverzija(3, br,2);
        strcat(cdata, br);
	/* NI */
     funkcija_konverzija(NI, br,2);
        strcat(cdata, br);
	/*MP */
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/* SLS*/
     funkcija_konverzija(SLS, br,2);
     SLS++;
     if (SLS==15)
       SLS=2;
        strcat(cdata, br);
	strcat(cdata, temp_buffer);
	
      funkcija_konverzija(0, br, 2*br_padding_bajtova);
	strcat(cdata, br);
	
      printf("---------------------    m3ua layer      %d -----------------\n", strlen(cdata));
	
	
       unsigned char udata[(strlen(cdata))/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
     for(i=0; i<(strlen(cdata))/2; i++)
       printf("%x ", udata[i]);
     printf("\n");
      
      int count = sizeof (udata);
      if (send(server, udata, count, 0) != count)
	error_fatal ("%s send() error\n", "client"); 
	
}


void posalji_sm2(int server,char* imsi,char* msc,char* pdu, char* br_malte)
   {
    	  
    char cdata[400];
    char br[30];
    int OPC=5050;
    int DPC=8080;
    
    cvor *pomocni = pronadji_cvor_po_vrednosti(MessageTypeValue, "DATA");  
    /*Version*/
    funkcija_konverzija(1, br, 2);
    strcpy(cdata, br);
    /*Reserved*/
    funkcija_konverzija(0, br, 2);
    strcat(cdata, br);
    /*Message class*/
    funkcija_konverzija(atoi(pomocni->prva), br, 2);
    strcat(cdata, br);
    /*Message Type*/
    funkcija_konverzija(atoi(pomocni->druga), br, 2);
    strcat(cdata, br);
    /*Message length*/
    
 // nije ovo fiksna duzina, zavisi od PDU-a
 int messLen=100+strlen(pdu)/2;
    funkcija_konverzija(messLen, br,8);
    strcat(cdata, br);

     /*Parametar tag*/
    funkcija_konverzija(528, br, 4);
    strcat(cdata, br);
 //nije fiksne duzine, zavisi od PDU-a   
    /*Parametar length*/
    funkcija_konverzija(89+strlen(pdu)/2, br,4);
        strcat(cdata, br);
	/* OPC */
    funkcija_konverzija(OPC, br,8);
        strcat(cdata, br);
	/* DPC */
    funkcija_konverzija(DPC, br, 8);
        strcat(cdata, br);
	/* SI */
    funkcija_konverzija(3, br,2);
        strcat(cdata, br);
	/* NI */
     funkcija_konverzija(2, br,2);
        strcat(cdata, br);
	/*MP */
     funkcija_konverzija(0, br,2);
        strcat(cdata, br);
	/* SLS*/
     funkcija_konverzija(SLS, br,2);
     SLS++;
     if (SLS==15)
       SLS=2;
        strcat(cdata, br);
	

    /*Message Type*/
    funkcija_konverzija(9, br,2);
    strcat(cdata, br);
     /*Message class*/
    funkcija_konverzija(128, br,2);
    strcat(cdata, br);
     /*Parameter to First Mandatoty var*/
    funkcija_konverzija(3, br,2);
    strcat(cdata, br);
      /*Parameter to Second Mandatoty var*/
    funkcija_konverzija(14, br,2);
    strcat(cdata, br);
      /*Parameter to third Mandatoty var*/
    funkcija_konverzija(25, br,2);
    strcat(cdata, br);
    
    /* je ne sais pas */
    funkcija_konverzija(11, br,2);
    strcat(cdata, br);
    
    /*Address indicator*/
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* MSC 8*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
     /*Numbering plan */
    funkcija_konverzija(17, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
       
     obradi_broj(msc);
  // MSC mozda baguje!!!!!!!!!!!!!!
      strcat(cdata, msc);
           obradi_broj(msc);


          /* je ne sais pas */
      funkcija_konverzija(11, br,2);
      strcat(cdata, br);
    funkcija_konverzija(18, br,2);
      strcat(cdata, br);
    /*SubSystemNumber*/
    /* MSC 8*/
    funkcija_konverzija(8, br,2);
      strcat(cdata, br);
     /* Translation type */
    funkcija_konverzija(0, br,2);
      strcat(cdata, br);
      

	 
     /*Numbering plan */
    funkcija_konverzija(17, br,2);
      strcat(cdata, br);
      /*nature of AI*/
      /*international number  04 */
    funkcija_konverzija(4, br,2);
      strcat(cdata, br);
       /* CGT Digital */
   // funkcija_konverzija(381641499068, br,12);
      obradi_broj(br_malte);
  strcat(cdata, "836100169400");
      //strcat(cdata, br_malte);
    
    funkcija_konverzija(43+strlen(pdu)/2, br,2);
      strcat(cdata, br);
       
   //TCAP
   
   strcat(cdata, "62");
  // nije const
     funkcija_konverzija(41+strlen(pdu)/2, br,2);
      strcat(cdata, br);
    
    /* Source transaction ID*/
    strcat(cdata, "4804");
     funkcija_konverzija(stid, br,8);
     // strcat(cdata, br);
      strcat(cdata, "02000000");
      stid++;
   /* //destination transaction id
     strcat(cdata, "4904041e35f9");
     
      strcat(cdata, "6c");
     //ovo nije const ili bar ne bi trebalo :) 
     // trebalo bi da stoji ovaj red
      //funkcija_konverzija(33+strlen(pdu)/2, br,2);
       strcat(cdata, "39");
     */
     //strcat(cdata, "4a");
     
    strcat(cdata, "6c");

       funkcija_konverzija(33+strlen(pdu)/2, br,2);
      strcat(cdata, br);
     
     /* GSM Mobile Application */
     
     strcat (cdata, "a1");
     
     
     funkcija_konverzija(31+strlen(pdu)/2, br,2);
      strcat(cdata, br);
     
     strcat (cdata,"0201");
     //incvoke ID
      strcat (cdata,"02");
     strcat(cdata, "02012e30");
     // pretposlednje 2e je operation code
     
     funkcija_konverzija(23+strlen(pdu)/2, br,2);
      strcat(cdata, br);
     
     strcat(cdata, "8008");
     // 08 je duzina imsi ja
     
     /* TBCD digits */
     //obradi_broj(imsi);
     strcat(cdata, imsi);
     
     // 07 je od duzine tog dela gde se br operatora stavlja, 91 je deo Maltinnog broja
      strcat (cdata, "840791");
      
            //obradi_broj(br_malte);

       //strcat(cdata, br_malte);
     strcat (cdata, "5396490125f4");
     strcat (cdata, "04");
     /* duzina pdu-a*/
     funkcija_konverzija(strlen(pdu)/2, br,2);
         strcat (cdata, br);
      strcat(cdata, pdu);
      
      funkcija_konverzija(5, br,2);
      strcat(cdata, br);
      
        
    funkcija_konverzija(0, br,4);
      strcat(cdata, br);
            
      unsigned char udata[(strlen(cdata)-1)/2];
    
    const char *p;
    unsigned char *up;

    for(p=cdata,up=udata;*p;p+=2,++up)
    {
        *up = hctoi(p[0])*16 + hctoi(p[1]);
    }
      
      int count2 = sizeof (udata);
      if (send(server, udata, count2, 0) != count2)
	error_fatal ("%s send() error\n", "client"); 
	  
	
     
   }


cvorH* obrisi_cvor(cvorH* koren, char* id){
	if(koren==NULL)
	{
	  printf(" - 1 - \n");
		return NULL;
	}
		
		
		
	if(strcmp(koren->id,id)>0)
	{
		koren->levo=obrisi_cvor(koren->levo, id);
		return koren;	
	}
	if (strcmp(koren->id,id)<0)
	{
		koren->desno=obrisi_cvor(koren->desno, id);
		return koren;	
	}
		
	if(strcmp(koren->id,id)==0){
	
		/* brisemo list */
		if(koren->levo==NULL && koren->desno==NULL){
			free(koren);	
		        printf(" - 2 - \n");

			return NULL;
		}

		/* brisemo cvor koji ima samo desno poddrvo */			
		if(koren->levo==NULL && koren->desno!=NULL){
			cvorH* pomocni_cvor=koren->desno;
			  printf(" - 3 - \n");

			free(koren);
			return pomocni_cvor;
		}
		
		/* brisemo cvor koji ima samo levo poddrvo */
		if(koren->levo!=NULL && koren->desno==NULL){
			cvorH* pomocni_cvor=koren->levo;
			free(koren);
			  printf(" - 4 - \n");

			return pomocni_cvor;
		}
		
		/* inace brisemo cvor koji ima i levo i desno poddrvo */
		/* 
			ideja je da nadjemo najmanji element desnog poddrveta i da njegovu vrednost zamenimo 
			sa vrednoscu cvora koji brisemo i da problem brisanje svedemo na desno poddrvo - slucaj koji 
			cemo dobiti sigurno odgovara jednom od 3 gornja slucaja 
		*/
		
		cvorH* min=min_cvor(koren->desno);
		if(min!=NULL)
		{
		koren->poslato=min->poslato;
		strcpy(koren->id, min->id);
		strcpy(koren->imsi, min->imsi);
		strcpy(koren->msc, min->msc);
		strcpy(koren->dest, min->dest);
		strcpy(koren->pdu, min->pdu);
		koren->desno=obrisi_cvor(koren->desno, id);
		  printf(" - 5 - \n");
		return koren;
		}
		
	}
	
	
	
}
static void* do_work (void *arg)
{

    int server;			/* Deskriptor konekcije sa serverom. */
    char buffer[BUFFER_SIZE];	/* Bafer za smestanje poruke od servera. */
    int count;			/* Broj bajtova primljenih u jednom paketu. */
    struct sockaddr_in addressclinet;
    int PORT;
    
    invokeID=2;
    SLS=2;
    struct sockaddr_in address = *((struct sockaddr_in *) arg);
    stid=1000;
    if ((errno = pthread_detach (pthread_self ())) != 0)
    error_fatal ("%s pthread_detach() error: %s\n", program, strerror (errno));
    
    
  if ((server =  socket (PF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
    error_fatal ("%s socket() error: %s\n", program, strerror (errno));
 
 
bzero (&addressclinet, sizeof (struct sockaddr_in));
addressclinet.sin_family = AF_INET;

addressclinet.sin_port = address.sin_port;
if (inet_pton (AF_INET, "10.0.45.1", &addressclinet.sin_addr) < 0)
error_fatal ("%s inet_pton() error: %s\n", program, strerror (errno));  


if (bind (server, (struct sockaddr *) &addressclinet, sizeof (struct sockaddr_in))< 0)
                   error_fatal ("%s bind() error: %s\n", program, strerror (errno));	

  /* Vrsi se povezivanje sa serverom. */
  if (connect (server, (struct sockaddr *) &address, sizeof (struct sockaddr_in)) < 0)
    error_fatal ("%s connect() error: %s\n", program, strerror (errno));
    
  
  printf("Konekcija sa serverom uspostavljena!\n");

    PORT=ntohs(addressclinet.sin_port);
    printf("Gradi se stek\n");
    izgradi_stek(server, addressclinet.sin_port);
    
    fd_set read_set;
    
    FD_ZERO (&read_set);
   
  for (;;)
  {
  FD_SET (server, &read_set);
           struct timeval tv;
           tv.tv_usec = 2;
	   tv.tv_sec = 0;
	   
      
      /* Server se blokira dok se ne pojavi aktivnost na nekom
       * od deskriptora iz radnog skupa. */
      select (MAX_DESCRIPTOR, &read_set, NULL, NULL, &tv);
	  /* Ako je tekuci socket aktivan...  */
	  if (FD_ISSET (server, &read_set))
	    {
	      /* ...pokusava se sa ucitavanjem poruke od 
	       * klijenta,...  */
	      count =   recv (server, buffer, BUFFER_SIZE, 0);	      
	      if(count>0)
	      {
		 printf("pristigao paket od servera\n");
		 pthread_mutex_lock (&mutex);
		 dodaj_u_red(&pocetak_reda, &kraj_reda, buffer, count);
		 pthread_mutex_unlock (&mutex);
	         FD_CLR (server, &read_set);
	    }
	    
	    if(count==0)
	    {
	       FD_CLR (server, &read_set);
	       close (server); 
               return;	       
	    }
	    
	}
    else{
      
      
      
    //nadji koji nema imsi i msc i  poslato 0 ili 1
    pthread_mutex_lock (&mutex);
    cvorH* pom=nadji_cvor_za_hlr(pocetakHM);
    pthread_mutex_unlock (&mutex);
    
    if (pom!=NULL)
    {
      printf("nasli cvor za hlr\n");
      pom->br_paketa=stid;
      time_t rawtime;
      time( &rawtime );
      pom->min=localtime( &rawtime )->tm_min;
      m3ua_pdu(pom->dest, "38160061490", server, 100, 5050, 1234, 0);
      sleep(3);
    }
    
    cvorH* pom2;
    pthread_mutex_lock (&mutex);
    pom2=nadji_cvor_za_forwardSM(pocetakHM);
    pthread_mutex_unlock (&mutex);
    
    while (pom2!=NULL) 
    {
    printf("-----------------------------------------------nasli nesto za forward  %s  %s \n", pom2->imsi, pom2->msc); 
    pom2->br_paketa=stid;
      time_t rawtime;
      time( &rawtime );
      pom2->min=localtime(&rawtime )->tm_min;
    m3ua_forwardSM("38160061490", pom2->imsi, pom2->pdu, pom2->msc, 101, 5050, 1234, 0, server); 
    sleep(3);
    pthread_mutex_lock (&mutex);
    pom2=nadji_cvor_za_forwardSM(pocetakHM);
    pthread_mutex_unlock (&mutex);
    }    
    
    
  }
}
  
  /* Zatvara se konekcija sa serverom. */
  if (close (server) < 0)
    error_fatal ("%s close() error: %s\n", program, strerror (errno)); 
    ind1=1;
}
 
static void* drljaj_po_bazi (void *arg)
{
   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;
   char query[1500];
   char *server = "ss7.cs-networks.net";
   char *user = "ss7user";
   char *password = "trlababalan"; /* set me first */
   char *database = "mobiclick";
   char id[10][50];
   int br, i;
       if ((errno = pthread_detach (pthread_self ())) != 0)
    error_fatal ("%s pthread_detach() error: %s\n", program, strerror (errno));
    
   conn = mysql_init(NULL);
   /* Connect to database */
   if (!mysql_real_connect(conn, server,user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }
   /* send SQL query */

while(1)
{
    if (br_elemenata_uHM (pocetakHM)<6)
    {
    if (mysql_query(conn, "select id, IMSI, MSCAddress, DestinationAddress, SMSTPDU from PendingSMS where Processed=0 and RouteToken='RTK_Emulator01' order by 2 asc limit 5"))
      fprintf(stderr, "prvi:  %s\n", mysql_error(conn));
    
   res = mysql_use_result(conn);
   br=0;
   //printf("SQL result:\n");
   while ((row = mysql_fetch_row(res)) != NULL)
   {
      pthread_mutex_lock (&mutex);
      pocetakHM=dodajIDuHash(pocetakHM,row[0], row[1], row[2], row[3], row[4]);   
      ispisiHM(pocetakHM);
      pthread_mutex_unlock (&mutex);
      strcpy(id[br], row[0]);
      br++;
   }
   
   //update-ujemo tabelu da znamo da su te poruke uzete vec
if(br>0)
{
strcpy(query, "update PendingSMS set Processed=1 where id in (");
for(i=0; i<br; i++)
{
  strcat(query, "'");
  strcat(query, id[i]);
  strcat(query, "'");
if((i+1)==br)
  break;
else
strcat(query, ",");
}

strcat(query, ")");
printf("%s\n", query);


/* radimo update*/
 if (mysql_query(conn,query)) {
      fprintf(stderr, "update PendigSMS %s\n", mysql_error(conn));
   }
} 
    } 
   
    pthread_mutex_lock (&mutex);
   cvorH* pom=nadji_poslato_za_insert(pocetakHM);
   pthread_mutex_unlock (&mutex);
   
   while(pom!=NULL)
   {
     
   if(pom->poslato==11)
  sprintf(query, "insert into SentSMS (ID, TimeDate, OriginatingAddress, DestinationAddress, MSCAddress, IMSI, SMSTPDU, DeliveryStatus) select ID, TimeDate, OriginatingAddress, DestinationAddress, '%s', '%s', SMSTPDU, %d from PendingSMS where id='%s'", pom->msc,  pom->imsi, 9, pom->id);
  else
  sprintf(query, "insert into SentSMS (ID, TimeDate, OriginatingAddress, DestinationAddress, MSCAddress, IMSI, SMSTPDU, DeliveryStatus) select ID, TimeDate, OriginatingAddress, DestinationAddress, '%s', '%s', SMSTPDU, %d from PendingSMS where id='%s'", pom->msc,  pom->imsi, 0, pom->id);
  printf("%s\n", query);
   
if (mysql_query(conn,query)) 
      fprintf(stderr, "insert %s\n", mysql_error(conn));

  sprintf(query, "delete from PendingSMS where id='%s'", pom->id);
   printf("%s\n", query); 
if (mysql_query(conn,query)) 
      fprintf(stderr, "delete %s\n", mysql_error(conn));


   pthread_mutex_lock (&mutex);
   pocetakHM=obrisi_cvor(pocetakHM, pom->id);
   pthread_mutex_unlock (&mutex);
printf("obrisali cvor\n");

   pthread_mutex_lock (&mutex);
   pom=nadji_poslato_za_insert(pocetakHM);
   pthread_mutex_unlock (&mutex);
   }
    
 sleep(1);   
  
}

   /* close connection */
   mysql_free_result(res);
   mysql_close(conn);
   printf("zatvorena konekcija s bazom\n");   
}

int isRRLsriSM(unsigned char* paket, int velicina)
{
 
  int i;
  for(i=0; i<velicina-10; i++)
   if ((paket[i]==162) && (paket[i+2]==2) && (paket[i+3]==1) && (paket[i+5]==48)  && (paket[i+7]==2) && (paket[i+8]==1)&& (paket[i+9]==45))
         return 1;   
    return 0;
  
}

int isRRL(unsigned char* paket, int velicina)
{
   int i;
  for(i=0; i<velicina-4; i++)
   if ((paket[i]==162) && (paket[i+1]==3) && (paket[i+2]==2) && (paket[i+3]==1))
         return 1;
   
    return 0;
}

int abortDtid(unsigned char* paket, int velicina)
{
  
  int i;
  for(i=0; i<velicina-4; i++)
   if ((paket[i]==107) && (paket[i+1]==5) && (paket[i+2]==48) && (paket[i+3]==3) && (paket[i+4]==10) && (paket[i+5]==1) && (paket[i+6]==0))
         return 1;
   
    return 0;
}

void uzmi_imsi_msc (unsigned char* buffer, int count)
{
  unsigned char imsi[20], msc[20];
 int i;
 int poc_msc, poc_imsi;
 int duzina_msc, duzina_imsi;
 unsigned char br[10];
 int br_bajtova_za_imsi;
 for (i=count; i>=0; i--)
  if ((buffer[i]==145) && (buffer[i-2]==129))
  {
    poc_msc=i+1;
    duzina_msc=buffer[i-1]-1;
  }
 
  for (i=count; i>=0; i--)
  if ((buffer[i]==2) && (buffer[i+1]==1) && (buffer[i+2]==45) && (buffer[i+3]==48) && (buffer[i+5]==4))
  {
    poc_imsi=i+7;
    br_bajtova_za_imsi=buffer[i+6];
  }
    
    
      
  int parnostmsc=0;
 
    for(i=0; i<duzina_msc; i++)
  {
 if ((i==duzina_msc-1) && (buffer[poc_msc+i]>=240))
 {
   parnostmsc=1;
     funkcija_konverzija(buffer[poc_msc+i]-240, br,2);
 }
 else
     funkcija_konverzija(buffer[poc_msc+i], br,2);
 if (i==0)
   strcpy(msc, br);
 else
   strcat(msc, br);
  }
   obradi_broj_bez_f(msc);
  printf("msc je %s, %d\n", msc, strlen(msc));
 
  if (parnostmsc)
  {
    int pom= strlen(msc);
     msc[ strlen(msc)-1]='\0';
  }
   int j=0;
   
   for(i=poc_imsi; i<poc_imsi+br_bajtova_za_imsi; i++)
  {
     funkcija_konverzija(buffer[i], br,2);
     imsi[j]=br[0];
     imsi[j+1]=br[1];
     j+=2;
 //printf("%x %s\n", buffer[i], br);
 //printf("imsi:  %s\n", imsi);
  }

  obradi_broj(imsi);
  imsi[br_bajtova_za_imsi*2]='\0';
  printf("imsi je %s, a msc je %s i  br imsi cifara je %d\n", imsi, msc, br_bajtova_za_imsi);
 
 char idpaketa[10];
 
 uzmiIDpaketa(buffer, count, idpaketa,2);
 int pot_id=0;
 int pr=1;
 for(i=7; i>3;i--)
 {
  if((idpaketa[i]>='0') && (idpaketa[i]<='9'))
   pot_id+=pr*(idpaketa[i]-'0');
  else
   pot_id+=pr*(idpaketa[i]-'a'+10);
   pr*=16; 
  
 }
  
   pthread_mutex_lock (&mutex);
   update_cvor_sa_imsijem(pocetakHM, pot_id, imsi, msc);
   ispisiHM(pocetakHM);
   pthread_mutex_unlock (&mutex);

}

static void* sredi_red_i_HM(void *arg)
{
  
    if ((errno = pthread_detach (pthread_self ())) != 0)
    error_fatal ("%s pthread_detach() error: %s\n", program, strerror (errno));

  int vel;
  char paket[500];
  while(1)
  {
  if(pocetak_reda==NULL)
  {
    sleep(2);
  }
  else
  {
    pthread_mutex_lock (&mutex);
    vel= uzmi_iz_reda(&pocetak_reda, &kraj_reda, paket);
    pthread_mutex_unlock (&mutex);
   
    if(vel>0)
    {
    char idpaketa[10];
    uzmiIDpaketa(paket, vel, idpaketa,2);
     int pot_id=0;
    int pr=1;
    int i;
    for(i=7; i>3;i--)
  {
  if((idpaketa[i]>='0') && (idpaketa[i]<='9'))
   pot_id+=pr*(idpaketa[i]-'0');
  else
   pot_id+=pr*(idpaketa[i]-'a'+10);
   pr*=16; 
  
 }
   printf("nit 3: id pristiglog paketa je %d\n", pot_id);  
      
      
   
    if(isRRLsriSM(paket, vel)==1)
    {
    printf("primili smo HLR odgovor\n");
    uzmi_imsi_msc(paket, vel);
    }
    
    else if(isRRL(paket, vel)==1)
    {
    printf("primili return result\n");
    
       pthread_mutex_lock (&mutex);
    cvorH* pom=nadji_cvor_br_paketa(pocetakHM,pot_id);
    if(pom!=NULL)
      pom->poslato=3;
    pthread_mutex_unlock (&mutex);
    }
    else
    {
      printf("primili neki result\n");
     pthread_mutex_lock (&mutex);
    cvorH* pom=nadji_cvor_br_paketa(pocetakHM,pot_id);
        if(pom!=NULL)
      pom->poslato=9;
    pthread_mutex_unlock (&mutex);
     printf("dosao je paket nepoznate prirode :)\n"); 
    }
  }
  } 
  }
}


int main (int argc, char **argv)
{
  
     pthread_t tid1, tid2, tid3;		/* Identifikator niti. */
     char buffer[200];
     struct sockaddr_in address;
     struct sockaddr_in address2;
   
     program=argv[0];
     ind1=0;
     ind2=0;
     brzanit=0;
     
     
         char prva[10];
	 char druga[50];
	 char treca[500];
	 char cetvrta[500];


	FILE* ulaz=fopen("MessageClassValues.txt","r");
	if(ulaz==NULL){
		fprintf(stderr,"Problem sa otvaranjem datoteke %s!\n", "MessageClassValues.txt");
		exit(EXIT_FAILURE);
	}
	
	while(fgets(buffer, 100, ulaz)!=0){
	strcpy(prva, "");
	strcpy(druga,"");
	strcpy(treca,"");
	strcpy(cetvrta, "");

		sscanf(buffer, "%s %s %s %s", prva, druga, treca, cetvrta);
		messageClassValues=dodaj_cvor(messageClassValues, prva, druga, treca, cetvrta);
		
	}
	fclose(ulaz);

	ulaz=fopen("MessageTypeValue.txt","r");
	if(ulaz==NULL){
		fprintf(stderr,"Problem sa otvaranjem datoteke %s!\n", "MessageTypeValue.txt");
		exit(EXIT_FAILURE);
	}
	while(fgets(buffer, 100, ulaz)!=0){
	strcpy(prva, "");
	strcpy(druga,"");
	strcpy(treca,"");
	strcpy(cetvrta, "");
		sscanf(buffer, "%s %s %s %s", prva, druga, treca, cetvrta);
		MessageTypeValue=dodaj_cvor(MessageTypeValue, prva, druga, treca, cetvrta);
		
	}
	fclose(ulaz);

	ulaz=fopen("ParametarOrder.txt","r");
	if(ulaz==NULL){
	strcpy(prva, "");
	strcpy(druga,"");
	strcpy(treca,"");
	strcpy(cetvrta, "");
		fprintf(stderr,"Problem sa otvaranjem datoteke %s!\n", "ParametarOrder.txt");
		exit(EXIT_FAILURE);
	}
	while(fgets(buffer, 100, ulaz)!=0){
		sscanf(buffer, "%s %s %s %s", prva, druga, treca, cetvrta);
		ParametarOrder=dodaj_cvor(ParametarOrder, prva, druga, treca, cetvrta);
	}
	fclose(ulaz);
		
	ulaz=fopen("ParametarValue.txt","r");
	if(ulaz==NULL){
		fprintf(stderr,"Problem sa otvaranjem datoteke %s!\n", "ParametarValue.txt");
		exit(EXIT_FAILURE);
	}
	while(fgets(buffer, 100, ulaz)!=0){
	strcpy(prva, "");
	strcpy(druga,"");
	strcpy(treca,"");
	strcpy(cetvrta, "");
		sscanf(buffer, "%s %s %s %s", prva, druga, treca, cetvrta);
		ParametarValue=dodaj_cvor(ParametarValue, prva, druga, treca, cetvrta);
	}
	fclose(ulaz);

	
    
     bzero (&address2, sizeof (struct sockaddr_in));
     address2.sin_family = AF_INET;
     address2.sin_port = htons (2905);
     inet_pton (AF_INET, "185.65.107.249", &address2.sin_addr);
	 
      pthread_create (&tid2, NULL, &do_work, &address2);
      pthread_create (&tid1, NULL, &drljaj_po_bazi, NULL);
      pthread_create (&tid3, NULL, &sredi_red_i_HM, NULL);

      
      
          pthread_join(tid1, NULL); 
	  pthread_join(tid2, NULL); 
          pthread_join(tid3, NULL);  

     
   /* Program se zavrsava. */
  exit (EXIT_SUCCESS);
}


