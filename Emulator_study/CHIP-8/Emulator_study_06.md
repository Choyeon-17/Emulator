# **Emulator_study_06**
## Opcode examples
처음에는 어려워 보였던 더 많은 opcode들입니다.
### Example 1: Opcode 0x2NNN
이 opcode는 주소 NNN에서 서브 루틴을 호출합니다. NNN을 해결하기 위해 일시적으로 점프해야 하므로, 프로그램 카운터의 현재 주소를 스택에 저장해야 합니다. 프로그램 카운터의 값을 스택에 저장한 후, 현재 스택을 덮어 쓰지 않도록 스택 포인터를 늘립니다. 프로그램 카운터를 저장했으므로 주소 NNN으로 점프할 수 있습니다. 주의해야 할 점은 특정 주소에서 서브 루틴을 호출하므로 점프하기 때문에, 프로그램 카운터를 2만큼 늘리면 안된다는 점입니다.
~~~c++
case 0x2000:
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFF;
break;
~~~
### Example 2: Opcode 0x8XY4
이 opcode는 VY의 값을 VX에 추가합니다. carry가 있는 경우 레지스터 VF는 1로 설정되고 없는 경우 0으로 설정됩니다. 레지스터는 0에서 255까지의 값(8 비트 값)만 저장할 수 있기 때문에, VX와 VY의 합이 255보다 크면, 레지스터에 저장할 수 없습니다.(또는 실제로 0부터 다시 카운트하기 시작합니다.) VX와 VY의 합이 255보다 큰 경우, 캐리 플래그를 사용하여 시스템에 두 값의 총 합이 실제로 255보다 큼을 알립니다. opcode를 실행한 후 프로그램 카운터를 2씩 늘리는 것을 주의해야 합니다.
~~~c++
case 0x0004:
    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
        V[0xF] = 1; // carry
    else
        V[0xF] = 0;
    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
    pc += 2;
break;
~~~
### Example 3: Opcode 0xFX33
주소 I, I + 1 및 I + 2에 VX의 이진 코드 10진수 표현을 저장합니다. 이 opcode를 구현하는 방법을 알 수 없어서 TJA의 솔루션을 사용했습니다.
~~~c++
case 0x0033:
    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
    pc += 2;
break;
~~~