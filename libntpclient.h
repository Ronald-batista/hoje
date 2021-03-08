
/*  @file libntpclient.h
    @author Julio Martins 
    @brief Biblioteca para sincronização de data/hora a partir de um SNTP
    @version 0.1
    @date 2021-03-05

    Copyright: Todos os direitos reservados - Tefway - 2021
*/

#include <sys/time.h>
#include <string>


#define NTP_PORT 123

/*  
    @brief NTP_TIMESTAMP_DELTA This leaves the seconds since the NTP epoch(1900) to UNIX epoch(1970) 
*/
#define NTP_TIMESTAMP_DELTA 2208988800ull


/* 
* @author Julio Martins
* @brief packet_t - define o pacote de 48 bytes para requisição de data/hora sobre o protocolo NTP
* 
*/
typedef struct {
    char liVerMode;             //Leap Indicator(2-bits)/Version of protocol(3-bits)/Mode(3-bits) (for client pick value 3)
    char stratum;               // 8 bits-> Stratum level of the local clock.
    char poll;                  // 8 bits-> Maximum interval between successive messages.
    char precision;             // 8 bits-> Precision of the local clock.
    
    unsigned int rtDelay;        // 32 bits -> Total round trip delay time.
    unsigned int rtDispersion;   // 32 bits -> Max error aloud from primary clock source.
    unsigned int refId;          // 32 bits -> Reference clock identifier.

    unsigned int refTimeSec;        // 32 bits -> Reference time-stamp seconds.
    unsigned int refTimeFrac;        // 32 bits -> Reference time-stamp fraction of a second.

    unsigned int origTimeSec;       // 32 bits -> Originate time-stamp seconds.
    unsigned int origTimFrac;       // 32 bits -> Originate time-stamp fraction of a second.

    unsigned int recTimeSec;         // 32 bits -> Received time-stamp seconds.
    unsigned int recTimeFrac;         // 32 bits -> Received time-stamp fraction of a second.

    unsigned int transTimeSec;         // 32 bits -> Transmit time-stamp seconds passed since NTP epoch.
    unsigned int transTimeFrac;         // 32 bits -> Transmit time-stamp fraction of a second.    

} packet_t;

using namespace std;

/*  
* @brief Define um objeto ClienteSNTP para operar com o NTP.
*/
class ClientSNTP{

    private:        
/*  
* @brief msg - o pacote enviado para o SNTP para requisição da data/hora atual
*        datetime - o valor da data/hora atual
         hostname - NTP server hostname list
*/
        packet_t *msg;
        timeval *datetime;
        std::string* hostname;

/* 
* @author Julio Martins
* @brief imprime mensagem de erro no stderr
* @param str - a mensagem
*/       
        void errorMsg( const char* str );
/*
* @author Julio Martins  
* @brief modifica a data/hora do sistema
*/       
        void setTimeFromSNTP( void );

/*
* @author Julio Martins  
* @brief consulta um SNTP e retorna a data/hora corrente
*/
        void getTimeFromSNTP( void );

        bool connectToSNTP( packet_t *packet, const char* host, int* sockfd, int timeout );
    
    public:
        ClientSNTP();
        ~ClientSNTP();
/*
* @author Julio Martins  
* @brief processa a atualização da data/hora corrente
*/
        void updateTime(void);
};
