// tst_moldata.cpp — unit tests for MolData static lookup functions.
//
// MolData is a pure static class (header-only implementation) providing:
//   NameToMW(element)       → atomic weight by symbol
//   MW(formula_string)      → molecular weight by formula (e.g. "CH4")
//   Hydrogens(element)      → normal valence / expected H count
//   bondLength(Z1, Z2, ord) → typical bond length in Å
//
// No Qt widget, Render2D, or OpenBabel dependency — QTEST_APPLESS_MAIN.

#include <QtTest>
#include <cmath>
#include "../xdrawchem/moldata.h"

class TestMolData : public QObject
{
    Q_OBJECT

private slots:

    // ── NameToMW: atomic weights ────────────────────────────────────────────
    void nameToMW_hydrogen()
    {
        QVERIFY(std::abs(MolData::NameToMW("H") - 1.00794) < 1e-4);
    }

    void nameToMW_carbon()
    {
        QVERIFY(std::abs(MolData::NameToMW("C") - 12.011) < 1e-3);
    }

    void nameToMW_nitrogen()
    {
        QVERIFY(std::abs(MolData::NameToMW("N") - 14.0067) < 1e-4);
    }

    void nameToMW_oxygen()
    {
        QVERIFY(std::abs(MolData::NameToMW("O") - 15.9994) < 1e-4);
    }

    void nameToMW_sulfur()
    {
        QVERIFY(std::abs(MolData::NameToMW("S") - 32.066) < 1e-3);
    }

    void nameToMW_chlorine()
    {
        QVERIFY(std::abs(MolData::NameToMW("Cl") - 35.4527) < 1e-4);
    }

    void nameToMW_bromine()
    {
        QVERIFY(std::abs(MolData::NameToMW("Br") - 79.904) < 1e-3);
    }

    void nameToMW_iodine()
    {
        QVERIFY(std::abs(MolData::NameToMW("I") - 126.9045) < 1e-4);
    }

    void nameToMW_case_insensitive()
    {
        QVERIFY(std::abs(MolData::NameToMW("c") - MolData::NameToMW("C")) < 1e-10);
        QVERIFY(std::abs(MolData::NameToMW("cl") - MolData::NameToMW("Cl")) < 1e-10);
    }

    void nameToMW_unknown_returns_carbon_default()
    {
        // Unknown elements return 6.0 (carbon default)
        QCOMPARE(MolData::NameToMW("Xx"), 6.0);
    }

    // ── MW: molecular formula parser ────────────────────────────────────────
    void mw_single_carbon()
    {
        // "C" → 12.011
        QVERIFY(std::abs(MolData::MW("C") - 12.011) < 1e-2);
    }

    void mw_water_H2O()
    {
        // H2O = 2*1.00794 + 15.9994 = 18.015
        double expected = 2 * 1.00794 + 15.9994;
        QVERIFY(std::abs(MolData::MW("H2O") - expected) < 0.01);
    }

    void mw_methane_CH4()
    {
        // CH4 = 12.011 + 4*1.00794 = 16.043
        double expected = 12.011 + 4 * 1.00794;
        QVERIFY(std::abs(MolData::MW("CH4") - expected) < 0.01);
    }

    void mw_ammonia_NH3()
    {
        // NH3 = 14.0067 + 3*1.00794 = 17.030
        double expected = 14.0067 + 3 * 1.00794;
        QVERIFY(std::abs(MolData::MW("NH3") - expected) < 0.01);
    }

    void mw_ethanol_C2H6O()
    {
        // C2H6O = 2*12.011 + 6*1.00794 + 15.9994 = 46.068
        double expected = 2 * 12.011 + 6 * 1.00794 + 15.9994;
        QVERIFY(std::abs(MolData::MW("C2H6O") - expected) < 0.01);
    }

    void mw_benzene_C6H6()
    {
        // C6H6 = 6*12.011 + 6*1.00794 = 78.112
        double expected = 6 * 12.011 + 6 * 1.00794;
        QVERIFY(std::abs(MolData::MW("C6H6") - expected) < 0.01);
    }

    void mw_acetic_acid_C2H4O2()
    {
        // C2H4O2 = 2*12.011 + 4*1.00794 + 2*15.9994 = 60.052
        double expected = 2 * 12.011 + 4 * 1.00794 + 2 * 15.9994;
        QVERIFY(std::abs(MolData::MW("C2H4O2") - expected) < 0.01);
    }

    void mw_chloromethane_CH3Cl()
    {
        // CH3Cl = 12.011 + 3*1.00794 + 35.4527 = 50.487
        double expected = 12.011 + 3 * 1.00794 + 35.4527;
        QVERIFY(std::abs(MolData::MW("CH3Cl") - expected) < 0.01);
    }

