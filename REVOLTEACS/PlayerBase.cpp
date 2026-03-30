#include "pch.h"
#include "PlayerBase.h"

// =============================================================================
// CPlayerBase — Constructor / Destructor
// =============================================================================

CPlayerBase::CPlayerBase()
    : m_iSocketID(0)
    , m_strCharacterName("")
    , m_sClass(0)
    , m_iNation(0)
    , m_iRace(0)
    , m_iLevel(0)
    , m_iMaxHp(0)
    , m_iHp(0)
    , m_iMaxMp(0)
    , m_iMp(0)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_fZ(0.0f)
    , m_iTargetID(-1)
    , m_iGold(0)
    , m_iNP(0)
    , m_iExp(0)
    , m_iMaxExp(0)
    , m_iStr(0)
    , m_iHpStat(0)
    , m_iDex(0)
    , m_iInt(0)
    , m_iMpStat(0)
    , m_iAttack(0)
    , m_iDefence(0)
    , m_iFireR(0)
    , m_iIceR(0)
    , m_iLightningR(0)
    , m_iMagicR(0)
    , m_iCurseR(0)
    , m_iPoisonR(0)
    , m_iZoneID(0)
    , m_iStateMove(0)
    , m_fYawToReach(0.0f)
    , m_iKnightsID(0)
    , m_iWeight(0)
    , m_iMaxWeight(0)
{
}

CPlayerBase::~CPlayerBase()
{
}

// =============================================================================
// UpdateFromMemory — KO_PTR_CHR + offset ile tum degerleri bellekten oku
// =============================================================================

void CPlayerBase::UpdateFromMemory()
{
    // chrBase = *(DWORD*)KO_PTR_CHR
    DWORD chrBase = *(DWORD*)KO_PTR_CHR;

    // Null kontrolu — karakter yuklenmemisse guncelleme atla
    if (chrBase == 0)
        return;

    // Kimlik
    m_iSocketID = *(uint16*)(chrBase + KO_OFF_ID);
    m_sClass    = *(uint16*)(chrBase + KO_OFF_CLASS);
    m_iNation   = *(uint8*)(chrBase + KO_OFF_NATION);
    m_iRace     = *(uint8*)(chrBase + KO_OFF_RACE);
    m_iLevel    = *(uint8*)(chrBase + KO_OFF_LEVEL);

    // Isim — char* olarak oku, std::string'e kopyala
    char* pName = (char*)(chrBase + KO_OFF_NAME);
    if (pName != nullptr)
        m_strCharacterName = std::string(pName);
    else
        m_strCharacterName = "";

    // Saglik / Mana
    m_iMaxHp = *(int32*)(chrBase + KO_OFF_MAXHP);
    m_iHp    = *(int32*)(chrBase + KO_OFF_HP);
    m_iMaxMp = *(int32*)(chrBase + KO_OFF_MAXMP);
    m_iMp    = *(int32*)(chrBase + KO_OFF_MP);

    // Pozisyon
    m_fX = *(float*)(chrBase + KO_OFF_X);
    m_fY = *(float*)(chrBase + KO_OFF_Y);
    m_fZ = *(float*)(chrBase + KO_OFF_Z);

    // Hedef
    m_iTargetID = *(int16*)(chrBase + KO_OFF_TARGET);

    // Ekonomi
    m_iGold = *(uint32*)(chrBase + KO_OFF_GOLD);
    m_iNP   = *(uint32*)(chrBase + KO_OFF_NP);

    // Deneyim (uint64)
    m_iExp    = *(uint64*)(chrBase + KO_OFF_EXP);
    m_iMaxExp = *(uint64*)(chrBase + KO_OFF_MAXEXP);

    // Stat degerleri
    m_iStr    = *(uint32*)(chrBase + KO_OFF_STAT_STR);
    m_iHpStat = *(uint32*)(chrBase + KO_OFF_STAT_HP);
    m_iDex    = *(uint32*)(chrBase + KO_OFF_STAT_DEX);
    m_iInt    = *(uint32*)(chrBase + KO_OFF_STAT_INT);
    m_iMpStat = *(uint32*)(chrBase + KO_OFF_STAT_MP);

    // Savas
    m_iAttack  = *(uint32*)(chrBase + KO_OFF_ATTACK);
    m_iDefence = *(uint32*)(chrBase + KO_OFF_DEFENCE);

    // Direncler
    m_iFireR      = *(uint32*)(chrBase + KO_OFF_FIRE_R);
    m_iIceR       = *(uint32*)(chrBase + KO_OFF_ICE_R);
    m_iLightningR = *(uint32*)(chrBase + KO_OFF_LIGHTNING_R);
    m_iMagicR     = *(uint32*)(chrBase + KO_OFF_MAGIC_R);
    m_iCurseR     = *(uint32*)(chrBase + KO_OFF_CURSE_R);
    m_iPoisonR    = *(uint32*)(chrBase + KO_OFF_POISON_R);

    // Bolge
    m_iZoneID = *(uint8*)(chrBase + KO_OFF_ZONE);

    // Hareket
    m_iStateMove  = *(uint8*)(chrBase + KO_OFF_STATEMOVE);
    m_fYawToReach = *(float*)(chrBase + KO_OFF_YAWTOREACH);

    // Klan
    m_iKnightsID = *(uint32*)(chrBase + KO_OFF_KNIGHTS_ID);

    // Agirlik (x10 olarak saklanir)
    m_iWeight    = *(uint32*)(chrBase + KO_OFF_WEIGHT);
    m_iMaxWeight = *(uint32*)(chrBase + KO_OFF_MAXWEIGHT);
}

