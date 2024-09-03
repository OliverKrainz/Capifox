//-----------------------
// Capi Client Program
//
// (C) Oliver Krainz 1996-2024
//
//-----------------------
#include <windows.h>
#include <iostream.h>
#include <string.h>
#include <fstream.h>

#include "capifunc.h"

static far char g_servernr[20] = "";

static HANDLE gh_data_buffer = 0;
static LPSTR  gp_data_buffer = NULL;

char      g_app_name[10];              // Enthält für das gesamte Programm den Namen der Anwendung.
HINSTANCE g_instance;                  // Instance-Handle der Anwendung.
HWND      g_main_window;               // Window-Handle des Anwendungsfensters.
HANDLE    g_main_font        = NULL;   // Font-Handle
HANDLE    g_cright_font      = NULL;   // Font-Handle Copyright Text
HANDLE    g_therm_rect_pen   = NULL;   // Handle für Pen-Thermometerumriss
HANDLE    g_therm_rect_brush = NULL;   // Handle für Brush-Thermometer
char      g_datei[100];                // Dateinamen für Bildschirmausgabe
char      g_status[100];               // Für Ausgabe Status
int       g_prozent = 0;               // Prozentfortschritt Thermometer
long      g_size = 0;                  // Zur Bestückung Thermometer

LPSTR     gp_rcvd_msg = NULL;
BOOL      g_ende_schleife = FALSE, g_online = FALSE;

WORD      g_plci, g_ncci, g_indication_nr;

BYTE      g_akt_aktion = 0;        // Enthält aktuelle Aktion
fstream   f_datei;                 // Stream für InfoAustauschdatei
fstream   f_download;              // Stream für Download
DWORD     g_download_akt_bytes;    // Anzahl der bisherigen DownloadBytes
DWORD     g_download_ges_bytes;    // Anzahl der insgesamt zu downloadenden Bytes.
BYTE      g_download_ziel[100];    // Zieldatei von Download

#define c_size_data_buffer 6200
#define c_block_size       2040
#define c_ende_schleife -1
#define c_weitermachen  -2

#define c_isdn_okay      "OKAY"
#define c_isdn_connect   "CONNECT"
#define c_isdn_error     "ERROR"
#define c_isdn_ende      'E'
#define c_isdn_command   'C'
#define c_isdn_download  'D'
#define c_isdn_downreq   "ISDN-DOWNLOAD"

#define c_res_error      "e"
#define c_res_disconnect "d"


#define c_res_error      "e"
#define c_res_disconnect "d"

#define c_res_download   'l'      // Kein String weil nicht über schreibe_fox_info ausgegeben.

#define c_akt_download     0x01
#define c_akt_download_req 0x02

char    g_puffer[c_block_size];

// Prototypen
//
// Prototypen
//
LRESULT CALLBACK message_handler(HWND par_window, UINT par_message,
				 WPARAM par_wparam, LPARAM par_lparam);
void             wmCreate(void);
void             wmPaint(HWND par_window, UINT par_message,
			 WPARAM par_wparam, LPARAM par_lparam);

int              schreibe_fox_info(char *, int);
int              fox_message_schleife(void);
void             init_download(char *);
void             starte_download(char *);
void             handle_download(LPSTR, int);

//--------------------
// x Sekunden warten.
//
void wait_x_sekunden(int par_sekunden)
{
  unsigned long l_start_zeit, l_ende_zeit;

  l_start_zeit = GetCurrentTime();
  l_ende_zeit = l_start_zeit + par_sekunden * 100;

  while ( (GetCurrentTime() >= l_start_zeit) && (GetCurrentTime() <= l_ende_zeit) )
    Yield();
}
// END OF wait_x_sekunden
//----------------------

//------------------
// Der Message-Handler.
// Wird von Windows bei jedem Event aufgerufen
// und muss die Events dann an die entsprechenden
// Funktionen weiterleiten.
//
LRESULT CALLBACK message_handler(HWND par_window, UINT par_message,
				 WPARAM par_wparam, LPARAM par_lparam)
{
    switch (par_message)
    {
	// Message WM_CREATE.
	// Wird von Create Window erzeugt. Wird aufgerufen bevor
	// das Fenster angezeigt wird.
        //
        case WM_CREATE:
          wmCreate();
	  break;

	// Message WM_PAINT.
	// Fensterinhalt neu aufbauen.
        //
        case WM_PAINT:
	  wmPaint(par_window, par_message, par_wparam, par_lparam);
	  break; 

        // Alle nicht explizit abgefangenen Botschaften müssen
        // über DefWindowProc ans Betriebssystem zur Standard-
        // verarbeitung weitergeleitet werden.
	//
	default: 
          return(DefWindowProc(par_window, par_message, par_wparam, par_lparam));

    }

    return 0L;
}
// END OF message_handler
//---------------

