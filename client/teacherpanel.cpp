#include "teacherpanel.h"
#include "src/core/apiclient.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QTimer>
#include <QPropertyAnimation>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

teacherpanel::teacherpanel(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

teacherpanel::~teacherpanel() {}

void teacherpanel::setApiClient(ApiClient *api)
{
    m_api = api;
    loadHomeworks();
}

void teacherpanel::setUsername(const QString &username)
{
    m_topbarUsername->setText(username);
}

// ==================== Event Filter ====================

bool teacherpanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame *frame = qobject_cast<QFrame*>(obj);
        if (frame && frame->objectName() == "studentItem") {
            int subId = frame->property("submissionId").toInt();
            selectStudent(frame);
            showSubmissionDetail(subId);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// ==================== 整体布局 ====================

void teacherpanel::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTopbar());

    auto *body = new QHBoxLayout;
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);
    body->addWidget(createSidebar());

    m_stacked = new QStackedWidget;
    m_stacked->addWidget(createPageHomeworkManage());  // 0
    m_stacked->addWidget(createPageGrading());         // 1
    body->addWidget(m_stacked);

    mainLayout->addLayout(body, 1);
    m_publishDialog = createPublishDialog();
    switchPage(0);
}

// ==================== 顶部栏 ====================

QWidget* teacherpanel::createTopbar()
{
    auto *bar = new QFrame;
    bar->setObjectName("topbar");
    bar->setFixedHeight(56);
    auto *lay = new QHBoxLayout(bar);
    lay->setContentsMargins(24, 0, 24, 0);

    auto *title = new QLabel("作业管理系统");
    title->setObjectName("topbarTitle");
    auto *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_topbarUsername = new QLabel("教师");
    m_topbarUsername->setObjectName("topbarUsername");

    auto *roleTag = new QLabel("教师");
    roleTag->setObjectName("topbarRoleTeacher");

    auto *logoutBtn = new QPushButton("退出登录");
    logoutBtn->setObjectName("topbarLogoutBtn");
    connect(logoutBtn, &QPushButton::clicked, this, &teacherpanel::logoutRequested);

    lay->addWidget(title);
    lay->addWidget(spacer);
    lay->addWidget(m_topbarUsername);
    lay->addWidget(roleTag);
    lay->addWidget(logoutBtn);
    return bar;
}

// ==================== 侧边栏 ====================

QWidget* teacherpanel::createSidebar()
{
    auto *sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(200);
    auto *lay = new QVBoxLayout(sidebar);
    lay->setContentsMargins(0, 16, 0, 0);
    lay->setSpacing(0);

    struct BtnDef { QString icon; QString text; int page; };
    QVector<BtnDef> defs = {
        {"📋", "作业管理", 0},
        {"✏️", "批改作业", 1},
    };

    for (const auto &d : defs) {
        auto *btn = new QPushButton(d.icon + "  " + d.text);
        btn->setObjectName("sidebarBtn");
        btn->setCheckable(true);
        connect(btn, &QPushButton::clicked, this, [=]() {
            if (d.page == 0) loadHomeworks();
            switchPage(d.page);
        });
        lay->addWidget(btn);
        m_sidebarBtns.append(btn);
    }
    lay->addStretch();
    return sidebar;
}

// ==================== 页面 0：作业管理 ====================

QWidget* teacherpanel::createPageHomeworkManage()
{
    auto *wrapper = new QWidget;
    auto *wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);

    auto *titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(24, 0, 24, 0);
    auto *header = new QLabel("作业管理");
    header->setObjectName("pageHeader");
    titleRow->addWidget(header);
    titleRow->addStretch();

    auto *addBtn = new QPushButton("+ 发布新作业");
    addBtn->setObjectName("primaryBtn");
    connect(addBtn, &QPushButton::clicked, this, [=]() {
        m_isEditMode = false;
        m_editHomeworkId = -1;
        m_dialogTitleLabel->setText("发布新作业");
        m_dialogSubmitBtn->setText("发布作业");
        m_hwTitleInput->clear();
        m_hwDescInput->clear();
        m_hwDeadlineInput->setText(QDateTime::currentDateTime().addDays(7).toString("yyyy-MM-dd hh:mm"));
        m_publishDialog->exec();
    });
    titleRow->addWidget(addBtn);
    wrapperLayout->addLayout(titleRow);

    m_hwTable = new QTableWidget;
    m_hwTable->setColumnCount(4);
    m_hwTable->setHorizontalHeaderLabels({"作业名称", "截止时间", "提交人数", "操作"});
    m_hwTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_hwTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_hwTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_hwTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_hwTable->verticalHeader()->setVisible(false);
    m_hwTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_hwTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_hwTable->setObjectName("hwManageTable");
    wrapperLayout->addWidget(m_hwTable, 1);

    return wrapper;
}

