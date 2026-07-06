#ifndef TEACHERPANEL_H
#define TEACHERPANEL_H

#include <QWidget>
#include <QVector>
#include <QJsonArray>

class QStackedWidget;
class QVBoxLayout;
class QTableWidget;
class QSpinBox;
class QTextEdit;
class QPushButton;
class QLabel;
class QLineEdit;
class QDialog;
class QFrame;
class ApiClient;

class teacherpanel : public QWidget
{
    Q_OBJECT

public:
    explicit teacherpanel(QWidget *parent = nullptr);
    ~teacherpanel();
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
    QWidget* createPageHomeworkManage();
    QWidget* createPageGrading();
    QWidget* createEmptyState(const QString &msg);

    QDialog* createPublishDialog();
    QFrame* createStudentItem(int subId, const QString &name, const QString &status,
                              bool graded, bool active = false);

    void switchPage(int index);
    void updateSidebarActive(int index);
    void showToast(const QString &msg);
    void selectStudent(QFrame *item);

    // 数据加载
    void loadHomeworks();
    void populateHomeworkTable();
    void loadSubmissionsForGrading(int homeworkId);
    void populateStudentList();
    void showSubmissionDetail(int submissionId);

    QStackedWidget *m_stacked;
    QVector<QPushButton*> m_sidebarBtns;
    QLabel *m_toast = nullptr;
    QDialog *m_publishDialog;
    QLineEdit *m_hwTitleInput;
    QTextEdit *m_hwDescInput;
    QLineEdit *m_hwDeadlineInput;
    QLabel *m_dialogTitleLabel;
    QPushButton *m_dialogSubmitBtn;
    QLabel *m_topbarUsername;

    ApiClient *m_api = nullptr;

    // 编辑模式
    bool m_isEditMode = false;
    int m_editHomeworkId = -1;

    // 作业管理页
    QTableWidget *m_hwTable = nullptr;

    // 数据缓存
    QJsonArray m_homeworks;
    QJsonArray m_currentSubmissions;  // 当前批改作业的提交列表

    // 批改页
    QLabel *m_gradingTitle = nullptr;
    QVBoxLayout *m_studentListLayout = nullptr;  // 学生列表布局（直接引用，避免 findChild）
    QVector<QFrame*> m_studentItems;
    QFrame *m_selectedStudentItem = nullptr;
    int m_currentGradingHwId = -1;
    int m_currentSubId = -1;

    // 批改区 UI
    QLabel *m_gradingStudentName = nullptr;
    QLabel *m_gradingFileName = nullptr;
    QPushButton *m_downloadFileBtn = nullptr;
    QLabel *m_gradingContent = nullptr;
    QSpinBox *m_gradingScore = nullptr;
    QTextEdit *m_gradingComment = nullptr;
    QString m_currentFilePath;  // 当前提交的文件服务器路径
};

#endif // TEACHERPANEL_H
