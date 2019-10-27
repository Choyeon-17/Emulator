# **Getting Started**
## Type definitions:
모든 머신과 마찬가지로 게임 보이는 주로 byte(8 bits)와 word(16 bits)와 같은 다른 크기의 데이터 타입을 사용합니다. z80과 같은 명령어 세트는 부호 있는 bytes와 부호 있는 words도 사용합니다. 따라서, 코드를 더 쉽게 이해하기 위해 다음에 나오는 것들을 typedef 했습니다.
~~~c++
typedef unsigned char BYTE;
typedef char SIGNED_BYTE;
typedef unsigned short WORD;
typedef signed short SIGNED_WORD;
~~~
이름을 변수가 저장할 수 있는 값 범위를 이해하기 쉽고 오버플로 오류를 추적할 수 있게 만드는 것과 동일한 방식으로 만드는 것이 좋습니다.
## Loading In The Game:
게임 보이 rom의 최대 크기는 0x200000 bytes이므로 이 크기의 배열이 있으면 게임을 저장할 수 있습니다. 이 배열은 게임 보이 카트리지를 나타냅니다. 내부 게임 보이 메모리는 카트리지의 첫 0x8000 bytes만 메모리에 저장하고 나머지는 카트리지에 남습니다.(에뮬레이터는 필요에 따라 카트리지의 데이터를 메모리로 스왑해야 합니다.)
### Declaration of cartridge memory:
~~~c++
BYTE m_CartridgeMemory[0x200000];
~~~
### Loading a game into cartridge memory:
~~~c++
memset(m_CartridgeMemory, 0, sizeof(m_CartridgeMemory));

FILE *in;
in = fopen("Tetris.gb", "rb");
fread(m_CartridgeMemory, 1, 0x200000, in);
fclose(in);
~~~
## The Emulation Loop:
CHIP-8과 달리 게임 보이 timing은 잘 문서화되어 있으며 게임 보이의 속도를 정확하게 에뮬레이트하는 것이 중요합니다. 이 게임 보이 문서는 각 opcode에 대한 클럭 사이클 타이밍을 제공합니다. 따라서, 각 명령이 원래 게임 보이 하드웨어에서 얼마나 많은 클럭 사이클을 수행하는지 알고 있다면, 이를 실행 카운터에 추가하여 타이밍을 추적할 수 있습니다. 각 명령의 소요 시간을 추적할 때 얻을 수 있는 주요 이점 중 하나는 이를 그래픽 에뮬레이션 및 게임 보이 클럭 타이머와 동기화하여 CPU와 동일한 속도로 에뮬레이션 할 수 있다는 것입니다.

그래픽 에뮬레이션에 SDL을 사용하며 에뮬레이터의 프레임 속도를 제한하는 기능이 제공됩니다. 프레임 속도를 초당 60 프레임으로 제한하기로 선택한 이유는 게임 보이가 초당 60회 화면을 새로 고치기 때문입니다. game pan docs 사이트에 따르면 게임 보이가 초당 예상할 수 있는 클럭 사이클의 양은 4194304이며, 이는 각 프레임마다 에뮬레이터를 초당 60회 업데이트하면 각 프레임은 69905(4194304 / 60) 클럭 사이클을 실행한다는 의미입니다. 그러면 에뮬레이터가 올바른 속도로 실행됩니다.
~~~c++
void Emulator::Update()
{
    const int MAXCYCLES = 69905;
    int cyclesThisUpdate = 0;

    while (cyclesThisUpdate < MAXCYCLES)
    {
        int cycles = ExecuteNextOpcode();
        cyclesThisUpdate += cycles;
        UpdateTimers(cycles);
        UpdateGraphics(cycles);
        DoInterupts();
    }
    
    RenderScreen();
}
~~~
따라서, 위 함수를 초당 60회 호출하면 RenderScreen도 게임 보이와 동일하게 초당 60회 호출됩니다. while 루프는 올바른 양의 명령이 이 프레임에서 실행되고 있는지 확인합니다.(물론 ExecuteNextOpcode() 함수가 다음 opcode가 실행하는 데 올바른 클럭 주기를 리턴한다고 가정합니다.) 또한, 타이머와 그래픽은 opcode에 걸린 클럭 사이클 수를 전달받아 CPU와 동일한 속도로 업데이트할 수 있습니다. 모든 명령 후에 인터럽트를 점검하고 필요한 경우 처리해야 합니다.