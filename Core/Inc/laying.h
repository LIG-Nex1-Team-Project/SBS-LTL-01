/*
 * laying.h
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#ifndef LAYING_H
#define LAYING_H

#include "types.h"

/**
 * @brief A31 CSU: 구동 시간 계산 제어 [cite: 302, 622]
 * 목표 각도와 현재 각도의 차이를 분석하여 모터 이동에 필요한 이론적 시간을 계산함[cite: 630].
 */
uint32_t calculateDriveTime(void);

/**
 * @brief A32 CSU: 모터 구동 신호 제어 [cite: 306, 646]
 * 목표 각도를 PWM 듀티비(5~10%)로 환산하여 타이머 레지스터에 적용함[cite: 656].
 */
void updateServoPWM(void);

/**
 * @brief A33 CSU: 정렬 상태 감시 제어 [cite: 310, 660]
 * 경과 시간을 체크하여 시스템의 '정렬 완료' 상태를 논리적으로 판정함[cite: 669].
 */
void checkAlignmentStatus(void);

/**
 * @brief A34 CSU: 발사 및 안전 제어 [cite: 316, 670]
 * 최종 레이저 출력을 제어하며, 설정된 사격 유지 시간(1초) 초과 시 자동 소등함[cite: 680].
 */
void controlLaserLaunch(void);

#endif /* LAYING_H */
