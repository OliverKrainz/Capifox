// CAPI-Funktionen.
//
#include <windows.h>
#include "capicons.h"


static HANDLE gh_capidll = 0;         // Handle der CAPI.DLL
static HANDLE gh_app_buffer = 0;      // Handle Empfangsbuffer
static LPSTR  gp_app_buffer = NULL;   // Pointer Empfangsbuffer
static WORD   g_app_id = 0;           // ID der App im Capi
static HANDLE gh_capi_msg = 0;        // Handle Messagestruktur
static LPSTR  gp_capi_msg = NULL;     // Pointer Messagestruktur
static WORD   g_req_msg_nummer = 1;   // Laufende Messagenummer für Requests
static BYTE   g_isdn_controller = 0;  // ISDN-Controller


int (CALLBACK *my_API_INSTALLED) (void);
int (CALLBACK *my_API_GET_VERSION) (LPSTR);
int (CALLBACK *my_API_MANUFACTURER) (int);
int (CALLBACK *my_API_GET_SERIAL_NUMBER) (LPSTR);
int (CALLBACK *my_API_GET_MANUFACTURER) (LPSTR);
int (CALLBACK *my_API_GET_ADDRESSMODE) (void);
int (CALLBACK *my_API_SET_SIGNAL) (int, void (far *) (void));
int (CALLBACK *my_API_REGISTER) (LPSTR, short, short, short, short);
int (CALLBACK *my_API_PUT_MESSAGE) (int, LPSTR);
int (CALLBACK *my_API_GET_MESSAGE) (int, char far * far *);
int (CALLBACK *my_API_RELEASE) (int);


//------------------
// CAPI.DLL laden.
//
int load_capidll(void)
{
  gh_capidll = LoadLibrary("capi.dll");

  return gh_capidll;
}
// END OF load_capidll
//--------------------

//-------------------
// CAPI.DLL wieder freigeben.
//
void release_capidll(void)
{
  if (gh_capidll > 32)
    FreeLibrary(gh_capidll);
}
// END OF release_capidll
//---------------------

//------------------
// Alle von den Capi-Funktionen
// belegten Speicherblöcke
// wieder freigeben.
//
void free_capi_memory(void)
{
  if (gp_app_buffer != NULL)
  {
    GlobalUnlock(gh_app_buffer);
    GlobalFree(gh_app_buffer);
  }

  if (gp_capi_msg != NULL)
  {
    GlobalUnlock(gh_capi_msg);
    GlobalFree(gh_capi_msg);
  }

}
// END OF free_capi_memory
//----------------------

//----------------
// Capi-ID wieder freigeben.
//
void free_capi_id(void)
{
  my_API_RELEASE(g_app_id);
}
// END OF free_capi_id
//------------------------


