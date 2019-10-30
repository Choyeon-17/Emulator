#include <cstdio>
#include <cstdint>
#include <GL/glut.h>

unsigned char memory[4096];

class Chip8
{
private:
	bool draw_flag;
	unsigned short opcode, I, pc, sp;
	unsigned short stack[16] = { 0 };
	unsigned char V[16] = { 0 }, gfx[64 * 32] = { 0 }, key[16] = { 0 };
	const unsigned char chip8_fontset[80] = 
	{
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
	Chip8()
		:opcode(0), I(0), pc(0x200), sp(0)
	{
		for (int i = 0; i < 80; ++i)
			memory[i] = chip8_fontset[i];
	}

	void emulateCycle(int timer)
	{
		opcode = memory[pc] << 8 | memory[pc + 1];

		switch (opcode & 0xF000)
		{
		case 0x0000:
			switch (opcode & 0x00FF)
			{
			case 0x00E0:
				// gfx[64 * 32] = { 0 };
				break;
			case 0x00EE:
				// 함수 호출
				break;
			default:
				// Calls RCA 1802 program at address NNN.
			}
			break;
		case 0x1000:
			// goto opcode & 0x0FFF;
			break;
		case 0x2000:
			// *(opcode & 0x0FFF)();
			break;
		case 0x3000:
			if (V[opcode & 0x0F00] == (opcode & 0x00FF))
				pc += 2;
			break;
		case 0x4000:
			if (V[opcode & 0x0F00] != (opcode & 0x00FF))
				pc += 2;
			break;
		case 0x5000:
			if (V[opcode & 0x0F00] == V[opcode & 0x00F0])
				pc += 2;
			break;
		case 0x6000:
			V[opcode & 0x0F00] = (opcode & 0x00FF);
			break;
		case 0x7000:
			V[opcode & 0x0F00] += (opcode & 0x00FF);
			break;
		case 0x8000:
			switch (opcode & 0x000F)
			{
			case 0x0000:
				V[opcode & 0x0F00] = V[opcode & 0x00F0];
				break;
			case 0x0001:
				V[opcode & 0x0F00] = V[opcode & 0x0F00] | V[opcode & 0x00F0];
				break;
			case 0x0002:
				V[opcode & 0x0F00] = V[opcode & 0x0F00] & V[opcode & 0x00F0];
				break;
			case 0x0003:
				V[opcode & 0x0F00] = V[opcode & 0x0F00] ^ V[opcode & 0x00F0];
				break;
			case 0x0004:
				V[opcode & 0x0F00] += V[opcode & 0x00F0];
				if ((V[opcode & 0x0F00] + V[opcode & 0x00F0]) > 0xF)
					V[0xF] = 1;
				else
					V[0xF] = 0;
				break;
			case 0x0005:
				V[opcode & 0x0F00] -= V[opcode & 0x00F0];
				if (V[opcode & 0x0F00] < V[opcode & 0x00F0])
					V[0xF] = 0;
				else
					V[0xF] = 1;
				break;
			case 0x0006:
				// V[0xF] = V[opcode & 0x0F00]의 최하위 비트;
 				V[opcode & 0x0F00] >>= 1;
				break;
			case 0x0007:
				V[opcode & 0x0F00] = V[opcode & 0x00F0] - V[opcode & 0x0F00];
				if (V[opcode & 0x0F00] > V[opcode & 0x00F0])
					V[0xF] = 0;
				else
					V[0xF] = 1;
				break;
			case 0x000E:
				// V[0xF] = V[opcode & 0x0F00]의 최상위 비트;
				V[opcode & 0x0F00] <<= 1;
				break;
			}
			break;
		case 0x9000:
			if (V[opcode & 0x0F00] != V[opcode & 0x00F0])
				pc += 2;
			break;
		case 0xA000:
			I = opcode & 0x0FFF;
			break;
		case 0xB000:
			pc = V[0] + (opcode & 0x0FFF);
			break;
		case 0xC000:
			V[opcode & 0x0F00] = rand() & (opcode & 0x00FF);
			break;
		case 0xD000:
			// draw(V[opcode & 0x0F00], V[opcode & 0x00F0], (opcode & 0x000F));
			break;
		case 0xE000:
			switch (opcode & 0x00FF)
			{
			case 0x009E:
				// if (key() == V[opcode & 0x0F00])
					// pc += 2;
				break;
			case 0x00A1:
				// if (key() != V[opcode & 0x0F00])
					// pc += 2;
				break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF)
			{
			case 0x0007:
				// V[opcode & 0x0F00] = get_delay();
				break;
			case 0x000A:
				// V[opcode & 0x0F00] = get_key();
				break;
			case 0x0015:
				// delay_timer(V[opcode & 0x0F00]);
				break;
			case 0x0018:
				// sound_timer(V[opcode & 0x0F00]);
				break;
			case 0x001E:
				I += V[opcode & 0x0F00];
				break;
			case 0x0029:
				// I = sprite_addr[(V[opcode & 0x0F00])];
				break;
			case 0x0033:
				break;
			case 0x0055:
				break;
			case 0x0065:
				break;
			}
			break;
		default:
			printf("Unknown opcode!\n");
		}

		pc += 2;
	}
};

int main()
{
	int timer;

	// setupGraphics();

	Chip8 Chip_8;

	// Chip_8.loadGame("pong");

	while (true)
	{
		Chip_8.emulateCycle(timer);

		printf("BEEP!\n");
	}

	return 0;
}