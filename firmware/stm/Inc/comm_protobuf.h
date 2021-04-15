/*
 * comm_protobuf.h
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#ifndef COMM_PROTOBUF_H_
#define COMM_PROTOBUF_H_

#include <cstdint>

void OnMainLoop_Protobuf();
void OnUSBReceive_Protobuf(uint8_t* Buf, uint32_t *Len);

#endif /* COMM_PROTOBUF_H_ */