//---------------------
// Funktionen verbinden.
//
int verbinde_funktionen(void)
{
  FARPROC l_fhandle;

  l_fhandle = GetProcAddress(gh_capidll, "API_INSTALLED");

  if (l_fhandle != NULL)
  {
    my_API_INSTALLED = (int (pascal far*)(void)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_GET_VERSION");

  if (l_fhandle != NULL)
  {
    my_API_GET_VERSION = (int (pascal far*)(LPSTR)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_MANUFACTURER");

  if (l_fhandle != NULL)
  {
    my_API_MANUFACTURER = (int (pascal far*)(int)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_GET_SERIAL_NUMBER");

  if (l_fhandle != NULL)
  {
    my_API_GET_SERIAL_NUMBER = (int (pascal far*)(LPSTR)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_GET_MANUFACTURER");

  if (l_fhandle != NULL)
  {
    my_API_GET_MANUFACTURER = (int (pascal far*)(LPSTR)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_GET_ADDRESSMODE");

  if (l_fhandle != NULL)
  {
    my_API_GET_ADDRESSMODE = (int (pascal far*)(void)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_SET_SIGNAL");

  if (l_fhandle != NULL)
  {
    my_API_SET_SIGNAL = (int (pascal far*)(int, void (far *) (void) )) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_REGISTER");

  if (l_fhandle != NULL)
  {
    my_API_REGISTER = (int (pascal far*)(LPSTR, short, short, short, short)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_PUT_MESSAGE");

  if (l_fhandle != NULL)
  {
    my_API_PUT_MESSAGE = (int (pascal far*)(int, LPSTR)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_GET_MESSAGE");

  if (l_fhandle != NULL)
  {
    my_API_GET_MESSAGE = (int (pascal far*)(int, char far * far *)) l_fhandle;
  }
  else
    return ERROR;

  l_fhandle = GetProcAddress(gh_capidll, "API_RELEASE");

  if (l_fhandle != NULL)
  {
    my_API_RELEASE = (int (pascal far*)(int)) l_fhandle;
  }
  else
    return ERROR;

  return SUCCESS;
}
// END OF verbinde_funktionen
//--------------------------

//-------------------------
// Capistrukturen initialisieren.
//
int init_capi_strukturen(void)
{

  // Empfangsbuffer der Anwendung initialisieren.
  //
  gh_app_buffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD) c_size_app_buffer);

  if (gh_app_buffer == NULL)
    return ERROR;

  gp_app_buffer = GlobalLock(gh_app_buffer);

  if (gp_app_buffer == NULL)
  {
    GlobalFree(gh_app_buffer);
    return ERROR;
  }

  // Messagestruktur initialisieren.
  //
  gh_capi_msg = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD) c_capi_msg_len);

  if (gh_capi_msg == NULL)
    return ERROR;

  gp_capi_msg = GlobalLock(gh_capi_msg);

  if (gp_capi_msg == NULL)
  {
    GlobalFree(gh_capi_msg);
    return ERROR;
  }

  return SUCCESS;
}
// END OF init_capi_strukturen
//----------------------

//-------------------------
// Anwendung beim CAPI anmelden.
//
int capi_anmeldung(void)
{

  // Capi-Funktion zum Anmelden der Applikation rufen.
  // Parameter:
  // 1) Zeiger auf Empfangsbuffer
  // 2) Anzahl der möglichen Messages vor Überlauf
  // 3) Anzahl der möglichen Schicht-3 Verbindungen
  // 4) Anzahl der maximal an die Anwendung meldbare Datenblöcke
  // 5) Maximale Größe B3-Daten
  //
  g_app_id = my_API_REGISTER(gp_app_buffer, 10, 1, 4, 2048);

  if (!g_app_id)
    return ERROR;

  return SUCCESS;
}
// END OF capi_anmeldung
//------------------------------

//-------------------------------
// CAPI-Anrufbeantwortung aktivieren.
//
int aktiviere_anrufbeantwortung(void)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_listen_request);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command = Listen
  //
  schreibe_BYTE(gp_capi_msg + 4, c_listen_command);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. ISDN-Controller
  //
  schreibe_BYTE(gp_capi_msg + 8, g_isdn_controller); 

  // 7. InfoMaske, welche Infos sollen an die Anwendung
  // weitergegeben werden.
  //
  schreibe_DWORD(gp_capi_msg + 9, 0l);

  // 8. EAZ-Maske = Auf welche Nummern soll die Anwendung reagieren.
  // 0x3ff = reagiert auf GlobalCall und alle EAZs.
  //
  schreibe_WORD(gp_capi_msg + 13, 0x03ff);

  // 9. Maske welche Dienste bedient werden sollen.
  // 0x0080 = Nur Datenübertragung 64 kbits/s.
  //
  schreibe_WORD(gp_capi_msg + 15, 0x0080); 

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return !l_status;
}
// END OF aktiviere_anrufbeantwortung
//----------------------------

//-------------------------------
// CAPI-Anrufbeantwortung deaktivieren.
//
int deaktiviere_anrufbeantwortung(void)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_listen_request);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command = Listen
  //
  schreibe_BYTE(gp_capi_msg + 4, c_listen_command);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. ISDN-Controller
  //
  schreibe_BYTE(gp_capi_msg + 8, g_isdn_controller); 

  // 7. InfoMaske, welche Infos sollen an die Anwendung
  // weitergegeben werden.
  //
  schreibe_DWORD(gp_capi_msg + 9, 0l);

  // 8. EAZ-Maske = Auf welche Nummern soll die Anwendung reagieren.
  // 0x3ff = reagiert auf GlobalCall und alle EAZs.
  //
  schreibe_WORD(gp_capi_msg + 13, 0x00);

  // 9. Maske welche Dienste bedient werden sollen.
  // 0x0080 = Nur Datenübertragung 64 kbits/s.
  //
  schreibe_WORD(gp_capi_msg + 15, 0x00); 

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return !l_status;
}
// END OF deaktiviere_anrufbeantwortung
//----------------------------

//----------------------
// Sende Connect-Request
//
int sende_connect_request(LPSTR par_fon_nr)
{
  int l_status;
  WORD l_laenge_zielnr;
  char l_fon_nr[20];
 
  l_laenge_zielnr = _fstrlen(par_fon_nr);

  _fstrcpy(l_fon_nr, par_fon_nr);

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_connect_request + 1 + l_laenge_zielnr);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command = Connect
  //
  schreibe_BYTE(gp_capi_msg + 4, c_connect_command);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. ISDN-Controller
  //
  schreibe_BYTE(gp_capi_msg + 8, g_isdn_controller); 

  // 7. Gewünschter B-Kanal.
  // 0x83 = beliebiger Kanal.
  //
  schreibe_BYTE(gp_capi_msg + 9, (unsigned char) 0x83);

  // 8. Welcher Informationen sollen angezeigt werden.
  // 01 = Date.
  //
  schreibe_DWORD(gp_capi_msg + 10, 0xc00000001);

  // 9. Outgoing Service.
  // 07 ist der Standardeintrag (???).
  //
  schreibe_BYTE(gp_capi_msg + 14, 0x07);

  // 10. Outgoind Service Zusatzangabe.
  // 00 = ???
  //
  schreibe_BYTE(gp_capi_msg + 15, 0x00);

  // 11. EAZ des Initiators.
  // 0x30 = '0'
  //
  schreibe_BYTE(gp_capi_msg + 16, 0x30);

  // 12. Länge der ZielNr.
  //
  schreibe_BYTE(gp_capi_msg + 17, l_laenge_zielnr + 1);

  // 13. Typ der ZielNr.
  // 0x81 = ???
  //
  schreibe_BYTE(gp_capi_msg + 18, (unsigned char) 0x81);

  // 14. ZielNr. 
  //
  _fstrcpy(gp_capi_msg + 19, l_fon_nr);


  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

  
}
// END OF sende_connect_request
//------------------------

//-----------------
// Protokol Schicht 2 einstellen.
//
int select_b2_protocol(WORD par_plci)
{

  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_select_b2_protocol);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command 
  //
  schreibe_BYTE(gp_capi_msg + 4, c_select_b2_protocol);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci); 

  // 7. Gewünschtes Protokoll
  // 0x01 = X75
  //
  schreibe_BYTE(gp_capi_msg + 10, 0x01);

  // 8. Länge DLPD = Data-Link-Protocol-Description
  //
  schreibe_BYTE(gp_capi_msg + 11, 0x03);

  // 9. Neue Maximale Länge Datenpaket.
  //
  schreibe_WORD(gp_capi_msg + 12, 2048);

  // 10. Link-Adress A.
  //
  schreibe_BYTE(gp_capi_msg + 14, 0x03); 

  // 11. Link-Adress B.
  //
  schreibe_BYTE(gp_capi_msg + 15, 0x01); 

  // 12. Modulo-Mode
  //
  schreibe_BYTE(gp_capi_msg + 16, 0x08); 

  // 13. Window-Size
  // Standard = 7
  //
  schreibe_BYTE(gp_capi_msg + 17, 0x07);


  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF select_b2_protocol
//------------------------

//-----------------
// Protokol Schicht 3 einstellen.
//
int select_b3_protocol(WORD par_plci)
{

  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_select_b3_protocol );

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command 
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_select_b3_protocol);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci); 

  // 7. Gewünschtes Protokoll
  // 0x04 = Transparent
  //
  schreibe_BYTE(gp_capi_msg + 10, 0x04);

  // 8. Länge NCPD = Network-Control-Protocol-Description
  //
  schreibe_BYTE(gp_capi_msg + 11, 0x00);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF select_b3_protocol
//------------------------

//----------------------
// Sende Connect-B3-Request
//
int sende_connect_b3_request(WORD par_plci)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_connect_b3_request);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command = Connect
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_connectb3_command);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci); 

  // 7. PLCI NCPI_LEN
  // Länge der Zusatzangaben für NCPI = 00
  //
  schreibe_BYTE(gp_capi_msg + 10, 0x00);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

  
}
// END OF sende_connect_b3_request
//------------------------

//-------------------
// Eine Capi-Nachricht liefern.
//
int hole_capi_nachricht(long par_rcv_pointer)
{
  int l_status;

  l_status = my_API_GET_MESSAGE(g_app_id, (char far * far *)par_rcv_pointer);

  return l_status;
}
// END OF hole_capi_nachricht
//-----------------------

//-------------------
// Beantworten von Schicht 3
// Anforderungen aktivieren.
//
int aktiviere_schicht3_beantwortung(WORD par_plci)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_listenb3_request);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command = Listen
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_listenb3_command);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci); 

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

  
}
// END OF aktiviere_schicht3_beantwortung
//--------------------------

