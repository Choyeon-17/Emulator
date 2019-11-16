# **The Timers**
## Timer Explanation:
이 섹션을 시작하기 섹션에 설명된 타이밍과 혼동해서는 안됩니다. 그 타이머들은 에뮬레이션 속도를 높이고 CPU를 그래픽과 동기화하는 것이었습니다. 게임 보이에는 메모리에 타이머가 있어, 특정 주파수에서 카운트 업하고 오버플로가 발생했을 때 인터럽트를 요청합니다. 이것이 이 섹션의 타이머입니다.

타이머는 메모리 주소 0xFF05에 있으며, 설정된 주파수에서 카운트 업됩니다. 카운트 업 주파수는 메모리 주소 0xFF07의 타이머 컨트롤러에 의해 설정됩니다. 타이머 오버플로가 발생하면(메모리 요소가 모두 부호없는 바이트이므로 오버플로가 255보다 큽니다.), 타이머에 인터럽트를 요청한 다음 메모리 주소 0xFF06의 타이머 변조기 값으로 재설정합니다. 이것은 다음과 같은 정의를 제공합니다:
~~~c++
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07
~~~
## Emulating Time:
타이머 컨트롤러(TMA)가 타이머(TIMA)가 카운트 업하도록 설정할 수 있는 주파수는 4가지 입니다:
~~~
4096 Hz
262144 Hz
65536 Hz
16384 Hz
~~~
첫 번째(4096 Hz)를 예로 들면, 이는 타이머가 초당 4096회 값을 증가시켜야 함을 의미합니다. 또한, 타이머 변조기가 항상 0이라고 가정하면(타이머가 오버플로 될 때마다 다시 0부터 카운트를 시작함을 의미합니다.), 타이머는 초당 16회 오버플로합니다(4096 / 256). 이는 타이머 에뮬레이션이 정확하고 올바른 속도로 타이머 인터럽트를 유발할 것임을 모니터하기 위해 중요합니다. 예를 들어, 주파수 4096을 코드에서 에뮬레이트하는 방법은 이 프레임을 실행한 각 opcode 클럭 사이클의 총계를 유지함으로써 CPU 타이밍을 정확하게 에뮬레이션할 수 있습니다. CPU 클럭 속도는 4194304 Hz에서 실행되므로 현재 타이머 주파수를 알고 있으면 타이머 레지스터를 증가시킬 때까지 얼마나 많은 클럭 사이클을 통과해야 하는지 알아낼 수 있습니다. 공식은 다음과 같습니다.
~~~c++
#define CLOCKSPEED 4194304
int m_TimerCounter = CLOCKSPEED / frequency;
~~~
현재 주파수가 4096 Hz이면 타이머 카운터는 1024(CLOCKSPEED / 4096)입니다. 이것은 '1024 클럭 사이클마다 CPU 시간이 한 번 증가해야 한다.'는 것을 의미합니다. 그러나, 주파수가 16384이면 카운터는 256(CLOCKSPEED / 16384)이 됩니다. 이는 "256 클럭 주기마다 CPU가 타이머를 한 번 증가시켜야 한다."는 것을 의미합니다.

