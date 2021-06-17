//
// Created by hannes on 15.06.21.
//

#include "IP4Header.h"

IP4Header::IP4Header(IP4Address *destinationAddress, IP4DataPart *dataPart) {
    this->destinationAddress = destinationAddress;
    protocolType = dataPart->getIP4ProtocolTypeAsInt();

    //we use minimal header if we create one
    totalLength = IP4HEADER_MIN_LENGTH + dataPart->getLengthInBytes();
}

IP4Header::~IP4Header() {
    delete sourceAddress;
    delete destinationAddress;
}

size_t IP4Header::getTotalDatagramLength() const {
    return (size_t) totalLength;
}

IP4DataPart::IP4ProtocolType IP4Header::getIP4ProtocolType() const {
    return IP4DataPart::parseIntAsIP4ProtocolType(protocolType);
}

IP4Address *IP4Header::getDestinationAddress() {
    return destinationAddress;
}

IP4Address *IP4Header::getSourceAddress() {
    return sourceAddress;
}

void IP4Header::setSourceAddress(IP4Address *address) {
    //Cleanup if already set
    delete this->sourceAddress;
    this->sourceAddress = address;
}

size_t IP4Header::getHeaderLength() const {
    //IP4 header length is not fixed size
    //-> calculate it from real header value for header length!
    return (size_t) (version_headerLength - 0x40) * 4;
}

uint8_t IP4Header::copyTo(Kernel::NetworkByteBlock *output) {
    uint8_t errors = 0;
    errors += output->append(version_headerLength);
    errors += output->append(typeOfService);
    errors += output->append(totalLength);
    errors += output->append(identification);
    errors += output->append(flags_fragmentOffset);
    errors += output->append(timeToLive);
    errors += output->append(protocolType);
    errors += output->append(headerChecksum);
    errors += output->append(sourceAddress, IP4ADDRESS_LENGTH);
    errors += output->append(destinationAddress, IP4ADDRESS_LENGTH);
    return errors;
}

uint8_t IP4Header::parse(Kernel::NetworkByteBlock *input) {
    if (input == nullptr || input->bytesRemaining() < IP4HEADER_MIN_LENGTH) {
        return 1;
    }

    if(sourceAddress!= nullptr || destinationAddress != nullptr){
        //Stop if already initialized!
        //-> no existing data is overwritten
        return 1;
    }

    uint8_t errors = 0;
    errors += input->read(&version_headerLength);
    errors += input->read(&typeOfService);
    errors += input->read(&totalLength);
    errors += input->read(&identification);
    errors += input->read(&flags_fragmentOffset);
    errors += input->read(&timeToLive);
    errors += input->read(&protocolType);
    errors += input->read(&headerChecksum);

    uint8_t addressBytes[IP4ADDRESS_LENGTH];
    errors += input->read(addressBytes, IP4ADDRESS_LENGTH);
    sourceAddress=new IP4Address(addressBytes);
    errors += input->read(addressBytes, IP4ADDRESS_LENGTH);
    destinationAddress=new IP4Address(addressBytes);

    //Skip additional bytes if incoming header is larger than our internal one
    //-> next layer would read our remaining header bytes as data otherwise!
    size_t remainingHeaderBytes = getHeaderLength() - IP4HEADER_MIN_LENGTH;
    //True if remainingHeaderBytes > 0
    if (remainingHeaderBytes) {
        errors += input->skip(remainingHeaderBytes);
    }

    return errors;
}

bool IP4Header::headerValid() {
    return calculateChecksum() == headerChecksum;
}

uint16_t IP4Header::calculateChecksum() const {
    //TODO: Implement this one!
    return headerChecksum;
}