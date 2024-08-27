**********************
* CAPI-Client Beispielprogramm
*
* (C) OK-Soft Oliver Kocks
* http://www.ok-soft.com
*
* Freeware
* - Nutzung auf eigene Gefahr
* - keinerlei Gewährleistung
* - keine kommerzielle Nutzung ohne unsere Einwilligung
* - Weitergabe nur komplett ohne Änderungen
*

* Konstanten 
*
#define c_isdn_connect    "CONNECT"
#define c_res_disconnect "d"
#define c_res_download   "l"
#define c_isdn_okay       "OKAY"
#define c_res_status     "s"
#define c_res_error      "e"

#define c_isdn_ende       "E"

REGIONAL l_antwort, l_cmd, l_pos, l_datei, l_long, l_groesse

  * Einstellungen festlegen.
  *
  DO set_settings
  
  * Initialisieren.
  *
  DO initialisieren
  
  ? "Initialisiere CAPI-Handler."

  * Externen Capiclient starten.
  * Die telefonnr ist die Zielnr. des
  * Servers ohne irgendwelche Sonderzeichen
  * wie "/" o.a.
  *
  RUN /N capiclie.exe 978831

  * Auf Bestätigung des Capiclients warten.
  *
  l_antwort = ""

  l_antwort = warte_auf_antwort()   && PL>

  ? "Antwort:"
  ? l_antwort

  IF ( l_antwort != c_isdn_okay )
    ? "Fehler beim Initialisieren"
    
    * Capiclient beenden.
    *
    DO write_cmd_capiclient WITH c_isdn_ende
    RETURN
  ENDIF

  * Jetzt auf Connect warten.
  *
  l_antwort = warte_auf_antwort()   && PL>

  ? "Antwort:"
  ? l_antwort

  IF ( l_antwort != c_isdn_connect )
    ? "Fehler beim Warten auf Connect."
    
    * Capiclient beenden.
    *
    DO write_cmd_capiclient WITH c_isdn_ende
    RETURN
  ENDIF

  l_antwort = ""

  * Dateidownload anfordern.
  *
  DO write_cmd_capiclient WITH c_res_download + "logoserv.bmp" + CHR(0) + ;
                               "logoclie.bmp" + CHR(0)
  
  l_antwort = warte_auf_antwort()   && PL>
  
  ? "Anwort:"
  ? l_antwort

  * Externe Exe beenden.
  *
  DO write_cmd_capiclient WITH c_isdn_ende

RETURN
* END OF main()
********************

*****************
* Auf Antwort vom Capi-Handler
* warten.
*
FUNCTION warte_auf_antwort
REGIONAL l_filehandle, l_endwait, l_antwort, l_lastread

  l_endwait = .F.
  
  l_antwort = ""
  
  DO WHILE ( !l_endwait )
  
    * InfoaustauschDatei öffnen.
    *
    l_filehandle = FOPEN("capiclie.inf")
    
    * Konnte sie geöffnet werden?
    *
    IF ( l_filehandle != -1 )

      l_antwort = ""
      
      * Kompletten Inhalt lesen.
      *
      l_lastread = "Start"
      DO WHILE ( !FEOF(l_filehandle) AND ( !EMPTY( l_lastread ) ) )
        l_lastread = FREAD(l_filehandle, 100)
        l_antwort = l_antwort + l_lastread
      ENDDO

      = FCLOSE(l_filehandle)
      
      
      * Ist die Datei nicht empty und steht irgendwas
      * für FoxPro drin? (Kennzeichen = "F")
      *
      IF !EMPTY(l_antwort) AND (LEFT(l_antwort, 1) = "F")

        * Jetzt Datei leeren.
        *
        l_filehandle = FCREATE("capiclie.inf")
        = FCLOSE(l_filehandle)
        
        l_endwait = .T.
      ENDIF
      
    ELSE
    
      * Ansonsten eine halbe Sekunde Pause.
      *
      = INKEY(2)
    ENDIF
  ENDDO
  
  * DateiInhalt zurückgeben, abzüglich der Kennung
  * für FoxPro ("F")
  *
  
RETURN SUBSTR(l_antwort, 2, LEN(l_antwort) - 1)
* END OF warte_auf_antwort
**********************

