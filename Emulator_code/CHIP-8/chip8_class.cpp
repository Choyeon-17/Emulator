#include "chip8_opcode_class.h"

/**
* @brief
* CHIP_8(): CHIP_8의 멤버를 초기화, memory[0-79]에 글꼴 세트를 로드,
* game.txt 파일을 memory[0x200-]부터 로드
*/
CHIP_8::CHIP_8() : opcode(0), I(0), pc(0x200), delay_timer(0), sound_timer(0) {
	memset(memory, 0, 4096);
	memset(V, 0, 16);
	memset(gfx, 0, 256);
	memset(stack, 0, 16);
	memset(key, 0, 16);

	for (int i = 0; i < 80; i++)
		memory[i] = fontset[i];

	ifstream rom("game.txt");
	string buffer;

	if (rom.is_open()) {
		rom.seekg(0, ios::end);
		int size = rom.tellg();
		buffer.resize(size);
		rom.seekg(0, ios::beg);
		rom.read(&buffer[0], size);
		memcpy(memory[0x200], buffer, size);
	}
}

/**
* @brief
* pc가 memory로부터 다음에 실행할 명령어를 인출합니다.
* @return uint16_t : opcode
*/
CHIP_8::Fetch_opcode() {
	return memory[pc] << 8 | memory[pc + 1];
}

/**
* @brief
* 타이머를 갱신합니다.
* @param delay_timer
* @param sound_timer
*/
CHIP_8::Update_timers(uint8_t delay_timer, uint8_t sound_timer) {
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
CHIP_8::emulate_cycle() {
	opcode = Fetch_opcode();

	Decode_opcode(opcode);

	Update_timers(delay_timer, sound_timer);
	}
};

/**
* @brief
* 명령어를 해독 및 실행합니다.
* @param opcode
*/
CHIP_8::Decode_opcode(uint16_t opcode) {
	draw_flag = false;
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
			pc += 2;
			if (V[X] == opcode & 0x00FF)
				pc += 2;
			break;
		}
		case 0x4000: {
			pc += 2;
			if (V[X] != opcode & 0x00FF)
				pc += 2;
			break;
		}
		case 0x5000: {
			pc += 2;
			if (V[X] == V[Y])
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
			pc += 2;
			if (V[X] != V[Y])
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
			uint16_t height = opcode & 0x000F;
			uint8_t pixels;
			V[0xF] = 0;

			for (int yline = 0; yline < height; yline++) {
				pixels = memory[I + yline];
				gfx[V[Y] + yline][V[X]] ^= pixels;
			}

			I += 5;
			draw_flag = true;
			pc += 2;
			break;
		}
		case 0xE000: {
			switch (opcode & 0x00FF) {
				case 0x009E: {
					pc += 2;
					if (key[V[X]] == 0)
						pc += 2;
					break;
				}
				case 0x00A1: {
					pc += 2;
					if (key[V[X]] != 0)
						pc += 2;
					break;
				}
			}
		}
		case 0xF000: {
			switch (opcode & 0x00FF) {
				case 0x0007:
					V[X] = delay_timer;
					break;
				case 0x000A: 
					V[X] = pressed_key();
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
					V[X] = I;
					break;
				case 0x0033:
					memory[I] = V[X] / 100;
					memory[I + 1] = (V[X] / 10) % 10;
					memory[I + 2] = V[X] % 10;
					break;
				case 0x0055:
					for (int i = 0; i <= X; i++)
						memory[I++] = V[i];
					I--;
					break;
					break;
				case 0x0065:
					for (int i = 0; i <= X; i++)
						V[i] = memory[I++];
					I--;
					break;
			}
			pc += 2;
		}
	}
}