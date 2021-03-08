
/*  @file libntpclient.c
    @author Julio Martins 
    @brief Arquivo Fonte para sincronização de data/hora a partir de um SNTP
    @version 0.1
    @date 2021-03-05

    Copyright: Todos os direitos reservados - Tefway - 2021
*/

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include "libntpclient.h"

using namespace std;



ClientSNTP :: ClientSNTP(){
    this->msg = new packet_t;
    this->datetime = new timeval;

    //host name list
    hostname = new string[5]
                        {"ntp.cais.rnp.br",
                        "a.st1.ntp.br",
                        "a.ntp.br",
                        "ut1-wwv.nist.gov", 
                        "ntps1-0.cs.tu-berlin.de"};

    
    *(this->msg) = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    memset( this->msg, 0, sizeof( packet_t ) );

    //Set only the first byte to 00|011|011 - Leap Indicator|Protocol Version|Mode
    this->msg->liVerMode = 0x1b;   // Represents 27 in base 10
}

ClientSNTP :: ~ClientSNTP(){
    delete this->msg;
    delete this->datetime;
    delete[] this->hostname;
}

void ClientSNTP :: setTimeFromSNTP(){
    
    if( settimeofday( this->datetime, nullptr ) < 0 )
        this->errorMsg("ERROR setting local time");
}

void ClientSNTP :: updateTime( void ){
    
    this->getTimeFromSNTP();
    this->setTimeFromSNTP();
}

void ClientSNTP :: getTimeFromSNTP( void ){

    int i, hn_len, tmout, sockfd;
    
    hn_len = sizeof( hostname )/sizeof(char*); //hostname list length
    tmout = 5000;        //wait 5.0 sec and try next server

    for(  i = 0; i < hn_len; i++ ){
        cout << hostname[i] << "\n";
        if ( this->connectToSNTP( this->msg, hostname[i].c_str(), &sockfd, tmout ) )
            break;
    }
    
    if( i >= hn_len )
        this->errorMsg( "ERROR connecting to SNTP" );

    // Send it the NTP packet it wants.
    if ( write( sockfd, ( char* ) this->msg, sizeof( packet_t ) ) < 0 )
        this->errorMsg( "ERROR writing to socket" );

    // Wait and receive the packet back from the server.
    if ( read( sockfd, ( char* ) this->msg, sizeof( packet_t ) ) < 0 )
        this->errorMsg( "ERROR reading from socket" );

    // ntohl() converts the bit/byte order from the network's to host's "endianness".
    this->msg->transTimeSec = ntohl( this->msg->transTimeSec ); // Time-stamp seconds.
    this->msg->transTimeFrac = ntohl( this->msg->transTimeFrac ); // Time-stamp fraction of a second.

    // Extract the time-stamp seconds from when the packet left the server.
    // Subtract 70 years (in seconds) passed since NTP epoch (1900).
    this->datetime->tv_sec = ( time_t ) ( this->msg->transTimeSec - NTP_TIMESTAMP_DELTA );
    this->datetime->tv_usec = 0;

}

bool ClientSNTP :: connectToSNTP( packet_t *packet, const char* hostname, int* sockfd, int tmout ){
   
    int sockflg; // Socket file descriptor and the n return result from writing/reading from the socket.
    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.
    fd_set waitset;
    timeval timeout;

    *sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.
    
    if ( *sockfd < 0 )
        this->errorMsg( "ERROR opening socket" );

    server = gethostbyname( hostname ); // Convert URL to IP.

    if ( server == nullptr )
        this->errorMsg( "ERROR, no such host" );

    // Zero out the server address structure.
    bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

    serv_addr.sin_family = AF_INET; //IPv4

    // Copy the server's IP address to the server address structure.
    bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

    // Convert the port number integer to network big-endian style and save it to the server address structure.
    serv_addr.sin_port = htons( NTP_PORT );

    //Dealling with timeout
    sockflg = fcntl ( (*sockfd), F_GETFL, NULL);
    if( sockflg < 0 )
        this->errorMsg("ERROR getting socket flags");

    //Non-blocking socket
    if ( fcntl ( (*sockfd), F_SETFL, sockflg | O_NONBLOCK) < 0 )
        this->errorMsg("ERROR setting non-blocking socket");

    timeout.tv_sec = tmout;
    timeout.tv_usec = 0;

    // Call up the server using its IP address and port number.
    if ( connect( (*sockfd), ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 ){
        if( errno == EINPROGRESS ){
            FD_ZERO(&waitset);
            FD_SET( (*sockfd), &waitset );
            
            errno = 0;

            //if timeout happened
            if( select( (*sockfd)+1, nullptr, &waitset, nullptr, &timeout ) == 0 ){
                errno = ETIMEDOUT;
                return false;
            }else if( errno != 0 )
                this->errorMsg( "ERROR during select");
        }else
            this->errorMsg( "ERROR connecting" );
    }

    if (fcntl ( (*sockfd), F_SETFL, sockflg ) < 0)
        this->errorMsg("ERROR resetting socket flags");

    return true;
}


void ClientSNTP :: errorMsg( const char* str ){
    perror( str );
    exit( EXIT_FAILURE );
}

int main(){
    ClientSNTP client;

    client.updateTime();

    return 0;
}