//-----------------
// Funktion für WindowsMessage
// WM_CREATE.
// Hier werden vor Aufbau des Hauptfensters die Font-
// und Farbeneinstellungen vorgenommen.
//
void wmCreate(void)
{
  LOGFONT   l_main_font;

  // Font-Data des Fensters definieren.
  //

  // Höhe.
  //
  l_main_font.lfHeight = 15;

  // Breite.
  //      
  l_main_font.lfWidth = 10;

  // Winkel zwischen Fluchtvektor und X-Achse
  // der Bildschirmoberfläche.
  //       
  l_main_font.lfEscapement = 0;

  // Winkel zwischen Basislinie und X-Achse.
  //    
  l_main_font.lfOrientation = 0;

  // Schriftstärke (in gedruckten Punkt pro 1000).
  //
  l_main_font.lfWeight = 300;

  // Kursiv.
  //
  l_main_font.lfItalic = 0;

  // Unterstrichen.
  //
  l_main_font.lfUnderline = 0;

  // Durchgestrichen.
  //
  l_main_font.lfStrikeOut = 0;

  // Zeichensatz.
  //
  l_main_font.lfCharSet = ANSI_CHARSET;

  // Ausgabegenauigkeit.
  //
  l_main_font.lfOutPrecision = OUT_DEFAULT_PRECIS;

  // Clip-Genauigkeit.
  //
  l_main_font.lfClipPrecision = CLIP_DEFAULT_PRECIS;

  // Ausgabequalität (bei Umsetzung virtuelle Schrift => physikalische)
  //
  l_main_font.lfQuality = PROOF_QUALITY;

  // Zeichenabstand und Schriftfamilie.
  //
  l_main_font.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;

  // Schriftart.
  //
  strcpy(l_main_font.lfFaceName,"Times New Roman");

  g_main_font = CreateFontIndirect( &l_main_font );

  // Font für Copyright-Text erstellen.
  //

  // Schriftart.
  //
  strcpy(l_main_font.lfFaceName,"Arial");

  // Höhe.
  //
  l_main_font.lfHeight = 11;

  // Breite.
  //      
  l_main_font.lfWidth = 8;

  g_cright_font = CreateFontIndirect( &l_main_font );

  // Pen für Thermometerumriss erstellen.
  //
  g_therm_rect_pen = CreatePen( PS_SOLID, 1, RGB( 128, 128, 128 ) );

  // Brush für Thermometer erstellen.
  //
  g_therm_rect_brush = CreateSolidBrush( RGB( 0, 128, 0 ) );

  // AnzeigeStrings leeren.
  //
  g_datei[0] = 0;
  g_status[0] = 0;
}
// END OF wmCreate
//-----------------------

//--------------------
// Fenster neu aufbauen.
//
void wmPaint(HWND par_window, UINT par_message,
	     WPARAM par_wparam, LPARAM par_lparam)
{
  HDC l_hdc;
  PAINTSTRUCT l_ps;
  RECT l_background;
  HANDLE l_old_pen, l_old_brush;
  int l_size;   
  char c_copyright[] = "Capifox (C) Oliver Krainz";

  // Ausgabe beginnen.
  //
  l_hdc = BeginPaint( par_window, &l_ps );

  GetClientRect( par_window, &l_background );

  // Hintergrund zeichnen.
  //
  FillRect(l_hdc, &l_background, GetStockObject(LTGRAY_BRUSH));

  // Textobjekt im Kontext selecten.
  //
  SelectObject( l_hdc, g_main_font );

  // Hintergrundmodus auf Transparent schalten.
  //
  SetBkMode(l_hdc, TRANSPARENT);

  // Farbe setzen.
  //
  SetTextColor(l_hdc, RGB(128,0,128));

  // Datei ausgeben.
  //
  TextOut(l_hdc, 5, 5, g_datei, strlen( g_datei ));

  // Farbe setzen.
  //
  SetTextColor(l_hdc, RGB(0,0,192));

  // StatusText ausgeben.
  //
  TextOut(l_hdc, 5, 25, g_status, strlen( g_status ));

  // Textobjekt im Kontext selecten.
  //
  SelectObject( l_hdc, g_cright_font );

  // CopyrightText ausgeben.
  //
  TextOut(l_hdc, 5, 70, &c_copyright[0], strlen( &c_copyright[0] ));

  if (g_akt_aktion == c_akt_download)
  {
    // Thermometerumriss zeichnen.
    //
    l_old_brush = SelectObject( l_hdc, GetStockObject( NULL_BRUSH ) );
    l_old_pen   = SelectObject( l_hdc, g_therm_rect_pen );
    MoveTo( l_hdc, 5, 45 );
    LineTo( l_hdc, 206, 45 );
    MoveTo( l_hdc, 5, 45 );
    LineTo( l_hdc, 5, 56 );
    SelectObject( l_hdc, GetStockObject( WHITE_PEN ) );
    MoveTo( l_hdc, 5, 55 );
    LineTo( l_hdc, 206, 55 );
    MoveTo( l_hdc, 205, 55 );
    LineTo( l_hdc, 205, 44 );

    // Eigentliches Thermometer zeichnen.
    //
    if ( g_prozent > 0 )
    {
      l_size = 6 + 2 * g_prozent - 2;
      l_old_brush = SelectObject( l_hdc, g_therm_rect_brush );
      Rectangle( l_hdc, 6, 46, l_size, 56 );
    }

    SelectObject( l_hdc, l_old_brush );
    SelectObject( l_hdc, l_old_pen );
  }

  // Ausgabe beenden.
  //
  EndPaint( par_window, &l_ps );  
}
// END OF wmPaint
//--------------------

