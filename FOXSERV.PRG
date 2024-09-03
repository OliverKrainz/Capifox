**********************
* CAPI-Server Beispielprogramm
* (C) Oliver Krainz 1996-2024
*

* Konstanten 
*
#define c_res_connect    "c"
#define c_res_disconnect "d"
#define c_res_download   "l"
#define c_isdn_okay       "o"
#define c_res_status     "s"
#define c_res_error      "e"

#define c_isdn_ende       "E"
#define c_isdn_command    "C"

REGIONAL l_antwort, l_cmd, l_pos, l_datei, l_long, l_groesse

  * Einstellungen festlegen.
  *
  DO set_settings
  
  * Initialisieren.
  *
  DO initialisieren
  
  ? "Initialisiere CAPI-Handler."

  * Externen Capiserver starten.
  *
  RUN /N capiserv.exe

  * Auf Bestätigung des Capiservers warten.
  *
  l_antwort = ""

  l_antwort = warte_auf_antwort()   && PL>

  ? "Antwort:"
  ? l_antwort

  IF ( l_antwort != c_isdn_okay )
    ? "Fehler beim Initialisieren"
    
    * Capiserver beenden.
    *
    DO write_cmd_capiserver WITH c_isdn_ende, c_isdn_command
    RETURN
  ENDIF

  * Jetzt auf Connect warten.
  *
  l_antwort = warte_auf_antwort()   && PL>

  ? "Antwort:"
  ? l_antwort

  IF ( l_antwort != c_res_connect )
    ? "Fehler beim Warten auf Connect."
    
    * Capiserver beenden.
    *
    DO write_cmd_capiserver WITH c_isdn_ende, c_isdn_command
    RETURN
  ENDIF

  l_antwort = ""

  * Solange auf Befehle des Clients antworten
  * bis das Disconnect erfolgt.
  *
  DO WHILE ( l_antwort != c_res_disconnect )

    l_antwort = warte_auf_antwort()
  
    ? "Antwort:"
    ? l_antwort
  
    * Anwort analysieren.
    *
    l_cmd = LEFT( l_antwort, 1 ) 
    
    DO CASE
    
      * Anfrage auf Download?
      *
      CASE l_cmd == c_res_download
      
        * Angeforderte Datei extrahieren.
        *
        l_datei = ""
        l_pos = 2
        
        DO WHILE ( SUBSTR( l_antwort, l_pos, 1 ) != CHR(0) )
          l_pos = l_pos + 1
        ENDDO
        
        l_datei = SUBSTR( l_antwort, 2, l_pos - 2 )
        
        ? "Angeforderte Datei:"
        ? l_datei
        
        * Hier könnten jetzt zum Beispiel ZugangsChecks
        * oder ähnliches durchgeführt werden.
        *
        * Wird der Download nicht gestattet, muss als
        * Dateigröße 0 übergeben werden. Der CapiServer
        * schickt dann eine entsprechende Antwort.
        
        * Wir geben an den Server die Kennung für
        * download zurück mit dem vorher übergebenen Dateinamen.
        * Dieser könnte auch geändert werden, z.B. wenn
        * der Server für den Dateitransfer eine Datei aus einem
        * anderen Verzeichnis nehmen soll.
        * Hinzu kommt die Größe der Datei als long-Wert.
        *
        = ADIR( a_tmp, l_datei )
        l_groesse = long_to_chr( a_tmp(2) )
        RELEASE a_tmp
        
        DO write_cmd_capiserver WITH c_res_download + l_datei + ;
                                     CHR(0) + l_groesse, c_isdn_command
        
      * Statusbericht?
      *
      CASE l_cmd == c_res_status
      
        * Downloadstatus?
        *
        IF ( SUBSTR( l_antwort, 2, 1 ) = c_res_download )
        
          * Dateitransferfortschritt steht als long-Zahl
          * an den Stellen 3-6.
          * Achtung! Die Zahl muss erst in FoxPro von
          * der long-Darstellung umgerechnet werden!
          *
          l_long = chr_to_long( SUBSTR( l_antwort, 3, 4 ) )
          
          ? "Transferierte Bytes: "
          ? l_long
        ENDIF
          
      * Disconnect?
      *
      CASE l_cmd == c_res_disconnect
      
        ? "Habe Disconnect erhalten, ich mache Schluss."
        
    ENDCASE

  ENDDO

  * Externe Exe beenden.
  *
  DO write_cmd_capiserver WITH c_isdn_ende, c_isdn_command

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
    l_filehandle = FOPEN("capiserv.inf")
    
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
        l_filehandle = FCREATE("capiserv.inf")
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
* Ein Command an den CapiServer
* geben.
*
PROCEDURE write_cmd_capiserver 
PARAMETER par_command, par_kennung
  
  * Kennung für CapiServer ergänzen.
  *
  IF EMPTY( par_kennung )
    par_command = "S" + par_command + CHR(0)
  ELSE
    par_command = par_kennung + par_command + CHR(0)
  ENDIF
  
  l_endwait = .F.

  l_try = 1
    
  l_antwort = ""
  
  ? "Schreibe an S: " + par_command 
  
  DO WHILE (!l_endwait)

    * Versuchen Datei zu öffnen.
    *  
    l_filehandle = FCREATE("capiserv.inf")
    
    * öffnen erfolgreich?
    *
    IF ( l_filehandle != -1 )
    
      * Befehl schreiben.
      *
      = FWRITE(l_filehandle, par_command)
      = FCLOSE(l_filehandle)
      
      l_endwait = .T.
      
    ELSE
    
      * Ansonsten etwas warten, nach
      * 5 Versuchen ist aber Schluss.
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
* END OF write_cmd_capiserver
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
  DEFINE WINDOW capiserv AT 0,0 SIZE 30,80 FONT "fixedsys", 9 ;
         SYSTEM ;
         TITLE "Capiserver"

  MOVE WINDOW capiserv CENTER       
  ACTIVATE WINDOW capiserv   
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
