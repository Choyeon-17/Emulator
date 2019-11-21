#include "chip8_opcode_class.h"

/**
* @brief
* Chip8(): Chip8의 멤버를 초기화, memory[0-79]에 글꼴 세트를 로드합니다.
*/
Chip8::Chip8(char* file_path) : opcode(0), I(0), pc(0x200), delay_timer(0), sound_timer(0), key(0) {
	memset(memory, 0, sizeof(memory));
	memset(reg, 0, sizeof(reg));
	memset(gfx, 0, sizeof(gfx));
	memset(stack, 0, sizeof(stack));
	memcpy(memory, fontset, sizeof(fontset));

	ifstream rom(file_path);

	if (rom.is_open()) {
		rom.seekg(0, ios::end);
		int size = rom.tellg();
		rom.seekg(0, ios::beg);
		rom.read(&memory[0x200], size);
	}
	else
		cout << "Failed to read file!" << endl;
}

/**
* @brief
* pc가 memory로부터 다음에 실행할 명령어를 인출합니다.
*/
void Chip8::Fetch() {
	opcode = memory[pc] << 8 | memory[pc + 1];
}

/**
* @brief
* 타이머를 갱신합니다.
*/
void Chip8::Delay_and_sound_timers() {
	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0) {
		cout << "BEEP!" << endl;
		sound_timer--;
	}
}

/**
* @brief
* 명령어 인출, 명령어 해독 및 실행, 타이머 갱신을 한 번 실행합니다.
*/
void Chip8::Emulate_cycle() {
	Fetch();

	Decode();

	Delay_and_sound_timers();
}

/**
* @brief
* 명령어를 해독 및 실행합니다.
*/
void Chip8::Decode() {
	draw_flag = false;
	int x = (opcode & 0x0F00) >> 8;
	int y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000) {

	case 0x0000: {
		switch (opcode & 0x00FF)
	case 0x00E0:
		memset(gfx, 0, 8 * 32);
		pc += 2;
		break;
	case 0x00EE:
		pc = stack->top();
		stack->pop();
		pc += 2;
		break;
	}

	case 0x1000:
		pc = opcode & 0x0FFF;
		break;

	case 0x2000:
		stack->push(pc);
		pc = opcode & 0x0FFF;
		break;

	case 0x3000:
		pc += 2;
		if (reg[x] == opcode & 0x00FF)
			pc += 2;
		break;

	case 0x4000:
		pc += 2;
		if (reg[x] != opcode & 0x00FF)
			pc += 2;
		break;

	case 0x5000:
		pc += 2;
		if (reg[x] == reg[y])
			pc += 2;
		break;

	case 0x6000:
		reg[x] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000:
		reg[x] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000: {
		switch (opcode & 0x000F) {

		case 0x0000:
			reg[x] = reg[y];
			break;

		case 0x0001:
			reg[x] = reg[x] | reg[y];
			break;

		case 0x0002:
			reg[x] = reg[x] & reg[y];
			break;

		case 0x0003:
			reg[x] = reg[x] ^ reg[y];
			break;

		case 0x0004:
			reg[x] += reg[y];
			if (reg[x] + reg[y] > 0xF)
				reg[0xF] = 1;
			else
				reg[0xF] = 0;
			break;

		case 0x0005:
			reg[x] -= reg[y];
			if (reg[x] < reg[y])
				reg[0xF] = 0;
			else
				reg[0xF] = 1;
			break;

		case 0x0006:
			reg[0xF] = (reg[x] << 7) >> 7;
			reg[x] >>= 1;
			break;

		case 0x0007:
			reg[x] = reg[y] - reg[x];
			if (reg[x] > reg[y])
				reg[0xF] = 0;
			else
				reg[0xF] = 1;
			break;

		case 0x000E:
			reg[0xF] = reg[x] >> 7;
			reg[x] <<= 1;
			break;
		}

		pc += 2;
		break;
	}

	case 0x9000:
		pc += 2;
		if (reg[x] != reg[y])
			pc += 2;
		break;

	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000:
		pc = reg[0] + opcode & 0x0FFF;
		break;

	case 0xC000:
		reg[x] = rand() & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: {
		uint16_t height = opcode & 0x000F;
		uint8_t pixels;
		reg[0xF] = 0;

		for (int yline = 0; yline < height; yline++) {
			pixels = memory[I + yline];
			gfx[reg[y] + yline][reg[x]] ^= pixels;
		}

		I += 5;
		draw_flag = true;
		pc += 2;
		break;
	}

	case 0xE000: {
		switch (opcode & 0x00FF) {

		case 0x009E:
			pc += 2;
			if (key[&reg[x]] == 0)
				pc += 2;
			break;

		case 0x00A1:
			pc += 2;
			if (key[&reg[x]] != 0)
				pc += 2;
			break;
		}
	}

	case 0xF000: {
		switch (opcode & 0x00FF) {

		case 0x0007:
			reg[x] = delay_timer;
			break;

		case 0x000A:
			// reg[x] = pressed_key();
			break;

		case 0x0015:
			delay_timer = reg[x];
			break;

		case 0x0018:
			sound_timer = reg[x];
			break;

		case 0x001E:
			I += reg[x];
			if (I + reg[x] > 0xFFF)
				reg[0xF] = 1;
			else
				reg[0xF] = 0;
			break;

		case 0x0029:
			reg[x] = I;
			break;

		case 0x0033:
			memory[I] = reg[x] / 100;
			memory[I + 1] = (reg[x] / 10) % 10;
			memory[I + 2] = reg[x] % 10;
			break;

		case 0x0055:
			for (int i = 0; i <= x; i++)
				memory[I++] = reg[i];
			I--;
			break;

		case 0x0065:
			for (int i = 0; i <= x; i++)
				reg[i] = memory[I++];
			I--;
			break;
		}
		pc += 2;
	}
	}
}