//-------------------
// CAPI initialisieren.
//
int initialisieren(void)
{
  HANDLE l_status;
  int l_loop;

  // Datenbuffer der Anwendung initialisieren.
  //
  gh_data_buffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD) c_size_data_buffer);

  if (gh_data_buffer == NULL)
    return ERROR;

  gp_data_buffer = GlobalLock(gh_data_buffer);

  if (gp_data_buffer == NULL)
  {
    return ERROR;
  }

  l_status = load_capidll();

  if (l_status <= 32)
    return ERROR;

  if (!verbinde_funktionen())
    return ERROR;

  if (!init_capi_strukturen())
    return ERROR;

  if (!my_API_INSTALLED())
    return ERROR;

  return SUCCESS;
}
// END OF initialisieren
//------------------------

//-----------------
// Anwendung beim Capi anmelden.
//
int anmeldung(void)
{
  int l_status;

  l_status = capi_anmeldung();

  return l_status;
}
// END OF anmeldung
//-------------------

//--------------------
// Server anwählen.
//
int call_server(void)
{

  int l_status;

  l_status = sende_connect_request((LPSTR) &g_servernr[0]);

  return !l_status;
}
// END OF call_server
//----------------------

//---------------------
// Windows-Message-Schleife.
// Notwendig um Windows-Messages
// zu behandeln.
//
int win_message_schleife(void)
{
  MSG l_msg;

  // PeekMessage (liefert WindowsBotschaften die
  // für die Anwendung bestimmt sind) so lange
  // aufrufen, wie noch Nachrichten in der
  // Warteschlange sind.
  // Jede Nachricht dann einfach weitergeben und
  // aus der Warteschlange löschen (PM_REMOVE).
  // Interessant ist nur WM_QUIT um die Anwendung
  // zu beenden.
  //
  while (PeekMessage(&l_msg, 0, 0, 0, PM_REMOVE))
  {

    TranslateMessage(&l_msg);
    DispatchMessage(&l_msg);

    if (l_msg.message == WM_QUIT)
    {
      return c_ende_schleife;
    }
  }
  return c_weitermachen;
}
// END OF win_message_schleife
//--------------------------

//-----------------------
// Handle Connect_Confirmation.
//
void handle_connect_confirmation(void)
{
  WORD l_info;

  g_plci = hole_WORD(gp_rcvd_msg + 8);

  l_info = hole_WORD(gp_rcvd_msg + 10);

  if (l_info != NO_ERROR)
    g_ende_schleife = TRUE;
}
// END OF handle_connect_confirmation
//----------------------


//-----------------------
// Handle Aktivmeldung Schicht 1
//
void handle_connect_active_indication(void)
{
  int l_status;
  WORD l_msg_nr;
  
  l_msg_nr = hole_WORD(gp_rcvd_msg + 6);

  l_status = bestaetige_schicht_1_aktiv(g_plci, l_msg_nr);

  if (l_status != NO_ERROR)
    g_ende_schleife = TRUE;

  l_status = select_b2_protocol(g_plci);

  if (l_status != NO_ERROR)
    g_ende_schleife = TRUE;

  // Schicht 3 aufbauen.
  //
  l_status = sende_connect_b3_request(g_plci);

  if (l_status != NO_ERROR)
  {
    g_ende_schleife = TRUE;

    // Verbindung abbauen.
    //
    sende_disconnect_request(g_plci);
  }

}
// END OF handle_connect_active_indication
//----------------------------

