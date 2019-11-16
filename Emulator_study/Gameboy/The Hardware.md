# **The Hardware**
## General Info:
아래의 모든 정보는 nocashs pan-docs에서 가져온 것입니다.
~~~
CPU: 8 bits(z80 프로세서와 유사합니다.)
Clock Speed: 4.194304MHz
Screen Resolution: 160 X 144
Vertiical Sync: 59.73Hz
Internal Memory size: 0x10000
~~~
CPU는 z80 프로세서와 매우 유사합니다.(두 개의 추가되는 명령이 있으며 모든 플래그가 사용되는 것은 아닙니다.)  
'Getting Started' 파일에서 정확한 타이밍과 수직 재생률을 위해 클럭 속도를 에뮬레이트하는 방법을 적었습니다. 화면 해상도를 에뮬레이션하는 방법에는 3D 배열을 사용하는 것이 있습니다. 첫 번째 부분은 화면 x축입니다. 두 번째는 y축이고 세 번째는 색상입니다.(빨간색, 녹색 및 파란색의 세 가지 요소) 이것은 다음 선언을 제공합니다.
~~~c++
BYTE m_ScreenData[160][144][3];
~~~
따라서, 화면의 중간 픽셀을 빨간색으로 설정하려면 다음과 같이 작성합니다.
~~~c++
m_ScreenData[160/2][144/2][0] = 0xFF;
m_ScreenData[160/2][144/2][1] = 0;
m_ScreenData[160/2][144/2][2] = 0;
~~~
CPU는 8 비트이고 메모리는 0x10000 바이트입니다. 따라서, 다음은 메인 메모리의 선언을 담당합니다.
~~~c++
BYTE m_Rom[0x10000];
~~~
## The Registers:
게임 보이에는 A, B, C, D, E, F, H, L이라는 8개의 레지스터가 있으며, 각각 크기는 8 비트입니다. 그러나, 이러한 레지스터는 종종 짝을 이루어 4개의 16 비트 레지스터를 형성합니다. 짝을 이루는 것은 AF, BC, DE, HL입니다. A는 누산기이고 F는 플래그 레지스터이기 때문에, AF는 다른 것보다 덜 사용됩니다. HL은 주로 게임 메모리를 나타내는 데 자주 사용됩니다.

코드에서 레지스터를 에뮬레이트하는 방법에는 여러 가지가 있습니다. 그 중 하나는 워드 변수로 각 레지스터 쌍을 표시하고, 개별 레지스터의 hi 및 lo 바이트를 검색하는 기능을 제공하는 것입니다. 또 다른 방법은 반대이며, 각 레지스터에 대해 8 바이트 변수를 가진 다음, 이들을 결합하여 쌍을 나타내는 워드를 형성하는 함수를 제공합니다. 그러나, 유니온을 사용하여 레지스터를 에뮬레이션하는 것을 선호합니다.
~~~c++
union Register
{
    WORD reg;
    struct
    {
        BYTE lo;
        BYTE hi;
    };
};

Register m_RegisterAF;
Register m_RegisterBC;
Register m_RegisterDE;
Register m_RegisterHL;
~~~
유니온의 각 필드는 메모리에서 동일한 영역을 차지합니다. 따라서, m_RegisterAF.reg = 0xAABB를 수행한 경우, m_RegisterAF.hi는 0xAA가 되고 m_RegisterAF.lo는 0xBB가 됩니다. 그런 다음, m_RegisterAF.lo = 0xCC를 수행할 수 있으며, m_RegisterAF.hi는 여전히 0xAA이나, m_RegisterAF.reg는 0xAACC입니다. 개별 레지스터(hi 및 lo 필드 포함)와 쌍(reg 필드 포함)에 쉽게 액세스할 수 있으므로 레지스터를 표시하는 데 적합합니다. BYTE lo가 BYTE hi 전에 선언된 이유는 endianess 때문입니다.
## The Flags:
앞에서 언급했듯이, F 레지스터는 플래그 레지스터로 두 배가 증가합니다. CPU가 사용하는 플래그는 "carry 플래그", "half carry 플래그", "subtract 플래그" 및 "zero 플래그"라는 4개의 플래그(8개가 있는 실제 z80과 반대)가 있습니다. carry 플래그는 계산 후에 설정되어 데이터 유형의 범위에 맞지 않는 값을 초래했습니다. 예를 들어 명령어가 레지스터 A를 250 + 7의 값으로 설정한 경우, A는 257로 설정됩니다. 그러나, 레지스터 A는 부호없는 바이트이므로 0과 255 사이의 값만 저장할 수 있습니다. 이는 A가 초과되었음을 의미합니다. 자체가 1이 되고, 캐리 플래그는 결과가 오버플로되었음을 표시하도록 설정됩니다.  
half carry 플래그는 계산이 하위 니블에서 상위 니블로 오버플로 할 때 또는 16 비트 연산에서 하위 바이트에서 상위 바이트로 오버플로된 경우를 제외하고 carry 플래그와 유사합니다. 이 플래그는 많은 명령어에 의해 설정되나, DAA 명령어에만 사용됩니다. 계산의 결과가 0이 되면, zero 플래그가 설정됩니다.  
subtract 플래그는 수학적인 연산이 뺄셈일 때 설정됩니다. 이것은 또한 DAA 지시에만 사용됩니다.  