// ==================== 页面 1：批改作业 ====================

QWidget* teacherpanel::createPageGrading()
{
    auto *wrapper = new QWidget;
    auto *mainLay = new QVBoxLayout(wrapper);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    auto *titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(24, 0, 24, 0);
    m_gradingTitle = new QLabel("批改作业");
    m_gradingTitle->setObjectName("pageHeader");
    titleRow->addWidget(m_gradingTitle);
    titleRow->addStretch();

    auto *selectLabel = new QLabel("选择作业：");
    selectLabel->setObjectName("infoLabel");
    titleRow->addWidget(selectLabel);

    auto *hwSelectBtn = new QPushButton("请选择作业 ▾");
    hwSelectBtn->setObjectName("sidebarBtn");
    titleRow->addWidget(hwSelectBtn);

    auto *refreshBtn = new QPushButton("刷新");
    refreshBtn->setObjectName("sidebarBtn");
    connect(refreshBtn, &QPushButton::clicked, this, [=]() {
        if (m_currentGradingHwId > 0)
            loadSubmissionsForGrading(m_currentGradingHwId);
    });
    titleRow->addWidget(refreshBtn);
    mainLay->addLayout(titleRow);

    auto *content = new QHBoxLayout;
    content->setContentsMargins(24, 16, 24, 24);
    content->setSpacing(16);

    // 左侧：学生列表
    auto *leftPanel = new QFrame;
    leftPanel->setObjectName("gradingLeftPanel");
    leftPanel->setFixedWidth(220);
    auto *leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(0);

    auto *stuLabel = new QLabel("提交学生");
    stuLabel->setObjectName("sectionTitle");
    stuLabel->setContentsMargins(16, 16, 16, 8);
    leftLay->addWidget(stuLabel);

    auto *stuScroll = new QScrollArea;
    stuScroll->setWidgetResizable(true);
    stuScroll->setFrameShape(QFrame::NoFrame);
    auto *stuContainer = new QWidget;
    auto *stuLayout = new QVBoxLayout(stuContainer);
    stuLayout->setContentsMargins(8, 0, 8, 8);
    stuLayout->setSpacing(4);
    stuLayout->addStretch();
    m_studentListLayout = stuLayout;  // 保存直接引用
    stuScroll->setWidget(stuContainer);
    leftLay->addWidget(stuScroll);
    content->addWidget(leftPanel);

    // 右侧：批改区
    auto *rightPanel = new QScrollArea;
    rightPanel->setWidgetResizable(true);
    rightPanel->setFrameShape(QFrame::NoFrame);

    auto *gradingWidget = new QWidget;
    auto *gradingLay = new QVBoxLayout(gradingWidget);
    gradingLay->setContentsMargins(0, 0, 0, 0);
    gradingLay->setSpacing(16);

    auto *stuInfoCard = new QFrame;
    stuInfoCard->setObjectName("homeworkInfoCard");
    auto *stuInfoLay = new QVBoxLayout(stuInfoCard);
    stuInfoLay->setContentsMargins(24, 24, 24, 24);
    stuInfoLay->setSpacing(8);

    m_gradingStudentName = new QLabel("学生：—");
    m_gradingStudentName->setObjectName("homeworkDetailTitle");
    stuInfoLay->addWidget(m_gradingStudentName);

    m_gradingFileName = new QLabel("文件：—");
    m_gradingFileName->setObjectName("infoLabel");
    stuInfoLay->addWidget(m_gradingFileName);

    m_downloadFileBtn = new QPushButton("📥 下载/查看文件");
    m_downloadFileBtn->setObjectName("downloadFileBtn");
    m_downloadFileBtn->setVisible(false);
    connect(m_downloadFileBtn, &QPushButton::clicked, this, [=]() {
        if (!m_api || m_currentFilePath.isEmpty()) return;
        QString defaultName = QFileInfo(m_currentFilePath).fileName();
        QString savePath = QFileDialog::getSaveFileName(this,
            "保存文件", QDir::homePath() + "/" + defaultName);
        if (savePath.isEmpty()) return;  // 用户取消
        m_downloadFileBtn->setText("⏳ 下载中...");
        m_downloadFileBtn->setEnabled(false);
        m_api->downloadFile(m_currentFilePath, savePath);
        // 用 SingleShot 防止多次点击累积连接
        QMetaObject::Connection conn;
        conn = connect(m_api, &ApiClient::fileDownloaded, this,
            [this, conn](const QString &localPath, bool ok, const QString &errorMsg) {
                disconnect(conn);
                if (!m_downloadFileBtn) return;
                m_downloadFileBtn->setText("📥 下载/查看文件");
                m_downloadFileBtn->setEnabled(true);
                if (ok && !localPath.isEmpty()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(localPath));
                } else {
                    QMessageBox::warning(this, "下载失败",
                        "文件下载失败：" + errorMsg);
                }
            });
    });
    stuInfoLay->addWidget(m_downloadFileBtn);

    m_gradingContent = new QLabel("");
    m_gradingContent->setObjectName("codeBlock");
    m_gradingContent->setWordWrap(true);
    m_gradingContent->setTextInteractionFlags(Qt::TextSelectableByMouse);
    stuInfoLay->addWidget(m_gradingContent);

    gradingLay->addWidget(stuInfoCard);

    auto *scoreCard = new QFrame;
    scoreCard->setObjectName("homeworkInfoCard");
    auto *scoreLay = new QVBoxLayout(scoreCard);
    scoreLay->setContentsMargins(24, 24, 24, 24);
    scoreLay->setSpacing(12);

    auto *scoreTitle = new QLabel("评分");
    scoreTitle->setObjectName("sectionTitle");
    scoreLay->addWidget(scoreTitle);

    m_gradingScore = new QSpinBox;
    m_gradingScore->setRange(0, 100);
    m_gradingScore->setValue(0);
    m_gradingScore->setObjectName("scoreSpinBox");
    scoreLay->addWidget(m_gradingScore);

    auto *commentTitle = new QLabel("评语");
    commentTitle->setObjectName("sectionTitle");
    scoreLay->addWidget(commentTitle);

    m_gradingComment = new QTextEdit;
    m_gradingComment->setPlaceholderText("请输入评语...");
    m_gradingComment->setObjectName("commentEdit");
    m_gradingComment->setMinimumHeight(80);
    scoreLay->addWidget(m_gradingComment);

    auto *submitScoreBtn = new QPushButton("提交评分");
    submitScoreBtn->setObjectName("primaryBtn");
    connect(submitScoreBtn, &QPushButton::clicked, this, [=]() {
        if (!m_api || m_currentSubId < 0) return;
        int score = m_gradingScore->value();
        QString comment = m_gradingComment->toPlainText();
        m_api->gradeSubmission(m_currentSubId, score, comment,
            [this](bool ok, const QJsonDocument &, const QString &msg) {
                if (ok) {
                    showToast("评分提交成功");
                    QTimer::singleShot(800, this, [=]() {
                        if (m_currentGradingHwId > 0)
                            loadSubmissionsForGrading(m_currentGradingHwId);
                    });
                } else {
                    showToast(msg);
                }
            });
    });
    scoreLay->addWidget(submitScoreBtn);
    gradingLay->addWidget(scoreCard);
    gradingLay->addStretch();

    rightPanel->setWidget(gradingWidget);
    content->addWidget(rightPanel, 1);

    mainLay->addLayout(content, 1);

    // 作业选择弹出菜单
    auto *selectMenu = new QMenu(hwSelectBtn);
    connect(hwSelectBtn, &QPushButton::clicked, this, [=]() {
        selectMenu->clear();
        for (const auto &hw : m_homeworks) {
            QJsonObject h = hw.toObject();
            auto *act = selectMenu->addAction(h["title"].toString());
            connect(act, &QAction::triggered, this, [=]() {
                hwSelectBtn->setText(h["title"].toString());
                m_currentGradingHwId = h["id"].toInt();
                m_gradingTitle->setText("批改：" + h["title"].toString());
                loadSubmissionsForGrading(h["id"].toInt());
            });
        }
        selectMenu->exec(hwSelectBtn->mapToGlobal(QPoint(0, hwSelectBtn->height())));
    });

    return wrapper;
}