//-----------------------
// Handle Select_b2_protocol_confirmation
//
void handle_select_b2_protocol_confirmation(void)
{
  int l_info, l_status;

  l_info = hole_WORD(gp_rcvd_msg + 10);

  // Ist ein Fehler aufgetreten?
  //
  if (l_info != NO_ERROR)
  {
//    cout << "ERROR: " << hex << l_info;
  }

  // B3-Protokoll einstellen.
  //
  l_status = select_b3_protocol(g_plci);

  if (l_status != NO_ERROR)
    g_ende_schleife = TRUE;
}
// END OF handle_select_b2_protocol_confirmation
//-----------------------

//------------------------
// Handle Select_b3_protocol_confirmation
//
void handle_select_b3_protocol_confirmation(void)
{
  int l_info, l_status;

  l_info = hole_WORD(gp_rcvd_msg + 10);

  // Ist ein Fehler aufgetreten?
  //
  if (l_info != NO_ERROR)
  {
//    cout << "ERROR: " << hex << l_info;
  }

}
// END OF handle_select_b3_protocol_confirmation
//-------------------------

//-------------------
// Handle Connect_b3_confirmation.
//
void handle_connect_b3_confirmation(void)
{
  WORD l_info;

  g_ncci = hole_WORD(gp_rcvd_msg + 10);

  l_info = hole_WORD(gp_rcvd_msg + 12);

  if (l_info != NO_ERROR)
  {
    g_ende_schleife = TRUE;

    // Verbindung abbauen.
    //
    sende_disconnect_request(g_plci);

  }

}
// END OF handle_connect_b3_confirmation
//-------------------------

//-------------------
// Aktivmeldung Schicht 3
//
void handle_connect_b3_active_indication(void)
{
  int l_status;
  WORD l_msg_nr;

  l_msg_nr = hole_WORD(gp_rcvd_msg + 6);

  l_status = bestaetige_schicht_3_aktiv(g_ncci, l_msg_nr);

  if (l_status != NO_ERROR)
    g_ende_schleife = TRUE;

  g_online = TRUE;

  schreibe_fox_info(c_isdn_connect, 0);
}
// END OF handle_connect_b3_active_indication
//-------------------------

//-----------------
// Ankommende Daten.
//
void handle_data_b3_indication(void)
{
  WORD  l_len, l_msg_nr;
  LPSTR l_p_data;
  BYTE  l_block_nr;
  char  l_cmdline[2050];
  int   l_loop;

  l_len      = hole_WORD(gp_rcvd_msg + 10);
  l_p_data   = (LPSTR) hole_DWORD(gp_rcvd_msg + 12);
  l_msg_nr   = hole_WORD(gp_rcvd_msg + 6);
  l_block_nr = hole_BYTE(gp_rcvd_msg + 16);


  // Weitergabe der Daten in
  // Abhängigkeit von der aktuellen
  // Aktion.
  //
  switch (g_akt_aktion)
  {
     int l_loop;

     // Keine aktuelle Aktion?
     // => Daten weitergeben an FoxPro.
     //
     case 0:
       for (l_loop = 0; l_loop < l_len; l_loop++)
         l_cmdline[l_loop] = *(l_p_data + l_loop);

       schreibe_fox_info(&l_cmdline[0], l_len);
       break;

     // Antwort auf Donwloadrequest?
     //
     case c_akt_download_req:
       for (l_loop = 0; l_loop < l_len; l_loop++)
	 l_cmdline[l_loop] = *(l_p_data + l_loop);

       starte_download(&l_cmdline[0]);
       break;

     // Läuft zur Zeit ein Download?
     //
     case c_akt_download:
       handle_download(l_p_data, l_len);
       break;
  }

  bestaetige_datenempfang(g_ncci, l_msg_nr, l_block_nr);

}
// END OF handle_data_b3_indication
//---------------------


//--------------------
// Bestätigung Data Request.
//
void handle_data_b3_confirmation(void)
{
  int l_status;

}
// END OF handle_data_b3_confirmation
//---------------------

//-----------------------
// Abbau Schicht 3
//
void handle_disconnect_b3_indication(void)
{
   int l_info, l_status;
   WORD l_msg_nr;

   l_info = hole_WORD(gp_rcvd_msg + 10);

   // Planmäßiger Abbau oder Abbau durch Fehler?
   //
   if ( (l_info != NO_ERROR) && (g_online == FALSE) )
     g_ende_schleife = TRUE;

   l_msg_nr = hole_WORD(gp_rcvd_msg + 6);

   l_status = bestaetige_schicht_3_abbau(g_ncci, l_msg_nr);

   if (l_status != NO_ERROR)
     g_ende_schleife = TRUE;

   // Verbindung abbauen.
   //
   l_status = sende_disconnect_request(g_plci);

}
// END OF handle_disconnect_b3_indication
//-----------------------

