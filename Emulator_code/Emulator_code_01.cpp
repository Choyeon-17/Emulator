#include <iostream>
#include <cstdint>

using namespace std;

uint8_t rom[4] =
{
	0x53, 0x00,
	0x13, 0x00
};

uint8_t ram[4] =
{
	0x12, 0x00,
	0x27, 0x00
};

int main()
{
	uint16_t opcode;
	int pc = 0;
	uint8_t tmpF, tmpB;

	for (; ; pc += 2)
	{
		opcode = (rom[pc] << 8) | (rom[pc + 1]);

		switch (opcode & 0xF000)
		{
		case 0x5000:
			cin >> hex >> tmpF >> hex >> tmpB;
			ram[pc] = tmpF;
			ram[pc + 1] = tmpB;
			cout << "0x" << hex << ((ram[pc] << 8) | (ram[pc + 1])) << endl;
			break;

		case 0x1000:
			cout << "0x" << hex << ((ram[pc] << 8) | (ram[pc + 1])) << endl;
			break;

		default:
			cout << "Unknown opcode!" << endl;
			return 1;
		}
	}
}