    void mw_increases_with_atoms()
    {
        // C < CC: more carbons → higher MW
        QVERIFY(MolData::MW("CC") > MolData::MW("C"));
    }

    // ── Hydrogens: normal valence ────────────────────────────────────────────
    void hydrogens_carbon_is_4()
    {
        QCOMPARE(MolData::Hydrogens("C"), 4);
    }

    void hydrogens_nitrogen_is_3()
    {
        QCOMPARE(MolData::Hydrogens("N"), 3);
    }

    void hydrogens_oxygen_is_2()
    {
        QCOMPARE(MolData::Hydrogens("O"), 2);
    }

    void hydrogens_hydrogen_is_1()
    {
        QCOMPARE(MolData::Hydrogens("H"), 1);
    }

    void hydrogens_chlorine_is_1()
    {
        QCOMPARE(MolData::Hydrogens("Cl"), 1);
    }

    void hydrogens_sulfur_is_2()
    {
        QCOMPARE(MolData::Hydrogens("S"), 2);
    }

    void hydrogens_silicon_is_4()
    {
        QCOMPARE(MolData::Hydrogens("Si"), 4);
    }

    void hydrogens_functional_groups()
    {
        // Functional groups treated as single-bond substituents
        QCOMPARE(MolData::Hydrogens("OH"), 1);
        QCOMPARE(MolData::Hydrogens("NH2"), 1);
        QCOMPARE(MolData::Hydrogens("COOH"), 1);
        QCOMPARE(MolData::Hydrogens("NO2"), 1);
    }

    void hydrogens_NH_is_2()
    {
        // NH has 2 free valences (secondary amine nitrogen)
        QCOMPARE(MolData::Hydrogens("NH"), 2);
    }

    // ── bondLength: typical covalent bond lengths (Å) ─────────────────────────
    void bondLength_CC_single_is_1_54()
    {
        // C–C single bond: 1.54 Å
        QVERIFY(std::abs(MolData::bondLength(6, 6, 1) - 1.54) < 0.01);
    }

    void bondLength_CC_double_is_1_34()
    {
        // C=C double bond: 1.34 Å
        QVERIFY(std::abs(MolData::bondLength(6, 6, 2) - 1.34) < 0.01);
    }

    void bondLength_CC_triple_is_1_20()
    {
        // C≡C triple bond: 1.20 Å
        QVERIFY(std::abs(MolData::bondLength(6, 6, 3) - 1.20) < 0.01);
    }

    void bondLength_CH_is_1_09()
    {
        // C–H bond: 1.09 Å
        QVERIFY(std::abs(MolData::bondLength(6, 1, 1) - 1.09) < 0.01);
    }

    void bondLength_CN_is_1_47()
    {
        // C–N single bond: 1.47 Å
        QVERIFY(std::abs(MolData::bondLength(6, 7, 1) - 1.47) < 0.01);
    }

    void bondLength_CO_is_1_43()
    {
        // C–O single bond: 1.43 Å
        QVERIFY(std::abs(MolData::bondLength(6, 8, 1) - 1.43) < 0.01);
    }

    void bondLength_CF_is_1_35()
    {
        // C–F single bond: 1.35 Å
        QVERIFY(std::abs(MolData::bondLength(6, 9, 1) - 1.35) < 0.01);
    }

    void bondLength_CCl_is_1_77()
    {
        // C–Cl single bond: 1.77 Å
        QVERIFY(std::abs(MolData::bondLength(6, 17, 1) - 1.77) < 0.01);
    }

    void bondLength_symmetric_argument_order()
    {
        // bondLength normalises Z1, Z2 (swaps if Z1 > Z2), result must be same
        QCOMPARE(MolData::bondLength(6, 8, 1), MolData::bondLength(8, 6, 1));
        QCOMPARE(MolData::bondLength(6, 7, 1), MolData::bondLength(7, 6, 1));
        QCOMPARE(MolData::bondLength(1, 6, 1), MolData::bondLength(6, 1, 1));
    }

    void bondLength_unknown_returns_default_1_54()
    {
        // Unknown bond type falls back to C–C single (1.54 Å)
        QVERIFY(std::abs(MolData::bondLength(14, 14, 1) - 1.54) < 0.01);
    }

    void bondLength_double_shorter_than_single()
    {
        QVERIFY(MolData::bondLength(6, 6, 2) < MolData::bondLength(6, 6, 1));
    }

    void bondLength_triple_shorter_than_double()
    {
        QVERIFY(MolData::bondLength(6, 6, 3) < MolData::bondLength(6, 6, 2));
    }
};

QTEST_APPLESS_MAIN(TestMolData)
#include "tst_moldata.moc"