//-----------------------
// Bestätigung über gewünschten B3-Disconnect.
//
void handle_disconnect_b3_confirmation(void)
{
   int l_info;

   l_info = hole_WORD(gp_rcvd_msg + 10);

   if (l_info != NO_ERROR)
   {
     // Verbindung abbauen.
     //
     sende_disconnect_request(g_plci);
   }

}
// END OF handle_disconnect_b3_confirmation
//--------------------

//---------------------
// Verbindungsabbau.
//
void handle_disconnect_indication(void)
{
  WORD l_msg_nr, l_info;

  l_info = hole_WORD(gp_rcvd_msg + 10);
  l_msg_nr = hole_WORD(gp_rcvd_msg + 6);

  // Planmäßiger Abbau oder Abbau durch Fehler?
  //
  if ( !g_online )
  {
    switch ( l_info )
    {
      case 0x3301:
	schreibe_fox_info( strcat( c_res_disconnect,
				   "ERRORDie ISDN-Karte ist nicht an den S0-Bus angeschlossen!" )
			   , 0 );
        break;

      case 0x348a:
	schreibe_fox_info( strcat( c_res_disconnect,
				   "ERRORIhre B-Kanäle sind belegt." )
			   , 0 );
        break;

      case 0x34bb:
	schreibe_fox_info( strcat( c_res_disconnect,
				   "ERRORDer Online-Server ist besetzt." )
			   , 0 );
        break;

      case 0x34ba:
	schreibe_fox_info( strcat( c_res_disconnect,
				   "ERRORDer Online-Server meldet sich nicht." )
			   , 0 );
        break;

      default:
        schreibe_fox_info( c_res_disconnect, 0 );
    }
  }
  else
    schreibe_fox_info(c_res_disconnect, 0);

  bestaetige_verbindungsabbau(g_plci, l_msg_nr);

  g_ende_schleife = TRUE;

  g_online = FALSE;

}
// END OF handle_disconnect_indication
//------------------------

//----------------------
// Capi-Schleife, wartet
// bis Programm beendet wird.
//
void capi_schleife(void)
{
  int  l_status;
  BYTE l_capi_command, l_capi_subcommand;
  WORD l_capi_combi_command;
  MSG  l_msg;

  while (!g_ende_schleife)
  {
    if (win_message_schleife() == c_ende_schleife)
      g_ende_schleife = TRUE;

    // Nachschauen, ob es eine neue Nachricht
    // vom Capi gibt.
    //
    l_status = hole_capi_nachricht((long)&gp_rcvd_msg);

    if ( (l_status != NO_ERROR) && (l_status != c_keine_nachricht) )
    {
      g_ende_schleife = TRUE;
      continue;
    }

    // Keine Nachricht vorhanden?
    //
    if (l_status == c_keine_nachricht)
    {
      // Diesen Umstand nutzen um die Windows
      // Nachrichten abzuarbeiten.
      //
      if (win_message_schleife() == c_ende_schleife)
	g_ende_schleife = TRUE;

      // Jetzt noch die Messages von FoxPro
      // abarbeiten.
      //
      if ( !g_akt_aktion )
      {

        if ( fox_message_schleife() == c_ende_schleife ) 
	  g_ende_schleife = TRUE;


      }
      continue;
    }

    // Ansonsten die empfangene Nachricht
    // verarbeiten.
    //
    l_capi_command       = hole_BYTE(gp_rcvd_msg+4);
    l_capi_subcommand    = hole_BYTE(gp_rcvd_msg+5);
    l_capi_combi_command = combi_cmd_subcmd(l_capi_command, l_capi_subcommand);

    switch (l_capi_combi_command)
    {

      // Verbindungsanfrage => Bestätigung.
      //
      case c_connect_confirmation:

	handle_connect_confirmation();
	break;

      // Aktivmeldung Schicht 1
      //
      case c_connect_active_indication:

	handle_connect_active_indication();
	break;

      // Bestätigung Schicht 2 Protokolleinstellung.
      //
      case c_select_b2_prot_confirmation:
	handle_select_b2_protocol_confirmation();
        break;

      // Bestätigung Schicht 3 Protokolleinstellung.
      //
      case c_select_b3_prot_confirmation:
	handle_select_b3_protocol_confirmation();
        break;

      // Schicht 3 Anfrage => Bestätigung.
      //
      case c_connect_b3_confirmation:

	handle_connect_b3_confirmation();
	break;

      // Aktivmeldung Schicht 3
      //
      case c_connect_b3_active_indication:

	handle_connect_b3_active_indication();
        break;

      // Ankommende Daten.
      //
      case c_data_b3_indication:

	handle_data_b3_indication();
	break;

      // Bestätigung Data-Request.
      //
      case c_data_b3_confirmation:

	handle_data_b3_confirmation();
	break;

      // Abbau Schicht 3 => Bestätigung.
      //
      case c_disconnect_b3_confirmation:

	handle_disconnect_b3_confirmation();
	break;

      // Abbau Schicht 3
      //
      case c_disconnect_b3_indication:

	handle_disconnect_b3_indication();
	break;

      // Abbau der Verbindung => Bestätigung.
      //
      case c_disconnect_confirmation:
	break;

      // Abbau Verbindung.
      //
      case c_disconnect_indication:

	handle_disconnect_indication();
	break;

      // Ansonsten unbekannte Nachricht.
      //
      default:
	break;

    } // switch (l_capi_combi_command)

  } // while (!g_ende_schleife)

}
// END OF capi_schleife
//-------------------