플래그를 참조할 때 다음 정의를 사용합니다.
~~~c++
#define FLAG_Z 7
#define FLAG_N 6
#define FLAG_H 5
#define FLAG_C 4
~~~
각 값은 해당 플래그에 대한 F 레지스터의 비트입니다.
## Program Counter and Stack Pointer:
프로그램 카운터는 메모리에서 실행할 다음 opcode의 주소입니다. 일부 opcode는 인수로 사용하기 위해 메모리에서 다음 1-2 바이트가 필요합니다. 이 경우 프로그램 카운터는 다음 실제 opcode를 가리키도록 건너뛰어야 합니다. 게임 보이 메모리의 크기는 0x10000 바이트이며, 이는 0에서 0xFFFF의 범위를 갖습니다. 이것은 또한 부호없는 워드 데이터 타입과 동일한 범위이며, 프로그램 카운터를 나타내는 데 적합합니다. 프로그램 카운터는 호출 명령 및 인터럽트 실행과 함께 점프 명령에서 값을 변경합니다. 프로그램 카운터는 0x100으로 초기화됩니다.

스택은 데이터가 로드되는 메모리의 장소입니다. 스택에 놓인 마지막 데이터는 스택에서 처음 검색됩니다. 스택 포인터는 메모리에서 스택의 맨 위, 즉 스택에 추가할 다음 데이터 조각이 차지할 메모리 영역을 가리킵니다. 프로그램 카운터와 마찬가지로 스택 포인터가 가리키는 주소는 메모리이므로 WORD 데이터 형식은 스택 포인터를 잘 나타냅니다. 그러나, 일부 opcode는 스택 포인터의 hi 및 lo 바이트를 사용하므로 레지스터와 동일한 방식으로 스택 포인터를 에뮬레이션하는 것이 더 쉽습니다. 스택 포인터는 0xFFFE로 초기화됩니다.  
위의 정보는 다음과 같은 선언을 제공합니다.
~~~c++
WORD m_ProgramCounter;
Register m_StackPointer;
~~~
## Initializing:
nocashs-pan-docs에도 다음 정보가 있습니다. 에뮬레이터가 시작되면 레지스터, 스택 포인터, 프로그램 카운터 및 특수 ROM 레지스터의 상태를 다음으로 설정해야 합니다.
~~~c++
m_ProgramCounter=0x100 ;
m_RegisterAF=0x01B0;
m_RegisterBC=0x0013;
m_RegisterDE=0x00D8;
m_RegisterHL=0x014D;
m_StackPointer=0xFFFE;
m_Rom[0xFF05] = 0x00 ;
m_Rom[0xFF06] = 0x00 ;
m_Rom[0xFF07] = 0x00 ;
m_Rom[0xFF10] = 0x80 ;
m_Rom[0xFF11] = 0xBF ;
m_Rom[0xFF12] = 0xF3 ;
m_Rom[0xFF14] = 0xBF ;
m_Rom[0xFF16] = 0x3F ;
m_Rom[0xFF17] = 0x00 ;
m_Rom[0xFF19] = 0xBF ;
m_Rom[0xFF1A] = 0x7F ;
m_Rom[0xFF1B] = 0xFF ;
m_Rom[0xFF1C] = 0x9F ;
m_Rom[0xFF1E] = 0xBF ;
m_Rom[0xFF20] = 0xFF ;
m_Rom[0xFF21] = 0x00 ;
m_Rom[0xFF22] = 0x00 ;
m_Rom[0xFF23] = 0xBF ;
m_Rom[0xFF24] = 0x77 ;
m_Rom[0xFF25] = 0xF3 ;
m_Rom[0xFF26] = 0xF1 ;
m_Rom[0xFF40] = 0x91 ;
m_Rom[0xFF42] = 0x00 ;
m_Rom[0xFF43] = 0x00 ;
m_Rom[0xFF45] = 0x00 ;
m_Rom[0xFF47] = 0xFC ;
m_Rom[0xFF48] = 0xFF ;
m_Rom[0xFF49] = 0xFF ;
m_Rom[0xFF4A] = 0x00 ;
m_Rom[0xFF4B] = 0x00 ;
m_Rom[0xFFFF] = 0x00 ; 
~~~
다른 m_Rom 요소는 원래 게임 보이 내에서 임의의 값으로 설정되었으나, 여기서는 모두 0으로 설정했습니다.