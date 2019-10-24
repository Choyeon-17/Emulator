# **Emulator_study_05**
## Getting started
이제 에뮬레이션의 기본 사항과 시스템 작동 방식을 알았으므로, 모든 조각을 모아서 에뮬레이터 코딩을 시작해야 합니다.
### Initialize system
첫 번째 에뮬레이션 사이클을 실행하기 전에, 시스템 상태를 준비해야 합니다. 메모리를 비우고 레지스터를 0으로 재설정합니다. CHIP-8에는 실제로 BIOS 또는 펌웨어가 없으나, 기본 글꼴 세트가 메모리에 저장되어 있습니다. 이 글꼴 세트는 메모리 위치 0x50 == 80 이하에서 로드해야 합니다. 기억해야 할 또 다른 중요한 사항은 시스템이 응용 프로그램이 메모리 위치 0x200에서 로드될 것으로 예상한다는 것입니다. 즉, 프로그램 카운터도 이 위치로 설정해야 합니다.
~~~c++
void chip8::initialize()
{
    pc = 0x200; // 프로그램 카운터는 0x200에서 시작합니다.
    opcode = 0; // 현재의 opcode를 재설정합니다.
    I = 0;      // 인덱스 레지스터를 재설정합니다.
    sp = 0;     // 스택 포인터를 재설정합니다.

    // 디스플레이를 비웁니다.
    // 스택을 비웁니다.
    // 레지스터 V0-VF를 비웁니다.
    // 메모리를 비웁니다.

    // 글꼴 세트를 로드합니다.
    for(int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // 타이머를 재설정합니다.
}
~~~
### Loading the program into the memory
에뮬레이터를 초기화 한 후, 프로그램을 메모리에 로드하고(바이너리 모드에서 fopen 사용) 0x200 == 512 위치에서 메모리를 채웁니다.
~~~c++
for (int i = 0; i < bufferSize; ++i)
    memory[i + 512] = buffer[i];
~~~
### Start the emulation
시스템은 이제 첫 번째 opcode를 실행할 준비가 되었습니다. 위에서 언급했듯이, opcode를 가져오고, 디코딩하고 실행해야 합니다. 이 예제에서는 현재 opcode의 첫 4 비트(0xA2F0에서 A에 해당하는 부분)를 읽어 opcode가 무엇인지, 에뮬레이터가 무엇을 해야 하는지 알아 볼 수 있습니다.
~~~c++
void chip8::emulateCycle()
{
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode opcode
    switch(opcode & 0xF000)
    {
        // Some opcodes //

        case 0xA000: // ANNN: I를 주소 NNN으로 설정한다.
            // Execute opcode
            I = opcode & 0x0FFF;
            pc += 2;
        break;

        // More opcodes //

        default:
            printf("Unknown opcode: 0x%x\n", opcode);
    }

    // Update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }
}
~~~
어떤 경우에는 opcode의 의미를 보기 위해 처음 4 비트에만 의존할 수 없습니다. 예를 들어, 0x00E0 및 0x00EE는 모두 0x0으로 시작합니다. 이 경우 추가 스위치를 추가하고 마지막 4 비트를 비교합니다.
~~~c++
// Decode opcode
switch(opcode & 0xF000)
{
    case 0x0000:
        switch(opcode & 0x000F)
        {
            case 0x0000: // 0x00E0: 스크린을 비우는 명령어
                // Execute opcode
            break;

            case 0x000E: // 0x00EE: 서브루틴으로부터 반환해주는 명령어
                // Execute opcode
            break;

            default:
                printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }

    break;

    // more opcodes //
}
~~~