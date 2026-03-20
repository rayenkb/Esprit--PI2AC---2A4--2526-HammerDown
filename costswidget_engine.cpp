#include "costswidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtMath>
#include <algorithm>

// ============================================================================
// COST CALCULATION ENGINE
// ============================================================================
CostCalculationEngine::CostCalculationEngine(QObject *parent) : QObject(parent) {}

void CostCalculationEngine::calculate() {
    m_results.clear();
    m_totalInventory = 0;
    m_totalTCO = 0;
    m_forecast90 = 0;
    m_topPerformer = "N/A";
    m_urgentActions = 0;

    double maxPrice = 0;

    // First pass: load equipment and find max price
    QSqlQuery q("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, QUANTITY, UNIT_PRICE, STATUS, "
                "DESCRIPTION, PURCHASE_DATE, NEXT_MAINTENANCE FROM EQUIPMENT ORDER BY EQUIPMENT_ID");
    QList<EquipmentFinancials> tempList;
    while (q.next()) {
        EquipmentFinancials ef;
        ef.id = q.value(0).toInt();
        ef.name = q.value(1).toString();
        ef.type = q.value(1).toString();
        ef.status = q.value(4).toString();
        ef.purchasePrice = q.value(3).toDouble();
        ef.purchaseDate = q.value(6).toDate();

        if (ef.purchaseDate.isValid()) {
            ef.ageInDays = ef.purchaseDate.daysTo(QDate::currentDate());
            ef.ageInYears = ef.ageInDays / 365.25;
        } else {
            ef.ageInDays = 0;
            ef.ageInYears = 0;
        }

        if (ef.purchasePrice > maxPrice) maxPrice = ef.purchasePrice;
        m_totalInventory += ef.purchasePrice;
        tempList.append(ef);
    }

    if (tempList.isEmpty()) {
        emit calculationComplete();
        return;
    }

    // Second pass: count events per equipment from EQUIPMENT table data
    for (auto &ef : tempList) {
        // Maintenance events: equipment with NEXT_MAINTENANCE set or status Under Maintenance
        QSqlQuery mq;
        mq.prepare("SELECT COUNT(*) FROM EQUIPMENT WHERE EQUIPMENT_ID = :id "
                    "AND (NEXT_MAINTENANCE IS NOT NULL OR STATUS = 'Under Maintenance')");
        mq.bindValue(":id", ef.id);
        if (mq.exec() && mq.next()) {
            ef.maintenanceEvents = mq.value(0).toInt();
        }

        // History events: use quantity changes and status as proxy
        ef.historyEvents = qMax(1, (int)(ef.ageInYears * 2.0));
        if (ef.status == "Under Maintenance") ef.maintenanceEvents = qMax(ef.maintenanceEvents, 2);
        if (ef.status == "Retired") ef.maintenanceEvents = qMax(ef.maintenanceEvents, 4);

        // Estimated Maintenance Cost
        ef.estimatedMaintenanceCost = ef.maintenanceEvents * 50.0;

        // Estimated Operational Cost
        ef.estimatedOperationalCost = ef.historyEvents * (ef.purchasePrice * 0.001);

        // Total TCO
        ef.totalTCO = ef.purchasePrice + ef.estimatedMaintenanceCost + ef.estimatedOperationalCost;

        // Cost Per Year
        ef.costPerYear = (ef.ageInYears > 0.1) ? ef.totalTCO / ef.ageInYears : ef.totalTCO;

        // Cost Per Day
        ef.costPerDay = (ef.ageInDays > 0) ? ef.totalTCO / ef.ageInDays : ef.totalTCO;

        // Book Value (straight-line depreciation over 20 years)
        double depreciation = ef.purchasePrice * (ef.ageInYears / 20.0);
        ef.bookValue = qMax(0.0, ef.purchasePrice - depreciation);
        ef.depreciationPct = (ef.purchasePrice > 0) ? qMin(100.0, (depreciation / ef.purchasePrice) * 100.0) : 0.0;

        // ROI Score (0-100)
        double activity = qMin(ef.historyEvents * 2.0, 30.0);
        double value = (maxPrice > 0) ? qMin((ef.purchasePrice / maxPrice) * 25.0, 25.0) : 0;
        double reliability = qMax(0.0, 25.0 - ef.maintenanceEvents * 5.0);
        double longevity = (ef.ageInYears < 20) ? 20.0 : qMax(0.0, 20.0 - (ef.ageInYears - 20.0) * 2.0);
        ef.roiScore = activity + value + reliability + longevity;

        // Recommendation
        if (ef.roiScore >= 80) { ef.recommendation = "EXCEPTIONAL"; ef.recommendationColor = "#4CAF7D"; }
        else if (ef.roiScore >= 60) { ef.recommendation = "GOOD VALUE"; ef.recommendationColor = "#C17F3E"; }
        else if (ef.roiScore >= 40) { ef.recommendation = "MONITOR"; ef.recommendationColor = "#3B82F6"; }
        else if (ef.roiScore >= 20) { ef.recommendation = "REVIEW"; ef.recommendationColor = "#F59E0B"; }
        else { ef.recommendation = "RETIRE"; ef.recommendationColor = "#CC2200"; }

        // Repair vs Replace
        ef.repairCost = qMax(15.0, ef.purchasePrice * 0.15);
        ef.replacementCost = ef.purchasePrice * 0.9;
        ef.remainingYears = qMax(0, 20 - (int)ef.ageInYears);
        ef.monthlyBudgetNeeded = (ef.remainingYears > 0) ?
            ef.replacementCost / (ef.remainingYears * 12.0) : ef.replacementCost;

        m_totalTCO += ef.totalTCO;

        // Count urgent
        if (ef.roiScore < 20 || ef.remainingYears <= 2) m_urgentActions++;
    }

    // Sort by ROI for top performer
    auto sorted = tempList;
    std::sort(sorted.begin(), sorted.end(), [](const EquipmentFinancials &a, const EquipmentFinancials &b) {
        return a.roiScore > b.roiScore;
    });
    if (!sorted.isEmpty()) m_topPerformer = sorted.first().name;

    // 90-day forecast
    for (const auto &ef : tempList) {
        m_forecast90 += ef.monthlyBudgetNeeded * 3.0;
    }

    m_results = tempList;
    emit calculationComplete();
}

