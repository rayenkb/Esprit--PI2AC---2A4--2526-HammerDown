#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QPoint>
#include <QModelIndex>

class QComboBox;
class QCamera;
class QMediaCaptureSession;
class QVideoSink;

// Employee declarations extracted from MainWindow and grouped by section.
#define MAINWINDOW_EMPLOYEE_PUBLIC_DECLS \
    void setupEmployeeModes(); \
    void setupEmployeeStats();

#define MAINWINDOW_EMPLOYEE_SLOT_DECLS \
    void onEmployeeClearFields(); \
    void onEmployeeAdd(); \
    void onEmployeeModify(); \
    void onEmployeeDelete(); \
    void onEmployeeRefreshView(); \
    void onEmployeeRefreshHistory(); \
    void updateSalaryInsight(); \
    void onSuggestSalary(); \
    void onStatsAiClicked(); \
    void onAIPulseClicked(); \
    void onAiPerformanceClicked(); \
    void onEmployeeSearch(); \
    void onEmployeeRowSelected(const QModelIndex &index); \
    void onEmployeeSendMail(); \
    void onEmployeeExportPDF(); \
    void onEmployeeExportHistoryPDF(); \
    void onEmployeeHistorySearch(); \
    void onEmployeeMailTemplateChanged(int index); \
    void processEmpCameraFrame();

#define MAINWINDOW_EMPLOYEE_PRIVATE_DECLS \
    void toggleEmployeeFields(bool active); \
    void onEmployeeEnsureHistoryTable(); \
    QCamera *m_empCamera = nullptr; \
    QMediaCaptureSession *m_empCaptureSession = nullptr; \
    QVideoSink *m_empVideoSink = nullptr; \
    bool m_isEmpFaceScanActive = false; \
    int m_faceScanStage = 0; \
    QString m_faceScanStatus;

#endif // EMPLOYEE_H