// ==================== 空状态 ====================

QWidget* teacherpanel::createEmptyState(const QString &msg)
{
    auto *w = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setAlignment(Qt::AlignCenter);
    auto *label = new QLabel(msg);
    label->setObjectName("emptyStateText");
    label->setAlignment(Qt::AlignCenter);
    lay->addWidget(label);
    return w;
}

// ==================== 发布/编辑弹窗 ====================

QDialog* teacherpanel::createPublishDialog()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("发布作业");
    dlg->setFixedSize(420, 450);
    dlg->setObjectName("publishDialog");

    auto *lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(16);

    m_dialogTitleLabel = new QLabel("发布新作业");
    m_dialogTitleLabel->setObjectName("dialogTitle");
    lay->addWidget(m_dialogTitleLabel);

    auto *titleLabel = new QLabel("作业标题");
    titleLabel->setObjectName("fieldLabel");
    lay->addWidget(titleLabel);

    m_hwTitleInput = new QLineEdit;
    m_hwTitleInput->setPlaceholderText("请输入作业标题");
    m_hwTitleInput->setObjectName("dialogInput");
    lay->addWidget(m_hwTitleInput);

    auto *descLabel = new QLabel("作业描述");
    descLabel->setObjectName("fieldLabel");
    lay->addWidget(descLabel);

    m_hwDescInput = new QTextEdit;
    m_hwDescInput->setPlaceholderText("请输入作业要求...");
    m_hwDescInput->setObjectName("dialogTextEdit");
    m_hwDescInput->setMinimumHeight(100);
    lay->addWidget(m_hwDescInput);

    auto *deadlineLabel = new QLabel("截止时间");
    deadlineLabel->setObjectName("fieldLabel");
    lay->addWidget(deadlineLabel);

    m_hwDeadlineInput = new QLineEdit;
    m_hwDeadlineInput->setPlaceholderText("格式：2026-07-15 23:59");
    m_hwDeadlineInput->setObjectName("dialogInput");
    lay->addWidget(m_hwDeadlineInput);

    lay->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setObjectName("cancelBtn");
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    m_dialogSubmitBtn = new QPushButton("发布作业");
    m_dialogSubmitBtn->setObjectName("primaryBtn");
    connect(m_dialogSubmitBtn, &QPushButton::clicked, this, [=]() {
        if (!m_api) return;
        QString title = m_hwTitleInput->text().trimmed();
        if (title.isEmpty()) {
            QMessageBox::warning(dlg, "提示", "标题不能为空");
            return;
        }
        QJsonObject data;
        data["title"] = title;
        data["description"] = m_hwDescInput->toPlainText();
        data["deadline"] = m_hwDeadlineInput->text();

        if (m_isEditMode && m_editHomeworkId > 0) {
            m_api->updateHomework(m_editHomeworkId, data,
                [this, dlg](bool ok, const QJsonDocument &, const QString &msg) {
                    if (ok) {
                        dlg->accept();
                        loadHomeworks();
                        showToast("作业更新成功");
                    } else {
                        QMessageBox::warning(dlg, "错误", msg);
                    }
                });
        } else {
            m_api->createHomework(data,
                [this, dlg](bool ok, const QJsonDocument &, const QString &msg) {
                    if (ok) {
                        dlg->accept();
                        loadHomeworks();
                        showToast("作业发布成功");
                    } else {
                        QMessageBox::warning(dlg, "错误", msg);
                    }
                });
        }
    });
    btnRow->addWidget(m_dialogSubmitBtn);
    lay->addLayout(btnRow);

    return dlg;
}

