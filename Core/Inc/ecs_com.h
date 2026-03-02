/*
 * ecs_com.h
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#ifndef ECS_COM_H
#define ECS_COM_H

#include "types.h"

/**
 * @brief A11 CSU: 외부 체계(ECS) 데이터 수신 [cite: 265, 427]
 * CAN FIFO에서 메시지를 읽어와 식별자를 확인하고 수신 처리함 [cite: 692-693].
 */
void receiveDataFromECS(void);

/**
 * @brief 데이터 변환 및 구조체 갱신 [cite: 426, 435]
 * 수신된 CAN 바이트 데이터를 float 및 uint8_t 형식으로 변환하여 전역 구조체에 저장함[cite: 701].
 */
void convertDataToLTLStruct(void);

/**
 * @brief A12 CSU: 외부 체계(ECS) 데이터 송신 [cite: 270, 439]
 * 현재 발사대의 상태(정렬 완료, 사격 중 등)를 CAN Frame으로 변환하여 ECS로 전송함 [cite: 694-696].
 */
void sendStateToECS(void);

#endif /* ECS_COM_H */