//-----------------------
// Alles wieder aufraeumen.
//
void aufraeumen()
{
  free_capi_id();

  free_capi_memory();

  release_capidll();

  if ( gh_data_buffer != NULL )
  {
    GlobalUnlock( gh_data_buffer );
    GlobalFree( gh_data_buffer );
  }

  if ( g_main_font != NULL )
    DeleteObject( g_main_font );

  if ( g_cright_font != NULL )
    DeleteObject( g_cright_font );

  if ( g_therm_rect_pen != NULL )
    DeleteObject( g_therm_rect_pen );

  if ( g_therm_rect_brush != NULL )
    DeleteObject( g_therm_rect_brush );              

}
// END OF aufraeumen
//---------------------

//-----------------------
// Einen Text in die Austauschdatei schreiben.
//
int schreibe_fox_info(char * par_string, int par_len)
{
  unsigned long l_ende_zeit;
  char l_kennung = 'F';
  BOOL l_endwait = FALSE, l_status = FALSE;

  l_ende_zeit = GetCurrentTime() + 2000;

  while ( !l_endwait )
  {
    f_datei.open("capiclie.inr", ios::binary | ios::out | ios::trunc);

    if (f_datei.fail())
    {
      if ( GetCurrentTime() > l_ende_zeit )
	l_endwait = TRUE;

      continue;
    }

    f_datei.write(&l_kennung, 1);

    if (!par_len)
      f_datei.write(par_string, strlen(par_string));
    else
      f_datei.write(par_string, par_len);

    f_datei.close();

    l_status = TRUE;
    l_endwait = TRUE;
  }

  if ( !l_status )
    return ERROR;
  else
    return SUCCESS;
}
// END OF schreibe_fox_info
//----------------------

//-----------------------
// Download initialisieren.
//
void init_download(char *par_p_cmdline)
{
  BYTE  l_datei[50];
  int   l_pos = 0, l_pos2 = 0;
  int l_loop;
  BYTE  l_ch;
  char  l_command[100];

  l_pos = strlen( c_isdn_downreq );

  // Dateinamen lesen.
  //
  while ( (l_ch = *par_p_cmdline++) > 0 )
  { 
    l_datei[l_pos2++] = l_ch;
    l_command[l_pos++] = l_ch;
  }

  l_datei[l_pos] = 0;

  for (l_loop = 0; l_loop <= l_pos ; l_loop++)
    *(gp_data_buffer+l_loop) = l_command[l_loop];

  sende_daten_request(g_ncci, gp_data_buffer, l_pos + 5, 1);

  // Zieldatei ermitteln und abspeichern.
  //
  l_pos2 = 0;

  while ( (l_ch = *par_p_cmdline++) > 0 )
    g_download_ziel[l_pos2++] = l_ch;

  // Aktion auf DownloadRequest setzen.
  //
  g_akt_aktion = c_akt_download_req;

}
// END OF init_download
//-----------------------