// ==================== 学生提交条目 ====================

QFrame* teacherpanel::createStudentItem(int subId, const QString &name,
                                         const QString &status, bool graded, bool active)
{
    auto *item = new QFrame;
    item->setObjectName("studentItem");
    if (active) item->setProperty("active", true);
    item->setCursor(Qt::PointingHandCursor);
    item->setFixedHeight(56);
    item->setProperty("submissionId", subId);
    item->installEventFilter(this);

    auto *lay = new QHBoxLayout(item);
    lay->setContentsMargins(16, 0, 16, 0);

    auto *nameLabel = new QLabel(name);
    nameLabel->setObjectName("studentName");
    lay->addWidget(nameLabel);
    lay->addStretch();

    auto *scoreTag = new QLabel(status);
    scoreTag->setObjectName(graded ? "tagGraded" : "tagPending");
    lay->addWidget(scoreTag);

    return item;
}

// ==================== 页面切换 ====================

void teacherpanel::switchPage(int index)
{
    m_stacked->setCurrentIndex(index);
    updateSidebarActive(index);
}

void teacherpanel::updateSidebarActive(int index)
{
    for (int i = 0; i < m_sidebarBtns.size(); ++i)
        m_sidebarBtns[i]->setChecked(i == index);
}

// ==================== 学生选中 ====================