// =============================================================================
// Sinif Tipi Kontrol Fonksiyonlari
// =============================================================================
// Warrior: 1 (base), 5 (Blade), 6 (Berserker)
// Rogue:   2 (base), 7 (Assassin), 8 (Archer)
// Mage:    3 (base), 9 (Fire), 10 (Ice)
// Priest:  4 (base), 11 (Heal), 12 (Buffer)
// Kurian:  13 (Porutu), 14 (Divine), 15 (Shadow)
// =============================================================================

bool CPlayerBase::isWarrior() const
{
    return (m_sClass == CLASS_WARRIOR ||
            m_sClass == CLASS_WARRIOR_BLADE ||
            m_sClass == CLASS_WARRIOR_BERSERKER);
}

bool CPlayerBase::isRogue() const
{
    return (m_sClass == CLASS_ROGUE ||
            m_sClass == CLASS_ROGUE_ASSASSIN ||
            m_sClass == CLASS_ROGUE_ARCHER);
}

bool CPlayerBase::isMage() const
{
    return (m_sClass == CLASS_MAGE ||
            m_sClass == CLASS_MAGE_FIRE ||
            m_sClass == CLASS_MAGE_ICE);
}

bool CPlayerBase::isPriest() const
{
    return (m_sClass == CLASS_PRIEST ||
            m_sClass == CLASS_PRIEST_HEAL ||
            m_sClass == CLASS_PRIEST_BUFFER);
}

bool CPlayerBase::isKurian() const
{
    return (m_sClass == CLASS_KURIAN_PORUTU ||
            m_sClass == CLASS_KURIAN_DIVINE ||
            m_sClass == CLASS_KURIAN_SHADOW);
}

// =============================================================================
// GetClassType — mevcut sinif degerini dondur
// =============================================================================

uint16 CPlayerBase::GetClassType() const
{
    return m_sClass;
}

// =============================================================================
// GetBaseClassType — temel sinif tipini dondur (1=Warrior, 2=Rogue, vb.)
// =============================================================================

uint8 CPlayerBase::GetBaseClassType() const
{
    if (isWarrior()) return CLASS_WARRIOR;   // 1
    if (isRogue())   return CLASS_ROGUE;     // 2
    if (isMage())    return CLASS_MAGE;      // 3
    if (isPriest())  return CLASS_PRIEST;    // 4
    if (isKurian())  return CLASS_KURIAN_PORUTU; // 13 (Kurian base)
    return 0; // Bilinmeyen sinif
}