//------------------------
// Connect Response senden.
//
int sende_connect_response(WORD par_plci, WORD par_indication_nr)
{

  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_connect_response);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command = 
  //
  schreibe_BYTE(gp_capi_msg + 4, c_connect_command);

  // 4. SubCommand = Request
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_indication_nr);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci);

  // 7. Reject Status (= 0 => kein Reject!)
  //
  schreibe_BYTE(gp_capi_msg + 10, 0x00); 

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF sende_connect_respone
//-------------------------

//---------------------
// Schicht 1 aktiv Meldung bestätigen.
//
int bestaetige_schicht_1_aktiv(WORD par_plci, WORD par_indication_nr)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_connect_active_rsp);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, c_connect_active_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_indication_nr);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

  
}
// END OF bestaetige_schicht_1_aktiv
//----------------------------------

//-------------------
// Bestätigung Schicht 3 Anfrage senden.
//
int sende_connect_b3_response(WORD par_ncci, WORD par_msg_nr)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_connectb3_response);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_connectb3_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_msg_nr);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_ncci);

  // 7. Reject Status (= 0 >= kein Reject!)
  //
  schreibe_BYTE(gp_capi_msg + 10, 0x00);

  // 8. NCPI Länge (??)
  //
  schreibe_BYTE(gp_capi_msg + 11, 0x00);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF sende_connect_b3_response
