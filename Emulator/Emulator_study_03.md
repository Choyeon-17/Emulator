# **Emulator_study_03**
## Game Loop
### The flow of the emulator
~~~ c++
#include // GLUT
#include // SDL
#include "chip8.h" // CPU core implementation

chip8 Chip_8;

int main()
{
    // 그래픽을 설정하고 입력 콜백을 등록합니다.
    setupGraphics();
    setupInput();

    // Chip8 시스템을 초기화하고 게임을 메모리에 로드합니다.
    Chip_8.initialize();
    Chip_8.loadGame("pong");

    // 에뮬레이션 루프
    while (true)
    {
        // 사이클을 한 번 에뮬레이트합니다.
        Chip_8.emulateCycle();

        // drawFlag가 설정되면, 스크린을 업데이트합니다.
        if (Chip_8.drawCycle)
            drawGraphics();

        // 키를 누른 상태를 저장합니다. (누르고 뗍니다.)
        Chip_8.setKeys();
    }

    return 0;
}
~~~
- Line 3-5: 이 예제에서는 opcode를 처리하기 위해 별도의 클래스를 작성한다고 가정합니다.  
- Line 10-11: 그래픽(창 크기, 디스플레이 모드)을 설정하고 시스템을 입력합니다.(콜백 관계)  
- Line 14: 메모리, 레지스터 그리고 스크린을 비웁니다.  
- Line 15: 메모리에 프로그램을 복사합니다.  
- Line 21: 시스템의 한 사이클을 에뮬레이트합니다.  
- Line 24: 모든 사이클이 스크린에 출력하는 역할을 하는 것은 아니므로, 스크린을 업데이트해야 할 때 drawFlag를 설정해야 합니다. 오직 두 개의 opcode만이 이 플래그를 설정해야 합니다:  
    - 0x00E0 - 스크린을 비웁니다.  
    - 0xDXYN - 스크린에 2D를 출력합니다.  
- Line 28: 키를 누르거나 뗐을 경우, 키패드를 에뮬레이트한 부분에서 이 상태를 저장해야 합니다.