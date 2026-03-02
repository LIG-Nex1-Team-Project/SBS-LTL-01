/*
 * state.h
 *
 *  Created on: Mar 2, 2026
 *      Author: User
 */

#ifndef STATE_H
#define STATE_H

#include "types.h"

/**
 * @brief A20 CSU: 최상위 상태 제어 프로세스 [cite: 498-499]
 * 현재 시스템 상태(Init, Standby, Align, Launch, Error)에 맞는 함수를 실행함[cite: 506].
 */
void executeStateProcess(void);

/**
 * @brief A21 CSU: 초기화 제어 [cite: 276, 512]
 * 전원 인가 후 변수 초기화 및 통신/모터/레이저 인터페이스를 준비함[cite: 520].
 */
void executeInitProcess(void);

/**
 * @brief A22 CSU: 신호 대기 제어 [cite: 279, 521]
 * 정상 준비 상태에서 ECS의 목표 각도 및 사격 승인 신호를 수신 대기함[cite: 534].
 */
void executeStanbyProcess(void);

/**
 * @brief A23 CSU: 정렬 수행 제어 [cite: 282, 537]
 * 목표 각도 수신 시 모터 구동을 시작하고 정렬 완료 여부를 감시함[cite: 547].
 */
void executeAlignProcess(void);

/**
 * @brief A24 CSU: 사격 통제 제어 [cite: 286, 561]
 * 정렬 완료 후 사격 승인 시 레이저를 점등하고 안전 절차를 관리함[cite: 570].
 */
void executeLaunchProcess(void);

/**
 * @brief A25 CSU: 오류 및 안전 제어 [cite: 290, 571]
 * 예외 상황 발생 시 하드웨어 출력을 즉시 차단(Failsafe)하고 오류 코드를 송신함[cite: 583].
 */
void executeErrorProcess(void);

#endif /* STATE_H */
