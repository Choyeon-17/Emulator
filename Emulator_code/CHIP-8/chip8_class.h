#include <iostream>
#include <fstream>
#include <stack>

/**
 * @author Choyeon-17
 * @date 2019-11-08-Fri
 * @brief
 * CHIP_8 클래스는 CHIP-8 시스템을 구현합니다.
 * @details
 * opcode는 CPU 명령어입니다.
 * I는 사용자가 지정해준 인덱스 레지스터입니다.
 * pc는 0x000부터 0xFFF까지의 값을 가질 수 있는 프로그램 카운터입니다.
 * CHIP-8 시스템에는 최대 60Hz씩 세는 delay_timer, sound_timer라는 두 개의
 * 타이머 레지스터가 있습니다. 0 이상으로 설정되면, 0까지 카운트 다운할
 * 것입니다. sound_timer가 0에 도달할 때마다 시스템의 버저가 울립니다.
 * memory[4096]는 CHIP-8 시스템의 정보를 저장하는 메모리입니다.
 * CHIP-8 시스템의 메모리 맵
 * 0x000-0x1FF - CHIP-8 인터프리터 (CHIP-8의 글꼴 세트를 포함합니다.)
 * 0x050-0x0A0 - 4x5 픽셀 글꼴 세트를 저장하는데 사용됩니다.
 * 0x200-0xFFF - 프로그램 ROM 및 작업 RAM
 * V[16]는 레지스터 배열입니다.
 * gfx[8 * 32]는 총 (uint8_t = 1byte = 8 bits) x 8 x 32개의 픽셀의 상태(0 or
 * 1)를 저장합니다. stack[16]은 프로그램이 특정 주소로 점프하거나 서브 루틴을
 * 호출하기 전에 프로그램 카운터를 저장합니다. CHIP-8은 16진수 기반
 * 키패드(0x0-0xF)가 있고, key[16]은 키의 현재 상태를 저장합니다. fontset[80]은
 * 화면에 출력할 문자의 정보를 담습니다.
 */

class CHIP_8 {
private:
    uint16_t opcode;
	uint16_t I;
	uint16_t pc;
	uint8_t delay_timer;
	uint8_t sound_timer;

	uint8_t memory[4096];
	uint8_t V[16];
	uint8_t gfx[8][32];
	stack<uint16_t> stack[16];
	uint8_t key[16];

	constexpr uint8_t fontset[80] = {
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

public:
    bool drag_flag;

    CHIP_8();
    uint16_t Fetch_opcode();
    void Decode_opcode(uint16_t opcode);
    void Update_timers(uint8_t delay_timer, uint8_t sound_timer);
    void emulate_cycle();
}