# **ROM and RAM Banking**
## Detecting Rom Bank Mode:
ROM 뱅킹에는 MBC1과 MBC2의 두 가지 유형이 있습니다. 대부분의 게임(80%)은 MBC1이므로 적절한 에뮬레이터를 사용하려면 에뮬레이션해야 합니다. Tetris 및 Bubble Ghost와 같은 일부 게임은 ROM 뱅킹을 전혀 사용하지 않습니다. 그들은 전체 게임을 메모리 영역 0x0-0x8000에 로드하고 메모리를 스왑할 필요가 없습니다. 게임이 어떤 ROM 모드인지 감지하려면 게임이 메모리에 로드된 후, 메모리 0x147을 읽어야 합니다. 0x147이 0이면 게임에 메모리 뱅킹이 없으나, 1, 2 또는 3이면 MBC1이고 5 또는 6이면 MBC2입니다. 이것은 다음 코드를 제공합니다:
~~~c++
m_MBC1 = false;
m_MBC2 = false;

switch (m_CartridgeMemory[0x147])
{
    case 1 ... 3 : m_MBC1 = true; break;
    case 5 ... 6 : m_MBC2 = true; break;
    default : break;
}
~~~
내부 메모리 주소 0x4000-0x7FFF에 현재 로드된 ROM 뱅크를 지정하려면 변수 선언도 필요합니다. ROM 뱅크 0은 메모리 주소 0x0-0x3FFF에 고정되므로 이 변수는 0이 아니어야 하며, 1 이상이어야 합니다. 에뮬레이터 로드 시 이 변수를 1로 초기화해야 합니다.
~~~c++
BYTE m_CurrentROMBank = 1;
~~~
## Detecting RAM Banking:
카트리지 메모리 주소 0x148은 게임의 RAM 뱅크 양을 나타냅니다. 최대 값은 4입니다. RAM 뱅크 한 개의 크기는 0x2000 바이트이므로 0x8000 크기의 배열이 있으면 모든 RAM 뱅크에 충분한 공간입니다. ROM 뱅킹과 마찬가지로 게임에서 0-3 사이의 값을 사용하는 RAM 뱅크를 가리키는 변수도 필요합니다. 이것은 우리에게 다음과 같은 선언을 제공합니다.
~~~c++
BYTE m_RAMBanks[0x8000];
BYTE m_CurrentRAMBank;
~~~
그런 다음, 다음과 같이 초기화해야 합니다.
~~~c++
memset(&m_RAMBanks, 0, sizeof(m_RAMBanks));
m_CurrentRAMBank = 0;
~~~
MBC2에서는 RAM 뱅킹이 사용되지 않습니다. 따라서, m_CurrentRAMBank는 항상 0입니다.
## Controlling reading from ROM and RAM:
Memory Control and Map 섹션에 명시된 바와 같이, 내부 메모리에 대한 읽기 및 쓰기를 제어해야 합니다. 판독을 제어하는 주된 이유는 모든 것이 올바른 ROM 및 RAM 뱅크에서 판독되도록 하기 위한 것입니다. 이것은 우리에게 다음과 같은 함수를 제공합니다.
~~~c++
// 읽기 전용 메모리는 멤버 변수를 수정해서는 안되므로 const를 사용합니다.
BYTE Emulator::ReadMemory(WORD address) const
{
    // ROM 메모리 뱅크에서 읽고 있습니까?
    if ((address >= 0x4000) && (address <= 0x7FFF))
    {
        WORD newAddress = address - 0x4000;
        return m_CartridgeMemory[newAddress + (m_CurrentROMBank * 0x4000)];
    }

    // RAM 메모리 뱅크에서 읽고 있습니까?
    else if ((address >= 0xA000) && (address <= 0xBFFF))
    {
        WORD newAddress = address - 0xA000;
        return m_RAMBanks[newAddress + (m_CurrentRAMBank * 0x2000)];
    }

    // else return memory
    return m_ROM[address];
}
~~~
내부 게임 보이 메모리에 액세스할 때, 항상 왜 이 메모리를 읽고 메모리를 작성해야 하는지 알 수 있어야 합니다.
## Changing the current ROM and RAM Banks:
게임에서 뱅크 변경을 요청하는 방식은 게임 보이가 ROM에 직접 쓰기를 시도하나, WriteMemory() 함수는 이를 트랩하고, ROM에 쓰려고 하는 이유를 해독합니다. ROM에 쓰려고 하는 메모리 주소에 따라 다른 조치를 취해야 합니다. 주소가 0x2000-0x4000 사이이면, ROM 뱅크 변경입니다. 주소가 0x4000-0x6000인 경우, 선택한 현재 ROM / RAM 모드에 따라 RAM 뱅크 변경 또는 ROM 뱅크 변경입니다. 값이 0x0-0x2000 사이이면 RAM 뱅크 쓰기를 활성화합니다. 이제 WriteMemory() 함수의 ROM 부분을 다음과 같이 변경할 수 있습니다.
~~~c++
void Emulator::WriteMemory(WORD address, BYTE data)
{
    if (address < 0x8000)
        HandleBanking(address, data);
    
    else if ((address >= 0xA000) && (address < 0xC000))
    {
        if (m_EnableRAM)
        {
            WORD newAddress = address - 0xA000;
            m_RAMBanks[newAddress + (m_CurrentRAMBank * 0x2000)] = data;
        }
    }

    // 이 함수의 나머지는 이전과 같이 수행됩니다.
}