// ============================================================================
// GROQ FINANCIAL INSIGHT CLIENT
// ============================================================================
CostGroqClient::CostGroqClient(QObject *parent) : QObject(parent) {
    m_network = new QNetworkAccessManager(this);
}

void CostGroqClient::requestInsight(const QString &sysPrompt, const QString &userPrompt) {
    if (m_apiKey.isEmpty()) {
        m_lastInsight = "Configure API key for AI insights.";
        emit insightReady(m_lastInsight);
        return;
    }

    m_isLoading = true;

    QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sysPrompt;
    QJsonObject usrMsg; usrMsg["role"] = "user"; usrMsg["content"] = userPrompt;
    QJsonArray messages; messages.append(sysMsg); messages.append(usrMsg);

    QJsonObject body;
    body["model"] = "llama-3.3-70b-versatile";
    body["messages"] = messages;
    body["max_tokens"] = 1000;
    body["temperature"] = 0.7;

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QNetworkReply *reply = m_network->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_isLoading = false;
        QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            m_lastInsight = QString("AI_ERROR (%1): %2").arg(reply->error()).arg(QString::fromUtf8(data));
            emit insightReady(m_lastInsight);
            reply->deleteLater();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QString content;
        const QJsonArray choices = doc.object().value("choices").toArray();
        if (!choices.isEmpty()) {
            content = choices.at(0).toObject().value("message").toObject().value("content").toString().trimmed();
        }
        if (content.isEmpty()) content = "No insight generated.";
        m_lastInsight = content;
        emit insightReady(content);
        reply->deleteLater();
    });
}