기본 시작 에뮬레이션 업데이트 루프(초당 60번)를 보면, UpdateTimers() 함수를 사용하여 타이머를 이미 고려했으며, 마지막 opcode의 클럭 사이클에서 Im이 지나가는 것을 볼 수 있습니다.
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
        DoInterrupts();
    }
    RenderScreen();
}
~~~
다음은 UpdateTimers() 함수를 구현하는 방법입니다.
~~~c++
void Emulator::UpdateTimers(int cycles)
{
    DoDividerRegister(cycles);

    // 클럭을 업데이트하려면 클럭을 활성화해야 합니다.
    if (IsclockEnabled())
    {
        m_TimerCounter -= cycles;

        // 타이머를 업데이트하기에 충분한 CPU 클럭 사이클이 발생했습니다.
        if (m_TimerCounter <= 0)
        {
            // m_TimerTracer를 올바른 값으로 재설정합니다.
            SetClockFreq();

            // 오버플로 타이머입니다.
            if (ReadMemory(TIMA) == 255)
            {
                WriteMemory(TIMA, ReadMemory(TMA));
                RequestInterrupt(2);
            }
            else
                WriteMemory(TIMA, ReadMemory(TIMA) + 1);
        }
    }
}
~~~
DoDividerRegister() 함수와 IsClockEnabled() 함수는 잠시 후에 구현됩니다. IsClockEnabled() 함수는 기본적으로 타이머 계산을 일시 중지하거나 다시 시작하는 타이머 컨트롤러(TMC)의 설정입니다. IsClockEnabled() 함수가 false를 반환하면 타이머 자체가 재설정되지 않으며, 둘 다 다시 활성화될 때까지 일시 중지됩니다. SetClockFreq() 함수도 잠시 후에 구현될 것입니다. 그 목적은 0에 도달하면 m_TimerCounter를 현재 클럭 주파수에 대한 올바른 값으로 재설정하여 올바른 속도로 다시 카운트 다운을 시작할 수 있도록 하는 것입니다. 코드의 나머지 부분은 현재 타이머(TIMA)의 값을 증가시키고, 오버플로 여부를 확인합니다. 오버플로가 발생하면 타이머(TIMA)를 타이머 변조기(TMA)의 값으로 재설정하고 타이머 중단을 요청합니다.
## The Timer Controller:
타이머 컨트롤러(TMC)는 타이머(DUH!)를 제어하는 3비트 레지스터입니다. 비트 1과 0은 타이머를 증가시킬 주파수를 지정하기 위해 결합됩니다. 다음은 매핑입니다:
~~~
00: 4096 Hz
01: 262144 Hz
10: 65536 Hz
11: 16384 Hz
~~~
비트 2는 타이머 활성화 여부(1) 또는 비활성화 여부(0)를 지정합니다. 이 정보를 사용하여 IsClockEnabled() 함수를 다음과 같이 작성할 수 있습니다:
~~~c++
bool Emulator::IsClockEnabled() const
{
    return TestBit(ReadMemory(TMC), 2)?true:false;
}
~~~
주파수의 기본 값은 4096 Hz이나, 주파수가 변경되었는지 확인하는 방법을 모니터링해야 합니다. 가장 쉬운 방법은 메모리 쓰기 기능을 편집하여 게임에서 타이머 컨트롤러를 변경하려고 하는지 감지하는 것입니다. 게임에서 타이머 컨트롤러를 변경하는 경우 현재 클럭 주파수가 게임에서 변경하려는 것과 다른지 확인하고, 타이머 카운터를 많이 재설정하여 새 주파수로 계산합니다. 메모리 쓰기 기능에 다음 코드를 추가하면 간단합니다:
~~~c++
else if (TMC == address)
{
    BYTE currentfreq = GetClockFreq();
    m_GameMemory[TMC] = data;
    BYTE newfreq = GetClockFreq();

    if (currentfreq != newfreq)
        setClockFreq();
}
~~~
GetClockFreq() 함수 및 SetClockFreq() 함수는 다음과 같이 정의됩니다.
~~~c++
// 클럭 주파수는 TMC의 비트 1과 0의 조합입니다.
BYTE Emulator::GetClockFreq() const
{
    return ReadMemory(TMC) & 0x3;
}

void Emulator::SetClockFreq()
{
    BYTE freq = GetClockFreq();
    switch (freq)
    {
        case 0: m_TimerCounter = 1024 ; break ; // freq 4096
        case 1: m_TimerCounter = 16 ; break ;// freq 262144
        case 2: m_TimerCounter = 64 ; break ;// freq 65536
        case 3: m_TimerCounter = 256 ; break ;// freq 16382
    }
}
~~~
m_TimerCounter는 CLOCKSPEED / frequency 값으로 설정됩니다.
## Divider Register:
에뮬레이션이 필요한 마지막 타이밍 관련 영역은 Divider 레지스터입니다. 타이머와 매우 유사하게 작동하므로 이 섹션에 타이머를 포함시켰고, UpdateTimers() 함수 내에 에뮬레이트하는 코드를 넣었습니다. 작동 방식은 지속적으로 0에서 255까지 카운트한 다음 오버플로가 발생하면 0에서 다시 시작합니다. 오버플로 시 인터럽트를 발생시키지 않으며, 타이머처럼 일시 중지할 수 없습니다. 16382의 주파수에서 카운트 업되므로 256 CPU 클럭 사이클마다 Divider 레지스터를 증가시켜야 합니다. 언제 증가가 필요한지 추적하기 위해 m_TimerCounter와 같은 다른 int 카운터가 필요하며, m_DividerCounter라고 불리는 이것은 처음에 0으로 설정되고 지속적으로 255로 증가하다가 다시 시작됩니다. Divider 레지스터는 레지스터 주소 0xFF04에 있습니다. UpdateTimers() 함수에서 호출된 DoDividerRegister() 함수는 다음과 같이 에뮬레이트 됩니다.
~~~c++
void Emulator::DoDividerRegister(int cycles)
{
    m_DividerRegister += cycles;
    if (m_DividerRegister >= 255)
    {
        m_DividerRegister = 0;
        ++m_ROM[0xFF04];
    }
}
~~~
WriteMemory() 함수를 사용하지 않고 Divider 레지스터를 직접 증가시키는 이유는 게임 보이 하드웨어 Divider 레지스터에 쓰는 것을 허용하지 않으며, 게임이 그렇게 할 때마다 Divider 레지스터를 0으로 재설정하기 때문입니다. WriteMemory() 함수에서 이 기능을 직접 구현해야 합니다.
~~~c++
// Divider 레지스터를 트랩합니다.
else if (0xFF04 == address)
    m_ROM[0xFF04] = 0;
~~~