#pragma once
#include <iostream>
#include <fstream>
#include <stack>

using namespace std;

/**
 * @author Choyeon-17
 * @date 2019-11-08-Fri
 * @brief
 * Chip8 클래스는 CHIP-8 시스템을 구현합니다.
 * @details
 * opcode는 CPU 명령어입니다.
 * I는 사용자가 지정해준 인덱스 레지스터입니다.
 * pc는 0x000부터 0xFFF까지의 값을 가질 수 있는 프로그램 카운터입니다.
 * CHIP-8 시스템에는 최대 60Hz씩 세는 delay_timer, sound_timer라는 두 개의
 * 타이머 레지스터가 있습니다. 0 이상으로 설정되면, 0까지 카운트 다운할
 * 것입니다. sound_timer는 0에 도달하기 전까지 매 사이클마다 시스템의 버저를 울립니다.
 * key는 현재 입력받은 키의 값을 저장합니다.
 * memory[4096]는 CHIP-8 시스템의 정보를 저장하는 메모리입니다.
 * CHIP-8 시스템의 메모리 맵
 * 0x000-0x1FF - CHIP-8 인터프리터 (CHIP-8의 글꼴 세트를 포함합니다.)
 * 0x050-0x0A0 - 4x5 픽셀 글꼴 세트를 저장하는데 사용됩니다.
 * 0x200-0xFFF - 프로그램 ROM 및 작업 RAM
 * reg[16]는 레지스터 배열입니다.
 * gfx[8 * 32]는 총 (uint8_t = 1byte = 8 bits) x 8 x 32개의 픽셀의 상태(0 or
 * 1)를 저장합니다.
 * stack[16]은 프로그램이 특정 주소로 점프하거나 서브 루틴을 호출하기 전에 프로그램 카운터를 저장합니다.
 * fontset[80]은 하나의 크기가 20 bits인 글꼴 16개가 담긴 상수 배열입니다.
 * private 영역에 Chip8() 디폴트 생성자를 선언함으로써 디폴트 생성자가 호출될 위험을 아예 없앱니다.
 */

class Chip8 {
private:
	uint16_t opcode;
	uint16_t I;
	uint16_t pc;
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint8_t key;

	uint8_t memory[4096];
	uint8_t reg[16];
	uint8_t gfx[8][32];
	stack<uint16_t> stack[16];

	const uint8_t fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	Chip8() { }

public:
	bool draw_flag;

	Chip8(char* file_path);
	void Fetch();
	void Decode();
	void Delay_and_sound_timers();
	void Emulate_cycle();
};