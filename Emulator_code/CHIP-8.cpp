#include <cstdio>
#include <cstdint>

unsigned char memory[4096] = { 0 }, V[16] = { 0 }, delay_timer, sound_timer;
unsigned short opcode = 0, I = 0, pc = 0x200, stack[16], sp = 0;

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
					// Execute opcode
					break;

				case 0x000E:
					// Execute opcode
					break;

				default:
					printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
				}
				break;
			case 0x1000:
				// Execute opcode
				break;
			}

			pc += 2;
		}
	}
};

int main()
{
	int timer;

	// setupGraphics();
	setupInput();

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