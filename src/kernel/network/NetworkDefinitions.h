//
// Created by hannes on 16.06.21.
//

#ifndef HHUOS_NETWORKDEFINITIONS_H
#define HHUOS_NETWORKDEFINITIONS_H

#define MAC_SIZE 6
#define IP4ADDRESS_LENGTH 4

#define ECHO_PORT_NUMBER 7 //RFC 862

//TODO: Verify values with RFC Sources
#define UDP4DATAPART_MAX_LENGTH 1500
#define UDP4HEADER_MAX_LENGTH 0xffff

#define IP4DATAPART_MAX_LENGTH 576 //Recommended in RFC 791 page 13
#define IP4HEADER_MAX_LENGTH 60 //RFC 791 page 13

#define ETHERNETDATAPART_MAX_LENGTH 1500
#define ETHERNETHEADER_MAX_LENGTH 14

#endif //HHUOS_NETWORKDEFINITIONS_H