void Emulator::HandleBanking(WORD address, BYTE data)
{
    // RAM 활성화
    if (address < 0x2000)
        if (m_MBC1 || m_MBC2)
            DoRAMBankEnable(address, data);
    
    // ROM 뱅크 변경
    else if ((address >= 0x200) && (address < 0x4000))
        if (m_MBC1 || m_MBC2)
            DoChangeLoROMBank(data);
    
    // ROM 또는 RAM 뱅크 변경
    else if((address >= 0x4000) && (address < 0x6000))
    {
        // MBC2에는 RAM 뱅크가 없으므로 항상 RAM 뱅크 0을 사용합니다.
        if (m_MBC1)
        {
            if (m_ROMBanking)
                DoChangeHiROMBank(data);
            else
                DoChangeRAMBank(data);
        }
    }

    // 위의 if문으로 ROM 뱅킹 또는 RAM 뱅킹을 수행하는지 여부가 변경됩니다.
    else if ((address >= 0x6000) && (address < 0x8000))
        if (m_MBC1)
            DoChangeROMRAMMode(data);
}
~~~
## Enabling RAM:
RAM 뱅크에 쓰려면 게임에서 구체적으로 램 뱅크 쓰기가 활성화되도록 요청해야 합니다. 0에서 0x2000 사이의 내부 ROM 주소에 쓰려고 시도합니다. MBC1의 경우 게임에서 메모리에 쓰는 데이터의 하단 니블이 0xA이면 RAM 뱅크 쓰기가 활성화되고, 하단 니블이 0이면 RAM 뱅크 쓰기가 비활성화됩니다. MBC2는 주소 바이트의 비트 4가 0이어야 하는 점을 제외하고는 동일합니다. 이것은 다음 기능을 제공합니다:
~~~c++
void Emulator::DoRAMBankEnable(WORD address, BYTE data)
{
    if (m_MBC2)
        if (TestBit(address, 4) == 1)
            return;
    
    BYTE testData = data & 0xF;
    if (testData == 0xA)
        m_EnableRAM = true;
    else if (testData == 0x0)
        m_EnableRAM = false;
}
~~~
## Changing ROM Banks Part 1:
메모리 뱅크가 MBC1인 경우, 현재 ROM 뱅크를 변경하는 데는 두 가지 방법이 있습니다. 첫 번째 방법은 게임이 메모리 주소 0x2000-0x3FFF에 쓰면 현재 롬 뱅크의 하위 5비트를 변경하나, 비트 5와 6은 변경하지 않는 것입니다. 두 번째 방법은 ROM 뱅킹 모드동안 메모리 주소 0x4000-0x5FFF에 쓰는 것인데, 비트 0-4는 아닌 비트 5와 6만 변경합니다. 따라서, 이 두 가지 방법을 결합하면 현재 ROM 뱅크가 사용 중인 비트 0-6을 변경할 수 있습니다. 그러나, 게임에서 MBC2를 사용하는 경우 훨씬 쉽습니다. 게임이 주소 0x2000-0x3FFF에 쓰면 현재 RAM 뱅크는 비트 0-3을 변경하고, 비트 5-6은 설정되지 않습니다. 이는 MBC2 모드에서 0x4000-0x5FFF를 주소로 쓰는 것이 아무 것도 수행하지 않음을 의미합니다. 이 섹션에서는 게임이 메모리 주소 0x2000-0x3FFF에 쓸 때 발생하는 상황에 대해 설명합니다.
~~~c++
void Emulator::DochangeLoROMBank(BYTE data)
{
    if (m_MBC2)
    {
        m_CurrentROMBank = data & 0xF;
        if (m_CurrentROMBank == 0)
            m_CurrentROMBank++;
        return;
    }

    BYTE lower5 = data & 31;
    m_CurrentROMBank &= 224; // lower5를 끕니다.
    m_CurrentROMBank != lower5;
    if (m_CurrentROMBank == 0)
        m_CurrentROMBank++;
}
~~~
따라서, MBC2를 사용하는 경우 현재 ROM 뱅크는 데이터의 하위 니블이 됩니다. 그러나, MBC1을 사용하는 경우 상위 3비트를 동일하게 유지하면서 하위 5비트의 현재 ROM 뱅크를 하위 5비트의 데이터로 설정해야 합니다. m_CurrentROMBank가 0으로 설정되어 있으면 증가하는 이유는 ROM 뱅크 0은 정적이며 항상 메모리 주소 0x000-0x4000에서 찾을 수 있기 때문에, ROM 뱅크 0은 메모리 0x4000-0x8000에 로드되지 않아야 합니다. m_CurrentROMBank가 0으로 설정된 경우 rombank 1로 처리되므로 rombank 1 이상은 0x4000-0x8000 뱅크 영역에 있습니다.
## Changing ROM Banks Part 2:
위에서 언급한 바와 같이 MBC1 모드에서 현재의 ROM 뱅크를 변경하는 두 가지 방법이 있습니다. 메모리 주소 0x4000-0x6000에 쓸 때, 비트 5와 6을 변경하는 방법과 m_ROMBanking이 true인 경우입니다.
~~~c++
void Emulator::DoChangeHiROMBank(BYTE data)
{
    // 현재 ROM의 상위 3비트를 끕니다.
    m_CurrentROMBank &= 31;

    // 데이터의 하위 5비트를 끕니다.
    data &= 224;
    m_CurrentROMBank != data;
    if (m_CurrentROMBank == 0)
        m_CurrentROMBank++;
}
~~~
## Changing RAM Banks:
카트리지에 외부 RAM이 있으므로 MBC2의 RAM 뱅크를 변경할 수 없습니다. MBC1에서 RAM 뱅크를 변경하려면 게임을 다시 메모리 주소 0x4000-0x6000에 기록해야 하나, 이번에는 m_ROMBanking이 false여야 합니다. 현재 RAM 뱅크는 다음과 같이 데이터의 하위 2비트로 설정됩니다:
~~~c++
void Emulator::DoChangeRAMBank(BYTE data)
{
    m_CurrentRAMBank = data & 0x3;
}
~~~
## Selecting either ROM or RAM banking mode:
마지막 부분은 계속 진행되는 이 m_ROMBanking입니다. 이 변수는 게임을 메모리 주소 0x4000-0x6000에 쓸 때 작동하는 방법을 담당합니다. 이 변수의 기본 값은 true이나, 게임을 메모리 주소 0x6000-0x8000에 쓸 때 MBC1 모드 동안 변경됩니다. 이 주소 범위에 기록되는 데이터의 최하위 비트가 0이면 m_ROMBanking이 true로 설정되고, 그렇지 않으면 RAM 뱅크 변경이 발생한다는 것을 의미하는 false로 설정됩니다. 게임 보이는 이 모드에서 RAM 뱅크 0만 사용할 수 있으므로 m_ROMBanking을 true로 설정할 때마다 m_CurrentRAMBank를 0으로 설정하는 것이 중요합니다.
~~~c++
void Emulator::DoChangeROMRAMMode(BYTE data)
{
    BYTE newData = data & 0x1;
    m_ROMBanking = (newData == 0)?true:false;
    if (m_ROMBanking)
        m_CurrentRAMBank = 0;
}
~~~