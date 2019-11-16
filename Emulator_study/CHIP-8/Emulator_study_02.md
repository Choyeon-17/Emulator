# **Emulator_study_02**
## A brief overview of the CHIP-8 system and some hints on how to implement the essential parts:
### Opcode
CHIP-8에는 35개의 opcode들이 있으며 모두 2 bytes입니다. 현재의 opcode를 저장하기 위해서, 2 bytes 저장을 허락하는 데이터 형이 필요합니다. unsigned short 형은 2 bytes의 길이를 가지고 있고, 그러므로 조건을 만족합니다.  
~~~ c++
unsigned short opcode;
~~~
### Memory
CHIP-8은 총 4K의 메모리를 가지고 있으며, 다음과 같이 에뮬레이션 할 수 있습니다.  
~~~ c++
unsigned char memory[4096];
~~~
### CPU registers
CHIP-8에는 V0, V1이라는 15개의 8 bits 범용 레지스터가 최대 VE까지 있습니다. 16번째 레지스터(VF)는 'carry flag'로 사용되기 때문에 범용 레지스터의 범위에 포함되지 않습니다. 8 bits는 1byte이므로 이 목적을 위해 unsigned char 형을 사용할 수 있습니다.  
~~~ c++
unsigned char V[16];
~~~
### Index register and a program counter
인덱스 레지스터 I와 0x000부터 0xFFF까지의 값을 가질 수 있는 프로그램 카운터가 있습니다. 
~~~ c++
unsigned short I;
unsigned short pc;
~~~
### The systems memory map
~~~ c++
0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
0x200-0xFFF - Program ROM and work RAM
~~~
### The graphics system
CHIP-8은 스크린에 2D 형식으로 출력하는 하나의 명령을 갖는다. 출력은 XOR 모드에서 이루어지며 출력 결과로 픽셀이 꺼지면 VF 레지스터가 설정됩니다. 이것은 충돌 감지에 사용됩니다. CHIP-8의 그래픽은 검정색과 하얀색이고, 스크린은 총 64 x 32개의 픽셀을 갖습니다. 이것은 픽셀 상태(1 or 0)를 가지는 배열을 사용함으로써 쉽게 구현될 수 있습니다.  
~~~ c++
unsigned char gfx[64 * 32];
~~~
### The Chip-8 doesn't have interupts and hardware registers
그러나 CHIP-8은 60Hz마다 세는 두 개의 타이머 레지스터가 있습니다. 0 이상으로 설정되면, 0까지 카운트 다운할 것입니다.  
~~~ c++
unsigned char delay_timer;
unsigned char sound_timer;
~~~
### The system's buzzer
사운드 타이머가 0에 도달할 때마다 시스템의 버저가 울립니다.
### Stack
CHIP-8 명령어 세트에는 프로그램이 특정 주소로 점프하거나 서브 루틴을 호출할 수 있는 opcode가 있음을 알아야 합니다. CPU specification 부분에는 스택이 언급되어 있지 않으나, 인터프리터의 일부로 스택을 구현해야 합니다. 스택은 점프가 수행되기 전에 현재 위치를 기억하는 데 사용됩니다. 따라서, 점프를 수행하거나 서브 루틴을 호출할 때마다 진행하기 전에 프로그램 카운터를 스택에 저장해야 합니다. 시스템에는 16단계의 스택이 있으며 사용된 스택 단계를 기억하려면, 스택 포인터(sp)를 구현해야 합니다.  
~~~ c++
unsigned short stack[16];
unsigned short sp;
~~~
### HEX based keypad
CHIP-8은 16진수 기반 키패드(0x0-0xF)가 있고, 키의 현재 상태를 저장하기 위해서 배열을 사용할 수 있습니다.  
~~~ c++
unsigned char key[16];
~~~