******************
* Ein Command an den CapiClient
* geben.
*
PROCEDURE write_cmd_capiclient
PARAMETER par_command
  
  * Kennung für CapiClient ergänzen.
  *
  par_command = "C" + par_command + CHR(0)
  
  l_endwait = .F.

  l_try = 1
    
  l_antwort = ""
  
  ? "Schreibe an C: " + par_command 
  
  DO WHILE (!l_endwait)

    * Versuchen Datei zu öffnen.
    *  
    l_filehandle = FCREATE("capiclie.inf")
    
    * Öffnen erfolgreich?
    *
    IF ( l_filehandle != -1 )
    
      * Befehl schreiben.
      *
      = FWRITE(l_filehandle, par_command)
      = FCLOSE(l_filehandle)
      
      l_endwait = .T.
      
    ELSE
    
      * Ansonsten etwas warten, nach
      * 5 Versuchen ist aber Schluß.
      * Kann theoretisch nie vorkommen,
      * aber man weiß ja nie...
      *
      IF l_try < 5
        = INKEY(1)
        l_try = l_try + 1
      ELSE
        l_endwait = .T.
      ENDIF
      
    ENDIF
  ENDDO
  
RETURN
* END OF write_cmd_capiclient
****************************

*******************
* Einstellungen festlegen.
*
PROCEDURE set_settings

  *** Voreinstellungen
  *
  SET TALK OFF
  
  * Console ein
  *
  SET CONSOLE ON

  * Keine gelöschten Sätze berücksichtigen.
  *
  SET DELETED ON

  * Exakte-Übereinstimmung bei Vergleichen ausschalten.
  *
  SET EXACT OFF

  * Nach erfolgloser Suche ähnlichsten Satz NICHT anspringen.
  *
  SET NEAR OFF

  * Jahreszahl 4-stellig.
  *
  SET CENTURY ON

  * Deutsches Datumsformat.
  *
  SET DATE TO german

  * Kein automatisches Verlassen der Felder.
  *
  SET CONFIRM ON

  * Das nervige Gepiepse deaktivieren.
  *
  SET BELL OFF
 
  * Keine Schutzabfragen ob Dateien überschrieben werden sollen.
  *
  SET SAFETY OFF
 
  * Automatische Flushen der Buffer ausschalten.
  *
  SET AUTOSAVE OFF
RETURN       
* END OF set_settings
***************************

********************
* Initialisieren.
*
PROCEDURE initialisieren

  * Einfaches Fenster definieren.
  *
  DEFINE WINDOW capiclie AT 0,0 SIZE 30,80 FONT "fixedsys", 9 ;
         SYSTEM ;
         TITLE "Capiclient"

  MOVE WINDOW capiclie CENTER       
  ACTIVATE WINDOW capiclie   
RETURN
* END OF initialisieren
**************************

**********************
* Format eine Zahl im Character-
* format in eine Long Zahl um.
*
FUNCTION chr_to_long
PARAMETER par_chr
REGIONAL l_long

  l_long = ASC( SUBSTR( par_chr, 1, 1 ) )       + ;  
           ASC( SUBSTR( par_chr, 2, 1 ) ) * 256 + ;
           ASC( SUBSTR( par_chr, 3, 1 ) ) * 65536 + ;
           ASC( SUBSTR( par_chr, 4, 1 ) ) * 16777216
           
RETURN l_long
* END OF chr_to_long
**************************           

********************
* Formt eine Zahl im
* numerischen FoxPro Format
* ins Chr-Long Format um.
*
FUNCTION long_to_chr
PARAMETER par_long
REGIONAL l_chr, l_num1, l_num2, l_num3, l_num4

  l_num1 = INT( par_long / 16777216 )
  par_long = par_long - l_num1 * 16777216
  
  l_num2 = INT( par_long / 65536 )
  par_long = par_long - l_num2 * 65536

  l_num3 = INT( par_long / 256 )
  par_long = par_long - l_num3 * 256
  
  l_num4= par_long

  l_chr  = CHR( l_num4 ) + CHR( l_num3 ) + ;
           CHR( l_num2 ) + CHR( l_num1 )
RETURN l_chr
* END OF long_to_chr
**********************
