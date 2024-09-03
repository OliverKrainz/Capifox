// In den CAPI-Funktionen benutzte
// Konstanten.
//
#define ERROR 0
#define SUCCESS 1
#define NO_ERROR 0


// Makros
//
#define schreibe_BYTE(pointer, wert)  (*(pointer) = (wert))
#define schreibe_WORD(pointer, wert)  (*(WORD FAR *)(pointer) = (WORD)(wert))
#define schreibe_DWORD(pointer, wert) (*(DWORD FAR *)(pointer) = (DWORD)(wert))
#define hole_BYTE(pointer)            ((BYTE) *(pointer))
#define hole_WORD(pointer)            (*(WORD FAR *)(pointer))
#define hole_DWORD(pointer)           (*(DWORD FAR *)(pointer))
#define combi_cmd_subcmd(cmd, subcmd) ((WORD) cmd << 8 | (WORD) subcmd); 


// Längen
//
#define c_size_app_buffer             20000
#define c_capi_msg_len                180
#define c_laenge_listen_request	      17
#define c_laenge_listenb3_request     10
#define c_laenge_connect_response     11
#define c_laenge_connect_active_rsp   10
#define c_laenge_connectb3_response   12
#define c_laenge_connect_b3_active_rsp 10
#define c_laenge_disconnect_b3_rsp    10
#define c_laenge_disconnect_response  10
#define c_laenge_connect_request      18
#define c_laenge_connect_b3_request   11
#define c_laenge_disconnect_request   11
#define c_laenge_disconnect_b3_req    11
#define c_laenge_data_request         19
#define c_laenge_data_response        11
#define c_laenge_select_b2_protocol   18
#define c_laenge_select_b3_protocol   12

// Message Commands
//
#define c_listen_command              0x05
#define c_listenb3_command            0x81
#define c_connect_command             0x02
#define c_connect_active_command      0x03
#define c_connectb3_command           0x82
#define c_connect_b3_active_command   0x83
#define c_disconnect_b3_command       0x84
#define c_disconnect_command          0x04
#define c_data_command                0x86
#define c_select_b2_protocol          0x40
#define c_select_b3_protocol          0x80

// Message Subcommands                                                 
//
#define c_request                     0x00
#define c_confirmation                0x01
#define c_indication                  0x02
#define c_response                    0x03

// Ergebnisse
//
#define c_keine_nachricht             0x1006
#define c_listen_confirmation         0x0501
#define c_connect_indication          0x0202
#define c_listen_b3_confirmation      0x8101
#define c_connect_active_indication   0x0302
#define c_connect_b3_indication       0x8202
#define c_connect_b3_active_indication 0x8302
#define c_disconnect_b3_indication    0x8402
#define c_disconnect_indication       0x0402
#define c_connect_confirmation        0x0201
#define c_connect_b3_confirmation     0x8201
#define c_disconnect_b3_confirmation  0x8401
#define c_disconnect_confirmation     0x0401
#define c_data_b3_confirmation        0x8601
#define c_data_b3_indication          0x8602
#define c_select_b2_prot_confirmation 0x4001
#define c_select_b3_prot_confirmation 0x8001