void teacherpanel::selectStudent(QFrame *item)
{
    if (m_selectedStudentItem) {
        m_selectedStudentItem->setProperty("active", false);
        m_selectedStudentItem->style()->unpolish(m_selectedStudentItem);
        m_selectedStudentItem->style()->polish(m_selectedStudentItem);
    }
    m_selectedStudentItem = item;
    if (item) {
        item->setProperty("active", true);
        item->style()->unpolish(item);
        item->style()->polish(item);
    }
}

// ==================== 数据加载 ====================

void teacherpanel::loadHomeworks()
{
    if (!m_api) return;
    m_api->getHomeworks([this](bool ok, const QJsonDocument &doc, const QString &) {
        if (ok) {
            m_homeworks = doc.array();
            populateHomeworkTable();
        }
    });
}

void teacherpanel::populateHomeworkTable()
{
    m_hwTable->setRowCount(0);

    if (m_homeworks.isEmpty()) {
        m_hwTable->setRowCount(1);
        auto *emptyItem = new QTableWidgetItem("暂无作业");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setFlags(Qt::NoItemFlags);
        m_hwTable->setItem(0, 0, emptyItem);
        return;
    }

    m_hwTable->setRowCount(m_homeworks.size());
    for (int i = 0; i < m_homeworks.size(); ++i) {
        QJsonObject hw = m_homeworks[i].toObject();

        auto *nameItem = new QTableWidgetItem(hw["title"].toString());
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_hwTable->setItem(i, 0, nameItem);

        QString dl = hw["deadline"].toString();
        auto *dlItem = new QTableWidgetItem(
            dl.isEmpty() ? "无限制" : QDateTime::fromString(dl, Qt::ISODate).toString("MM-dd hh:mm"));
        dlItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_hwTable->setItem(i, 1, dlItem);

        auto *countItem = new QTableWidgetItem("加载中...");
        countItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_hwTable->setItem(i, 2, countItem);

        m_api->getSubmissions(hw["id"].toInt(),
            [this, i](bool ok, const QJsonDocument &doc, const QString &) {
                if (ok) {
                    QJsonArray subs = doc.array();
                    auto *item = m_hwTable->item(i, 2);
                    if (item) item->setText(QString::number(subs.size()));
                }
            });

        auto *opsBtn = new QPushButton("...");
        opsBtn->setObjectName("opsBtn");
        opsBtn->setFixedWidth(60);

        int hwId = hw["id"].toInt();
        auto *menu = new QMenu(opsBtn);

        auto *gradeAct = menu->addAction("批改");
        connect(gradeAct, &QAction::triggered, this, [=]() {
            m_currentGradingHwId = hwId;
            m_gradingTitle->setText("批改：" + hw["title"].toString());
            switchPage(1);
            loadSubmissionsForGrading(hwId);
        });

        auto *editAct = menu->addAction("编辑");
        connect(editAct, &QAction::triggered, this, [=]() {
            m_isEditMode = true;
            m_editHomeworkId = hwId;
            m_dialogTitleLabel->setText("编辑作业");
            m_dialogSubmitBtn->setText("保存修改");
            m_hwTitleInput->setText(hw["title"].toString());
            m_hwDescInput->setText(hw["description"].toString());
            QString deadline = hw["deadline"].toString();
            m_hwDeadlineInput->setText(
                deadline.isEmpty() ? "" : QDateTime::fromString(deadline, Qt::ISODate).toString("yyyy-MM-dd hh:mm"));
            m_publishDialog->exec();
        });

        auto *delAct = menu->addAction("删除");
        connect(delAct, &QAction::triggered, this, [=]() {
            auto result = QMessageBox::question(this, "确认删除",
                "确定要删除作业\"" + hw["title"].toString() + "\"吗？\n此操作不可恢复。",
                QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes) {
                m_api->deleteHomework(hwId,
                    [this](bool ok, const QJsonDocument &, const QString &msg) {
                        if (ok) {
                            showToast("删除成功");
                            loadHomeworks();
                        } else {
                            QMessageBox::warning(this, "错误", msg);
                        }
                    });
            }
        });

        opsBtn->setMenu(menu);
        m_hwTable->setCellWidget(i, 3, opsBtn);
    }
}

