#include <cstdio>
#include <cstdint>

unsigned char V[16] = { 0 };
unsigned short opcode = 0, I = 0, pc = 0x200, stack[16], sp = 0;

unsigned char memory[4096] = 
{
    0x00, 0x00, 
    0x00, 0x0E, 
    0x20, 0x00, 
    0x53, 0x00, 
    0x13, 0x00
};

uint8_t ram[4] = 
{
    0x12, 0x00, 
    0x27, 0x00
};

class Chip8
{
public:
	void emulateCycle(int timer)
	{
		for (int i = timer; i > 0; i--)
		{
			opcode = memory[pc] << 8 | memory[pc + 1];

			switch (opcode & 0xF000)
			{
			case 0x0000:
				switch (opcode & 0x000F)
				{
				case 0x0000:
					// 스크린을 비운다.
					break;

				case 0x000E:
					// 서브 루틴으로부터 반환한다.
					break;

				default:
					printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
				}
				break;

			case 0x2000:
				stack[sp] = pc;
                ++sp;
                pc = opcode & 0x0FFF;
				break;

            case 0x5000:
                printf("My opcode: %X\n", (ram[pc - 6] << 8) | (ram[pc - 4]));
                break;

            case 0x1000:
                printf("Wizard\n");
                break;

            default:
                printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
			}

			pc += 2;
		}
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
		scanf("%d", &timer);

		if (timer == 0)
			break;
		
		Chip_8.emulateCycle(timer);

		printf("BEEP!\n");

		// if (Chip_8.drawCycle)
			// drawGraphics();

		// Chip_8.setKeys();
	}

	return 0;
}
