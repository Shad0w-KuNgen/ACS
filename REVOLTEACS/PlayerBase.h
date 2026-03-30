#pragma once

// =============================================================================
// CPlayerBase — Oyuncu veri okuma sinifi
// KO_PTR_CHR (0x01092964) uzerinden bellekten oyuncu verilerini okur
// =============================================================================

#include <string>

class CPlayerBase
{
public:
    CPlayerBase();
    ~CPlayerBase();

    // Bellekten tum oyuncu verilerini guncelle
    void UpdateFromMemory();

    // Sinif tipi kontrol fonksiyonlari
    bool isWarrior() const;
    bool isRogue() const;
    bool isMage() const;
    bool isPriest() const;
    bool isKurian() const;

    // Sinif tipi bilgi fonksiyonlari
    uint16 GetClassType() const;
    uint8  GetBaseClassType() const;

    // =========================================================================
    // Oyuncu Veri Alanlari
    // =========================================================================

    // Kimlik
    uint16      m_iSocketID;            // KO_OFF_ID        (0x690)
    std::string m_strCharacterName;     // KO_OFF_NAME       (0x694)
    uint16      m_sClass;               // KO_OFF_CLASS      (0x6A4)
    uint8       m_iNation;              // KO_OFF_NATION     (0x6B4)
    uint8       m_iRace;                // KO_OFF_RACE       (0x6B8)
    uint8       m_iLevel;               // KO_OFF_LEVEL      (0x6C0)

    // Saglik / Mana
    int32       m_iMaxHp;               // KO_OFF_MAXHP      (0x6C4)
    int32       m_iHp;                  // KO_OFF_HP         (0x6C8)
    int32       m_iMaxMp;               // KO_OFF_MAXMP      (0xBBC)
    int32       m_iMp;                  // KO_OFF_MP         (0xBC0)

    // Pozisyon
    float       m_fX;                   // KO_OFF_X          (0x3D0)
    float       m_fY;                   // KO_OFF_Y          (0x3D4)
    float       m_fZ;                   // KO_OFF_Z          (0x3D8)

    // Hedef
    int16       m_iTargetID;            // KO_OFF_TARGET     (0x650)

    // Ekonomi
    uint32      m_iGold;                // KO_OFF_GOLD       (0xBCC)
    uint32      m_iNP;                  // KO_OFF_NP         (0xBE0)

    // Deneyim
    uint64      m_iExp;                 // KO_OFF_EXP        (0xBD8)
    uint64      m_iMaxExp;              // KO_OFF_MAXEXP     (0xBD0)

    // Stat degerleri
    uint32      m_iStr;                 // KO_OFF_STAT_STR   (0xBF4)
    uint32      m_iHpStat;             // KO_OFF_STAT_HP    (0xBFC)
    uint32      m_iDex;                 // KO_OFF_STAT_DEX   (0xC04)
    uint32      m_iInt;                 // KO_OFF_STAT_INT   (0xC0C)
    uint32      m_iMpStat;             // KO_OFF_STAT_MP    (0xC14)

    // Savas
    uint32      m_iAttack;              // KO_OFF_ATTACK     (0xC1C)
    uint32      m_iDefence;             // KO_OFF_DEFENCE    (0xC24)

    // Direncler
    uint32      m_iFireR;               // KO_OFF_FIRE_R     (0xC2C)
    uint32      m_iIceR;                // KO_OFF_ICE_R      (0xC34)
    uint32      m_iLightningR;          // KO_OFF_LIGHTNING_R (0xC3C)
    uint32      m_iMagicR;              // KO_OFF_MAGIC_R    (0xC44)
    uint32      m_iCurseR;              // KO_OFF_CURSE_R    (0xC4C)
    uint32      m_iPoisonR;             // KO_OFF_POISON_R   (0xC54)

    // Bolge
    uint8       m_iZoneID;              // KO_OFF_ZONE       (0xC60)

    // Hareket
    uint8       m_iStateMove;           // KO_OFF_STATEMOVE  (0x140)
    float       m_fYawToReach;          // KO_OFF_YAWTOREACH (0x17C)

    // Klan
    uint32      m_iKnightsID;           // KO_OFF_KNIGHTS_ID (0x6EC)

    // Agirlik (x10 olarak saklanir, gercek deger = okunan / 10)
    uint32      m_iWeight;              // KO_OFF_WEIGHT     (0xBF0) raw x10
    uint32      m_iMaxWeight;           // KO_OFF_MAXWEIGHT  (0xBE8) raw x10

    // Yardimci: gercek agirlik degerleri
    float GetWeight() const     { return m_iWeight / 10.0f; }
    float GetMaxWeight() const  { return m_iMaxWeight / 10.0f; }
};