//-----------------------

//---------------------
// Schicht 3 aktiv Meldung bestätigen.
//
int bestaetige_schicht_3_aktiv(WORD par_ncci, WORD par_indication_nr)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_connect_b3_active_rsp);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_connect_b3_active_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_indication_nr);

  // 6. NCCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_ncci);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

  
}
// END OF bestaetige_schicht_3_aktiv
//----------------------------------

//---------------------
// Schicht 3 Abbau bestätigen.
//
int bestaetige_schicht_3_abbau(WORD par_ncci, WORD par_indication_nr)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_disconnect_b3_rsp);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_disconnect_b3_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_indication_nr);

  // 6. NCCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_ncci);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF bestaetige_schicht_3_abbau
//--------------------------

//--------------------
// Schicht 3 abbauen.
//
int sende_disconnect_b3_request(WORD par_ncci)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_disconnect_b3_req);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_disconnect_b3_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. NCCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_ncci);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF sende_disconnect_b3_request
//--------------------

//--------------------
// Verbindung trennen.
//
int sende_disconnect_request(WORD par_plci)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_disconnect_request);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, c_disconnect_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci);

  // 7. Grund des Abbaus.
  // 00 = Alles okay, Verbindungsabbau ist gewollt.
  //
  schreibe_BYTE(gp_capi_msg + 10, 0x00);


  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF sende_disconnect_request
//--------------------------

//------------------------
// Verbindungsabbau bestätigen.
//
int bestaetige_verbindungsabbau(WORD par_plci, WORD par_indication_nr)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_disconnect_response);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, c_disconnect_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_indication_nr);

  // 6. PLCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_plci);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF bestaetige_verbindungsabbau
//---------------------------

//------------------
// Daten Request.
//
int sende_daten_request(WORD par_ncci, LPSTR par_daten, WORD par_len, BYTE par_block)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_data_request);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_data_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_request);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, g_req_msg_nummer++ & 0x7fff);

  // 6. NCCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_ncci);

  // 7. Länge der Daten.
  //
  schreibe_WORD(gp_capi_msg + 10, par_len);

  // 8. Pointer auf Daten
  //
  schreibe_DWORD(gp_capi_msg + 12, par_daten);

  // 9. Blocknummer (wird später von der Confirmation
  // benutzt).
  //
  schreibe_BYTE(gp_capi_msg + 16, par_block);

  // 10. Flags für Zusatzdienste. Im Moment nicht benutzt.
  //
  schreibe_WORD(gp_capi_msg + 17, 0x0000);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;
}
// END OF sende_daten_request
//------------------

//------------------------
// Datenempfang bestätigen.
//
int bestaetige_datenempfang(WORD par_ncci, WORD par_indication_nr, BYTE par_block_nr)
{
  int l_status;

  // CAPI_Message bestücken.
  //

  // 1. Gesamtlänge
  //
  schreibe_WORD(gp_capi_msg, c_laenge_data_response);

  // 2. Applikation-ID
  //
  schreibe_WORD(gp_capi_msg + 2, g_app_id);

  // 3. Command  
  //
  schreibe_BYTE(gp_capi_msg + 4, (unsigned char) c_data_command);

  // 4. SubCommand
  //
  schreibe_BYTE(gp_capi_msg + 5, c_response);

  // 5. MessageNummer
  //
  schreibe_WORD(gp_capi_msg + 6, par_indication_nr);

  // 6. NCCI
  //
  schreibe_WORD(gp_capi_msg + 8, par_ncci);

  // 7. Blocknr
  //
  schreibe_BYTE(gp_capi_msg + 10, par_block_nr);

  // Jetzt Message absenden.
  //
  l_status = my_API_PUT_MESSAGE (g_app_id, gp_capi_msg);

  return l_status;

}
// END OF bestaetige_datenempfang
//---------------------------
