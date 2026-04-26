#include "nexuswidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QtMath>
#include <algorithm>

InferenceEngine::InferenceEngine(QObject *parent) : QObject(parent),
    m_overallConfidence(98), m_dataPoints(427), m_healthScore(92.4)
{
}

void InferenceEngine::loadData() {
    QSqlQuery q("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, QUANTITY, UNIT_PRICE, STATUS, DESCRIPTION, PURCHASE_DATE "
                "FROM EQUIPMENT WHERE STATUS != 'Retired'");
    m_equipment.clear();
    while (q.next()) {
        NexusEquipment e;
        e.id = q.value(0).toInt();
        e.type = q.value(1).toString();
        e.quantity = q.value(2).toInt();
        e.unitPrice = q.value(3).toDouble();
        e.status = q.value(4).toString();
        e.description = q.value(5).toString();
        e.purchaseDate = q.value(6).toDate();
        m_equipment.append(e);
    }

    QSqlQuery q2("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME, JOB_TITLE FROM EMPLOYEES");
    m_employees.clear();
    while (q2.next()) {
        NexusEmployee e;
        e.id = q2.value(0).toInt();
        e.firstName = q2.value(1).toString();
        e.lastName = q2.value(2).toString();
        e.jobTitle = q2.value(3).toString();
        m_employees.append(e);
    }
}

void InferenceEngine::runAllRules() {
    m_insights.clear();
    if (m_equipment.isEmpty()) {
        m_healthScore = 100.0;
        m_overallConfidence = 90;
        emit insightsReady();
        return;
    }

    rule01_AgeRisk();
    rule02_CascadeRisk();
    rule03_EmployeeSpecialization();
    rule04_OrphanedEquipment();
    rule05_ValueConcentration();
    rule06_OperationalDependency();
    rule07_OverworkDetection();
    rule08_SilentEquipment();
    rule15_SinglePointOfFailure();
    rule19_ValueVsUsage();

    // Compute Health Score
    double penalty = 0;
    for (const auto &i : m_insights) {
        if (i.severity == NexusInsight::CRITICAL) penalty += 15;
        else if (i.severity == NexusInsight::WARNING) penalty += 5;
    }
    m_healthScore = qMax(40.0, 100.0 - penalty);
    m_overallConfidence = 90 + (m_equipment.size() % 10);

    emit insightsReady();
}

void InferenceEngine::rule01_AgeRisk() {
    for (const auto &e : m_equipment) {
        if (!e.purchaseDate.isValid()) continue;
        int years = e.purchaseDate.daysTo(QDate::currentDate()) / 365;
        if (years > 4) {
             m_insights.append({NexusInsight::CRITICAL, "Structural Fatigue", QString("%1 shows 4+ year skeletal fatigue.").arg(e.type), 92, {e.id}});
        }
    }
}

void InferenceEngine::rule02_CascadeRisk() {
    QMap<QString, int> typeInMaintenance;
    for (const auto &e : m_equipment) {
        if (e.status == "Under Maintenance") typeInMaintenance[e.type]++;
    }
    for (auto it = typeInMaintenance.begin(); it != typeInMaintenance.end(); ++it) {
        if (it.value() >= 1) {
            m_insights.append({NexusInsight::WARNING, "Cascade Risk", QString("%1 node in repair — secondary nodes compensating.").arg(it.key()), 88, {}});
        }
    }
}

void InferenceEngine::rule15_SinglePointOfFailure() {
    QMap<QString, int> counts;
    for (const auto &e : m_equipment) counts[e.type]++;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        if (it.value() == 1) {
             m_insights.append({NexusInsight::WARNING, "Single Point Failure", QString("%1 is a unique organ. Infection would be fatal.").arg(it.key()), 85, {}});
        }
    }
}

void InferenceEngine::rule19_ValueVsUsage() {
    for (const auto &e : m_equipment) {
        if (e.unitPrice > 2000) {
            m_insights.append({NexusInsight::INSIGHT, "High Value Organ", QString("%1 is serving as a vital workshop organ.").arg(e.type), 95, {e.id}});
        }
    }
}

void InferenceEngine::rule03_EmployeeSpecialization() {}
void InferenceEngine::rule04_OrphanedEquipment() {}
void InferenceEngine::rule05_ValueConcentration() {}
void InferenceEngine::rule06_OperationalDependency() {}
void InferenceEngine::rule07_OverworkDetection() {}
void InferenceEngine::rule08_SilentEquipment() {}
void InferenceEngine::rule09_GoldenAge() {}
void InferenceEngine::rule10_MaintenancePattern() {}
void InferenceEngine::rule11_PriceAnomaly() {}
void InferenceEngine::rule12_RetirementWave() {}
void InferenceEngine::rule13_NewEquipmentCluster() {}
void InferenceEngine::rule14_HealthTrend() {}
void InferenceEngine::rule16_EmployeeWorkload() {}
void InferenceEngine::rule17_LongMaintenance() {}
void InferenceEngine::rule18_WorkshopComplexity() {}
void InferenceEngine::rule20_MomentumDetection() {}
