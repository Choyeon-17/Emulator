# **Memory Control and Map**
## Memory Map:
nocash-s pandocs에서 가져온 메모리 맵은 다음과 같습니다.
~~~
0000-3FFF 16KB ROM Bank 00 (in cartridge, fixed at bank 00)
4000-7FFF 16KB ROM Bank 01..NN (in cartridge, switchable bank number)
8000-9FFF 8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
A000-BFFF 8KB External RAM (in cartridge, switchable bank, if any)
C000-CFFF 4KB Work RAM Bank 0 (WRAM)
D000-DFFF 4KB Work RAM Bank 1 (WRAM) (switchable bank 1-7 in CGB Mode)
E000-FDFF Same as C000-DDFF (ECHO) (typically not used)
FE00-FE9F Sprite Attribute Table (OAM)
FEA0-FEFF Not Usable
FF00-FF7F I/O Ports
FF80-FFFE High RAM (HRAM)
FFFF Interrupt Enable Register
~~~
앞에서 설명한 것처럼 내부 메모리에는 게임 메모리의 0x8000(0x0000-0x7FFF) 공간만 있습니다. 그러나, 대부분의 게임 크기는 0x8000보다 크기 때문에 메모리 뱅킹이 필요합니다. 첫 0x4000 바이트는 게임 메모리의 첫 0x4000(0000-0x3FFF)(ROM bank 0) 바이트가 배치되는 위치입니다. 두 번째 0x4000(0x4000-0x7FFF)도 게임 메모리 용이나, 이 영역은 ROM bank 영역이므로 게임의 현재 상태에 따라 이 메모리 영역에 다른 게임 메모리 뱅크가 포함됩니다. ECHO 메모리 영역(0xE000-0xFDFF)은 여기에 기록된 모든 데이터가 해당 RAM 메모리 영역 0xC000-0xDDFF에도 기록되므로 매우 다릅니다. 그래서 에코라고 불리는 것입니다. RAM 메모리에 쓰는 것을 ECHO 메모리에도 쓰는지는 모르겠으나, 이것을 에뮬레이션 할 때마다 제대로 작동하지 않았고, 가져오니까 아무런 문제가 없었습니다. HRAM은 스택이 데이터를 저장하는 위치입니다. 스택 포인터는 0xFFFE에서 시작하여 무언가가 스택에 푸시될 때마다 메모리에서 작동합니다.
## Memory Control:
메모리에 대한 읽기 및 쓰기 액세스를 잘 제어해야 합니다. 예를 들어, 처음 0x8000 바이트는 읽기 전용이므로 아무 것도 쓰지 않아야 합니다. 또한, ECHO 메모리에 기록되는 내용은 작업 RAM에 반영되어야 합니다. 또한, 뱅크 중 하나에서 읽을 때 올바른 뱅크에서 읽어야 합니다. 메모리에 대한 읽기 및 쓰기 액세스를 제어하는 가장 좋은 방법은 ReadMemory 함수 및 WriteMemory 함수를 만드는 것입니다. 메모리를 읽고 쓸 때마다 이 두 기능을 사용해야 합니다. 이렇게 하면 메모리 액세스가 올바르게 제어됩니다. 이에 대한 유일한 예외는 이전 섹션 하드웨어에 표시된 ROM 메모리를 초기화 할 때입니다. 아래는 WriteMemory가 메모리 액세스를 제어하는 데 어떻게 도움이 되는지에 대한 예입니다.
~~~c++
void Emulator::WriteMemory(WORD address, BYTE data)
{
    if (address < 0x8000)
        // ROM에 쓰기 작업을 허락하지 않습니다.

    else if ((address>=0xE000) && (address<0xFE00>))
    {
        m_Rom[address] = data;
        WriteMemory(address-0x2000, data);
        // RAM에 적은 것과 같이 ECHO ram에도 적습니다.
    }

    else if ((address>=0xFEA0) && (address<0xFEFF>))
    {
        // 이 영역은 제한되어 있습니다.
    }

    else
    {
        m_Rom[address] = data;
        // 이 영역을 제어할 필요가 없으므로 메모리에 씁니다.
    }
}
~~~
이것은 WriteMemory 함수의 매우 단순화된 버전이나, 그 중요성에 대한 기본 아이디어를 제공해야 합니다.