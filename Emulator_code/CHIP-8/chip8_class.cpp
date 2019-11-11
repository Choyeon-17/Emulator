#include <fstream>
#include <iostream>
#include <stack>
#include <vector>

using namespace std;

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

class CHIP_8
{
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
	uint8_t fontset[80] = {
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
	/**
   * @brief
   * CHIP_8(): CHIP_8의 멤버를 초기화, memory[0-79]에 글꼴 세트를 로드, game.txt
   * 파일을 memory[0x200-]부터 로드
   */
	CHIP_8() : opcode(0), I(0), pc(0x200), delay_timer(0), sound_timer(0) {
		memset(memory, 0, 4096);
		memset(V, 0, 16);
		memset(gfx, 0, 256);
		memset(stack, 0, 16);
		memset(key, 0, 16);

		for (int i = 0; i < 80; i++)
			memory[i] = fontset[i];

		/**
     * @brief
     * line 91-92: 파일 읽기 준비
     * line 95: 위치 지정자를 파일 끝으로 옮깁니다.
     * line 96: 그 위치를 읽습니다.(파일의 크기)
     * line 97: 그 크기의 문자열을 할당합니다.
     * line 98: 위치 지정자를 다시 파일 맨 앞으로 옮깁니다.
     * line 99: 파일 전체 내용을 읽어서 문자열에 저장합니다.
     * line 100: memory[0x200-]부터 파일 전체 내용을 복사합니다.
     */
		ifstream game("game.txt");
		string buffer;

		if (game.is_open()) {
			game.seekg(0, ios::end);
			int size = game.tellg();
			buffer.resize(size);
			game.seekg(0, ios::beg);
			game.read(&buffer[0], size);
			memcpy(memory[0x200], buffer, size);
		}
	}

	/**
   * @brief
   * pc가 memory로부터 다음에 실행할 명령어를 인출합니다.
   * @return uint16_t : opcode
   */
	uint16_t Fetch_opcode() {
		return memory[pc] << 8 | memory[pc + 1];
	}

	uint8_t key();

	uint8_t get_delay();

	uint8_t get_key();

	/**
   * @brief
   * 명령어를 해독 및 실행합니다.
   * @return void형 함수이므로 반환 값을 갖지 않습니다.
   * @param opcode
   */
	void Decode_opcode(uint16_t opcode);

	/**
   * @brief
   * 타이머를 갱신합니다.
   * @param delay_timer
   * @param sound_timer
   */
	void Update_timers(uint8_t delay_timer, uint8_t sound_timer) {
		if (delay_timer > 0)
			delay_timer--;

		if (sound_timer > 0) {
			if (sound_timer == 1)
				cout << "BEEP!" << endl;
			sound_timer--;
		}
	}

	/**
   * @brief
   * 명령어 인출, 명령어 해독 및 실행, 타이머 갱신을 한 번 실행합니다.
   */
	void emulate_cycle() {
		opcode = Fetch_opcode();

		Decode_opcode(opcode);

		Update_timers(delay_timer, sound_timer);
	}
};

CHIP_8::Decode_opcode(uint16_t opcode)
{
	int X = (opcode & 0x0F00) >> 8;
	int Y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000) {
		case 0x0000: {
			switch (opcode & 0x00FF) {
			case 0x00E0:
				memset(gfx, 0, 8 * 32);
				pc += 2;
				break;
			case 0x00EE:
				pc = stack.pop();
				pc += 2;
				break;
			}
		}
		case 0x1000:
			pc = opcode & 0x0FFF;
			break;
		case 0x2000:
			stack.push(pc);
			pc = opcode & 0x0FFF;
			break;
		case 0x3000: {
			if (V[X] == opcode & 0x00FF)
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x4000: {
			if (V[X] != opcode & 0x00FF)
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x5000: {
			if (V[X] == V[Y])
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x6000:
			V[X] = opcode & 0x00FF;
			pc += 2;
			break;
		case 0x7000:
			V[X] += opcode & 0x00FF;
			pc += 2;
			break;
		case 0x8000: {
			switch (opcode & 0x000F) {
				case 0x0000:
					V[X] = V[Y];
					break;
				case 0x0001:
					V[X] = V[X] | V[Y];
					break;
				case 0x0002:
					V[X] = V[X] & V[Y];
					break;
				case 0x0003:
					V[X] = V[X] ^ V[Y];
					break;
				case 0x0004: {
					V[X] += V[Y];
					if (V[X] + V[Y] > 0xF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					break;
				}
				case 0x0005: {
					V[X] -= V[Y];
					if (V[X] < V[Y])
						V[0xF] = 0;
					else
						V[0xF] = 1;
				}
				case 0x0006:
					V[0xF] = (V[X] << 7) >> 7;
					V[X] >>= 1;
					break;
				case 0x0007: {
					V[X] = V[Y] - V[X];
					if (V[X] > V[Y])
						V[0xF] = 0;
					else
						V[0xF] = 1;
				}
				case 0x000E: {
					V[0xF] = V[X] >> 7;
					V[X] <<= 1;
				}
			}
			pc += 2;
			break;
		}
		case 0x9000: {
			if (V[X] != V[Y])
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0xA000:
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		case 0xB000:
			pc = V[0] + opcode & 0x0FFF;
			break;
		case 0xC000:
			V[X] = rand() & (opcode & 0x00FF);
			pc += 2;
			break;
		case 0xD000: {
			unsigned short height = opcode & 0x000F;
    		unsigned short pixel;
    		V[0xF] = 0;

    		for (int yline = 0; yline < height; yline++) {
        		pixel = memory[I + yline];
        		
				for (int xline = 0; xline < 8; xline++) {
            		if ((pixel & (0x80 >> xline)) != 0) {
                		if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                    		V[0xF] = 1;
                	gfx[x + xline + ((y + yline) * 64)] ^= 1;
            		}
        		}
    		}
		}
		case 0xE000: {
			switch (opcode & 0x00FF) {
				case 0x009E: {
					if (key() == V[X])
						pc += 4;
					else
						pc += 2;
					break;
				}
				case 0x00A1: {
					if (key() != V[X])
						pc += 4;
					else
						pc += 2;
					break;
				}
			}
		}
		case 0xF000: {
			switch (opcode & 0x00FF) {
				case 0x0007:
					V[X] = get_delay();
					break;
				case 0x000A:
					V[X] = get_key();
					break;
				case 0x0015:
					delay_timer = V[X];
					break;
				case 0x0018:
					sound_timer = V[X];
					break;
				case 0x001E: {
					I += V[X];
					if (I + V[X] > 0xFFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					break;
				}
				case 0x0029:
					break;
				case 0x0033:
					break;
				case 0x0055:
					break;
				case 0x0065:
					break;
			}
			pc += 2;
		}
	}
}