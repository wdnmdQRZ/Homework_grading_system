#ifndef STUDENTPANEL_H
#define STUDENTPANEL_H

#include <QWidget>
#include <QVector>
#include <QJsonArray>

class QStackedWidget;
class QTableWidget;
class QPushButton;
class QLabel;
class QFrame;
class QVBoxLayout;
class QTextEdit;
class ApiClient;

class studentpanel : public QWidget
{
    Q_OBJECT

public:
    explicit studentpanel(QWidget *parent = nullptr);
    ~studentpanel();
    void setApiClient(ApiClient *api);
    void setUsername(const QString &username);

signals:
    void logoutRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUi();
    QWidget* createTopbar();
    QWidget* createSidebar();
    QWidget* createPageHomeworkList();
    QWidget* createPageDetail();
    QWidget* createPageMySubmissions();
    QWidget* createEmptyState(const QString &msg);

    void switchPage(int index);
    void updateSidebarActive(int index);
    void showToast(const QString &msg);

    void loadHomeworks();
    void showHomeworkDetail(int homeworkId);
    void loadMySubmissions();

    QFrame* createHomeworkCard(int homeworkId, const QString &title, const QString &desc,
                               const QString &teacher, const QString &deadline,
                               const QString &status, bool expired = false);
    void populateHomeworkList();
    void populateMySubmissions();

    QStackedWidget *m_stacked;
    QVector<QPushButton*> m_sidebarBtns;
    QLabel *m_toast = nullptr;
    QLabel *m_topbarUsername;

    ApiClient *m_api = nullptr;

    QJsonArray m_homeworks;
    QJsonArray m_submissions;
    QVector<int> m_cardHomeworkIds;

    QWidget *m_detailPage = nullptr;
    QLabel *m_detailTitle = nullptr;
    QLabel *m_detailDesc = nullptr;
    QLabel *m_detailTeacher = nullptr;
    QLabel *m_detailDeadline = nullptr;
    QLabel *m_detailStatus = nullptr;
    QWidget *m_submitArea = nullptr;
    QWidget *m_submittedArea = nullptr;
    QLabel *m_submittedFileName = nullptr;
    QLabel *m_submittedTime = nullptr;
    QLabel *m_submittedScore = nullptr;
    QLabel *m_submittedComment = nullptr;
    QString m_selectedFilePath;
    QTextEdit *m_contentEdit = nullptr;
    int m_currentHomeworkId = -1;

    QTableWidget *m_submissionsTable = nullptr;
    QWidget *m_submissionDetailArea = nullptr;

    QVBoxLayout *m_homeworkListLayout = nullptr;
};

#endif // STUDENTPANEL_H