void teacherpanel::loadSubmissionsForGrading(int homeworkId)
{
    if (!m_api) return;
    m_api->getSubmissions(homeworkId,
        [this](bool ok, const QJsonDocument &doc, const QString &) {
            m_currentSubmissions = ok ? doc.array() : QJsonArray();
            populateStudentList();
        });
}

void teacherpanel::populateStudentList()
{
    for (auto *item : m_studentItems) item->deleteLater();
    m_studentItems.clear();
    m_selectedStudentItem = nullptr;

    if (m_currentSubmissions.isEmpty() || !m_studentListLayout) {
        m_gradingStudentName->setText("学生：—");
        m_gradingFileName->setText("文件：—");
        if (m_downloadFileBtn) m_downloadFileBtn->setVisible(false);
        m_currentFilePath.clear();
        m_gradingContent->setText("");
        m_gradingScore->setValue(0);
        m_gradingComment->clear();
        return;
    }

    // 清空布局中的旧项
    QLayoutItem *item;
    while ((item = m_studentListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    for (const auto &sub : m_currentSubmissions) {
        QJsonObject s = sub.toObject();
        int subId = s["id"].toInt();
        QString name = s["student_name"].toString();
        bool graded = !s["score"].isNull() && s["score"].toInt(-1) >= 0;
        QString status = graded ? (QString::number(s["score"].toInt()) + "分 已批改") : "待批改";

        auto *itemWidget = createStudentItem(subId, name, status, graded);
        m_studentItems.append(itemWidget);
        m_studentListLayout->addWidget(itemWidget);
    }
    m_studentListLayout->addStretch();

    if (!m_studentItems.isEmpty()) {
        selectStudent(m_studentItems.first());
        showSubmissionDetail(m_currentSubmissions[0].toObject()["id"].toInt());
    }
}

void teacherpanel::showSubmissionDetail(int submissionId)
{
    if (!m_api) return;
    m_currentSubId = submissionId;

    m_api->getSubmission(submissionId,
        [this](bool ok, const QJsonDocument &doc, const QString &) {
            if (!ok || !m_gradingStudentName) return;
            QJsonObject s = doc.object();
            m_gradingStudentName->setText("学生：" + s["student_name"].toString());

            // 文件信息：有文件路径则显示下载按钮
            QString filePath = s["file_path"].toString();
            QString fileName = s["file_name"].toString();
            if (!filePath.isEmpty() && !fileName.isEmpty()) {
                if (m_gradingFileName) m_gradingFileName->setText("文件：" + fileName);
                m_currentFilePath = filePath;
                m_downloadFileBtn->setVisible(true);
            } else {
                if (m_gradingFileName) m_gradingFileName->setText("文件：（无附件）");
                m_currentFilePath.clear();
                m_downloadFileBtn->setVisible(false);
            }

            if (m_gradingContent) m_gradingContent->setText(s["content"].toString());
            int sc = s["score"].toInt(-1);
            if (m_gradingScore) m_gradingScore->setValue(sc >= 0 ? sc : 0);
            if (m_gradingComment) m_gradingComment->setPlainText(s["comment"].toString());
        });
}

// ==================== Toast ====================

void teacherpanel::showToast(const QString &msg)
{
    if (!m_toast) {
        m_toast = new QLabel(msg, this);
        m_toast->setObjectName("toast");
        m_toast->setAlignment(Qt::AlignCenter);
    } else {
        m_toast->setText(msg);
    }
    m_toast->setFixedSize(300, 44);
    m_toast->move((width() - m_toast->width()) / 2, height() - 80);
    m_toast->show();
    m_toast->raise();

    auto *anim = new QPropertyAnimation(m_toast, "windowOpacity");
    anim->setDuration(2000);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    connect(anim, &QPropertyAnimation::finished, m_toast, &QLabel::hide);
}