//-------------------
// Download starten.
//
void starte_download(char * par_p_cmdline)
{
  char l_ch;

  // Dateinamen überlesen, wird im Moment
  // nicht verwendet.
  //
  while ( (l_ch = *(par_p_cmdline)) > 0 )
    par_p_cmdline++;

  // Zeiger auf Größenangabe bewegen.
  //
  par_p_cmdline++;

  movmem(par_p_cmdline, &g_download_ges_bytes, 4);
  g_size = g_download_ges_bytes;

  // Fenster mit Thermometer anzeigen.
  // Aber nur, wenn mehr als 10000 Bytes
  // übertragen werden.
  //
  if ( g_size > 10000 )
  {
    g_prozent = 0;
    strcpy( &g_status[0], "Dateiübertragung..." );


    ShowWindow(g_main_window, SW_SHOWNORMAL);
  }

  // Größe = 0?
  // => Download wurde verweigert.
  //
  if (g_download_ges_bytes == 0)
  {
    // FoxPro das mitteilen.
    //
    schreibe_fox_info("ERRORDownload verweigert.", 0);

    // Aktionstatus zurücksetzen.
    //
    g_akt_aktion = 0;
  }
  else
  {
    // Zieldatei öffnen und leeren.
    //
    f_download.open(&g_download_ziel[0], ios::binary | ios::out | ios::trunc);

    // Aktionsstatus ändern.
    //
    g_akt_aktion = c_akt_download;

    // Gelesene Bytes zurücksetzen.
    //
    g_download_akt_bytes = 0x00L;

  }
}
// END OF starte_download
//------------------------

//--------------------------
// Handle Download
//
void handle_download(LPSTR par_p_data, int par_len)
{
  int  l_loop;

  // Daten umkopieren, leider notwendig da
  // die Streams keine far Daten verarbeiten.
  //
  for (l_loop = 0; l_loop < par_len; l_loop++)
    g_puffer[l_loop] = *(par_p_data + l_loop);

  f_download.write(&g_puffer[0], par_len);

  g_download_akt_bytes += par_len;

  // Thermometer updaten.
  //
  if ( g_download_ges_bytes > 10000 )
  {
    g_prozent =  g_download_akt_bytes / (float) g_download_ges_bytes * 100.0;
    InvalidateRect( g_main_window, NULL, FALSE );
    win_message_schleife();
  }

  // Downloadende erreicht?
  //
  if (g_download_akt_bytes >= g_download_ges_bytes)
  {
    f_download.close();

    g_akt_aktion = 0;

    schreibe_fox_info("DOWNLOADENDE", 0);

    // Fenster wieder verstecken.
    //
    ShowWindow(g_main_window, SW_HIDE);

  }
}
// END OF handle_download
//------------------------

//-----------------------
// FoxMessageSchleife prüft auf Commandos
// von FoxPro.
//
int fox_message_schleife(void)
{
  BYTE l_ch = 0;
  BYTE l_cmd;
  char l_cmd_line[2050];
  int l_last_read, l_loop;


  f_datei.open("capiclie.ins", ios::binary | ios::in);

  // Datei kann nicht geöffnet werden?
  // => gerade in Bearbeitung von FoxPro.
  //
  if (f_datei.fail())
    return 0;

  // Nachschauen, ob 1. Zeichen ein "D" oder "C" ist.
  //
  f_datei.read(&l_ch, 1);

  if ( ( l_ch != 'D' ) && ( l_ch != c_isdn_command ) )
  {
    f_datei.close();
    return 0;
  }

  // Wenn es sich um ein Commando handelt:
  // Commando lesen.
  //
  if ( l_ch == c_isdn_command )
    f_datei.read(&l_cmd, 1);

  // Rest der Commandozeile lesen.
  //
  f_datei.read(&l_cmd_line[0], 2050);
  l_last_read = f_datei.gcount();

  f_datei.close();

  // Datei leeren.
  //
  f_datei.open("capiclie.ins", ios::binary | ios::out | ios::trunc);
  f_datei.close();

  // Wenn es sich um Daten für das Frontend
  // handelt diese einfach weitergeben.
  //
  if ( l_ch == 'D' )
  {
    for (l_loop = 0; l_loop < l_last_read; l_loop++)
      *(gp_data_buffer + l_loop) = l_cmd_line[l_loop];

    sende_daten_request(g_ncci, gp_data_buffer, l_last_read, 1);

    return c_weitermachen;
  }

  switch (l_cmd)
  {
    case c_isdn_download:
      init_download(&l_cmd_line[0]);
      break;


    case c_isdn_ende:
      return c_ende_schleife;
  }

  return c_weitermachen;

}
// END OF fox_message_schleife
//-------------------------

