# **Emulator_study_04**
## Emulation cycle
~~~ c++
void chip8::initialize()
{
    // 레지스터와 메모리를 한 번 초기화합니다.
}

void chip8::emulateCycle()
{
    // Fetch Opcode
    // Decode Opcode
    // Execute Opcode

    // Update timers
}
~~~
모든 사이클에서, 함수 emulateCycle은 CHIP-8 CPU의 하나의 사이클을 에뮬레이트하는 것이라고 불립니다.
### Fetch opcode
이 단계에서, 시스템은 프로그램 카운터에 의해 지정된 위치의 메모리에서 하나의 opcode를 가져옵니다. CHIP-8 에뮬레이터에서 데이터는 1byte의 크기를 갖는 각 주소에 저장됩니다. 하나의 opcode가 2 bytes의 크기이므로, 실제 opcode를 얻기 위해서는 두 개의 연속 바이트를 가져와서 병합해야 합니다. 이것이 어떻게 작동하는지 보여주기 위해 opcode 0xA2F0을 사용할 것입니다.
~~~ c++
// 다음과 같이 가정합니다.
memory[pc] == 0xA2
memory[pc + 1] == 0xF0
~~~
두 바이트를 병합하고 unsigned short(2 bytes 데이터형)에 저장하려면 비트 OR 연산을 사용합니다.
~~~c++
opcode = memory[pc] << 8 | memory[pc + 1];
~~~
먼저 0xA2를 8 비트만큼 왼쪽으로 시프트하여 8개의 0을 추가합니다.
~~~c++
0xA2:        0xA2 << 8 = 0xA200  HEX
10100010:    1010001000000000    BIN
~~~
다음으로 비트 OR 연산을 사용하여 병합합니다.
~~~c++
1010001000000000 |  // 0xA200
        11110000 =  // 0xF00F0
-------------------
1010001011110000    // 0xA2F0
~~~
### Decode opcode
현재 opcode를 저장했으므로, opcode를 해독하고 opcode 테이블을 확인하여 그 의미를 확인해야 합니다. 동일한 opcode를 계속 사용할 것입니다.
~~~c++
0xA2F0 // 어셈블리어: mvi(A) 2F0h(hex)
~~~
opcode 테이블을 살펴보면 다음과 같이 알려줍니다.  
- ANNN: Sets I to the address NNN  
즉, 0xA2F0에서 A는 mvi 명령어이고 2F0은 I의 주소입니다.
### Execute opcode
이제 opcode로 무엇을 해야 하는지 알았으므로 에뮬레이터에서 opcode를 실행할 수 있습니다. 예제 명령어 0xA2F0의 경우 0x2F0 값을 인덱스 레지스터 I에 저장해야 합니다. 오직 12 비트만이 저장해야 하는 값을 포함하고 있으므로, 비트 AND 연산자(&)를 사용하여 처음 4 비트(nibble)를 제거합니다.
~~~c++
1010001011110000 &  // 0xA2F0 (opcode)
0000111111111111 =  // 0x0FFF
-------------------
0000001011110000    // 0x02F0(0x2F0)
~~~
즉, 주소 값을 만드는 과정이다.  
### Resulting code
~~~c++
I = opcode & 0x0FFF;
pc += 2;
~~~
모든 명령어의 길이는 2 bytes이므로, 실행된 opcode마다 프로그램 카운터를 2씩 증가시켜야 합니다. 메모리의 특정 주소로 이동하거나 서브 루틴을 호출하지 않는 경우(이 경우 스택에 프로그램 카운터를 저장해야 함) 이는 사실입니다. 다음 opcode를 건너 뛰려면 프로그램 카운터를 4씩 늘리면 됩니다.
### Timer
opcode를 실행하는 것 외에도, CHIP-8에는 또한 구현해야 할 두 개의 타이머가 있습니다. 위에서 언급했듯이, 두 타이머(delay timer와 sound timer)가 모두 0보다 큰 값으로 설정되어 있으면 카운트 다운을 0까지 합니다. 이러한 타이머들은 60Hz에서 카운트 다운되므로, 에뮬레이션 사이클을 늦추는 것을 구현할 수 있습니다.(1초에 60개의 opcode 실행) 즉, 1으로 설정되어 있으면 1초에 1개의 opcode를 실행하는 것입니다.