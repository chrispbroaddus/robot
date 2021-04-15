/*
 * comm_ascii.h
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#ifndef COMM_ASCII_H_
#define COMM_ASCII_H_

#include <cstdint>

void OnMainLoop_ASCII();
void OnUSBReceive_ASCII(uint8_t* Buf, uint32_t *Len);

#endif /* COMM_ASCII_H_ */