//------------------------
// Windowklasse der Anwendung definieren.
//
void init_window_class(HINSTANCE par_instance)
{
  WNDCLASS cl_window_class;

  // Name der Klasse (frei vergebbar).
  //
  cl_window_class.lpszClassName = g_app_name;

  // Instanzenhandler der Anwendung.
  //
  cl_window_class.hInstance     = par_instance;

  // Funktion der Anwendung die das Messagehandling übernimmt.
  //
  cl_window_class.lpfnWndProc   = message_handler;

  // Cursor definieren.
  // Wir verwenden einen der vordefinierten Zeiger (1.Parameter NULL),
  // und zwar den normalen Pfeil (IDC_ARROW).
  //
  cl_window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);

  // Welches Icon soll verwendet werden (z.B. beim minimieren).
  // Wir verwenden das normale Icon (also keines).
  //
  cl_window_class.hIcon         = NULL;

  // Welches Menu soll standardmäßig bei Fenster dieser Anwendung
  // verwendet werden.
  // In unserem Fall gar keins.
  //
  cl_window_class.lpszMenuName  = NULL;

  // Welcher Hintergrund soll verwendet werden.
  // Wir nehmen den Standard-Hintergrund.
  //
  cl_window_class.hbrBackground = COLOR_WINDOW + 1;

  // Den Klassenstil festlegen.
  // Wir wählen Fenster ohne Close-Option.
  //
  cl_window_class.style         = CS_HREDRAW | CS_VREDRAW;

  // Bei den folgenden zwei Komponenten kann man
  // zusätzliche Bytes zur Klassendefinition resevieren.
  //
  cl_window_class.cbClsExtra    = 0;
  cl_window_class.cbWndExtra    = 0;

  // Jetzt die Klasse beim Betriebssystem registrieren.
  //
  RegisterClass(&cl_window_class);
}
// END OF init_window_class
//-----------------

//-----------------
// Anwendungsfenster definieren.
//
void init_app_window(HINSTANCE par_instance)
{
  // Anwendungshandle in globaler Variable speichern.
  //
  g_instance = par_instance;

  // Hauptfenster erstellen.
  //
  g_main_window = CreateWindow
                  (
                    g_app_name,              // Window Klasse
		    "CAPI-Client v 2.0",     // Window title
		    WS_OVERLAPPEDWINDOW,     // Windowtyp
		    CW_USEDEFAULT,           // x-Koordinate des Fensters
		    0,                       // y-Koordinate oder Fensterdarstellung
                    350,                     // Breite des Fensters
                    120,                      // Höhe
                    NULL,                    // Dieses Fenster hat kein parent Window
                    NULL,                    // Es wird das Klassenmenu benutzt.
                    par_instance,            // Der Erschaffer
                    NULL                     // no parms to pass on
                  );
  // Und anzeigen.
  //
//  ShowWindow(g_main_window, SW_SHOWNORMAL);

}
// END OF init_app_window
//-----------------


//--------------------
// Anwendung initialisieren.
// Der mühsame Teil.
//
void init_application(HINSTANCE par_instance)
{
  const char c_app_name[] = "CAPICLIE";

  // Zuerst den Anwendungsnamen festlegen.
  //
  movmem(c_app_name, &g_app_name[0], 9);

  // WindowKlasse definieren und registrieren.
  //
  init_window_class(par_instance);

  // Anwendungsfenster definieren.
  //
  init_app_window(par_instance);

}
// END OF init_application
//----------------------

//------------------------
// Main().
// Wird beim Starten der Anwendung zuerst gerufen.
// Wird als den Pascal-Aufrufkonventionen folgend definiert (muss so sein).
//
// Parameter:
// par_instance      - Das eindeutige Instanzen-Handle der Anwendung.
// par_last_instance - Das Instanzen-Handle der letzten Instanz der Anwendung.
//                     Ist 0 bei der ersten Instanz.
// par_p_cmdline     - Ein char far* auf die Command line die die Anwendung
//                     gestartet hat.
// par_cmd_show      - Gewünschter Window-Status (normal, minimiert etc)
//
// Rückgabewert:
// Integerwert	     - Von Windows nicht benutzt. Z.B. fürs debuggen.
//
#pragma argsused
int PASCAL WinMain(HINSTANCE par_instance, HINSTANCE par_last_instance,
                   LPSTR par_p_cmdline, int par_cmd_show)
{

  unsigned char l_loop = 0, l_ch;

  while ((l_ch = *par_p_cmdline) > 32)
  {
    g_servernr[l_loop++] = l_ch;
    par_p_cmdline++;
  }
  g_servernr[l_loop] = 0;

  if (!initialisieren())
  {
    schreibe_fox_info("ERRORFehler beim Initialisieren.", 0);
    return ERROR;
  }

  if (!anmeldung())
  {
    schreibe_fox_info("ERRORFehler bei CAPI-Anmeldung.", 0);
    return ERROR;
  }

  if (!call_server())
  {
    schreibe_fox_info("ERRORFehler beim Verbindungsaufbau.", 0);
    return ERROR;
  }

  init_application( par_instance );

  capi_schleife();

  aufraeumen();

  return SUCCESS;
}
// END OF WinMain()
//--------------------