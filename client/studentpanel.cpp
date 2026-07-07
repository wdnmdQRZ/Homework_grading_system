#include "studentpanel.h"
#include "src/core/apiclient.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QEvent>

studentpanel::studentpanel(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

studentpanel::~studentpanel() {}

void studentpanel::setApiClient(ApiClient *api)
{
    m_api = api;
    loadHomeworks();
    loadMySubmissions();
}

void studentpanel::setUsername(const QString &username)
{
    m_topbarUsername->setText(username);
}

// ==================== Event Filter ====================

bool studentpanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame *card = qobject_cast<QFrame*>(obj);
        if (card && card->property("clickable").toBool()) {
            int hid = card->property("homeworkId").toInt();
            if (hid > 0) showHomeworkDetail(hid);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// ==================== 整体布局 ====================

void studentpanel::setupUi()
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
    m_stacked->addWidget(createPageHomeworkList());    // 0
    m_stacked->addWidget(createPageDetail());          // 1
    m_stacked->addWidget(createPageMySubmissions());   // 2
    body->addWidget(m_stacked);

    mainLayout->addLayout(body, 1);
    switchPage(0);
}

// ==================== 顶部栏 ====================

QWidget* studentpanel::createTopbar()
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

    m_topbarUsername = new QLabel("学生");
    m_topbarUsername->setObjectName("topbarUsername");

    auto *roleTag = new QLabel("学生");
    roleTag->setObjectName("topbarRoleStudent");

    auto *logoutBtn = new QPushButton("退出登录");
    logoutBtn->setObjectName("topbarLogoutBtn");
    connect(logoutBtn, &QPushButton::clicked, this, &studentpanel::logoutRequested);

    lay->addWidget(title);
    lay->addWidget(spacer);
    lay->addWidget(m_topbarUsername);
    lay->addWidget(roleTag);
    lay->addWidget(logoutBtn);
    return bar;
}

// ==================== 侧边栏 ====================

QWidget* studentpanel::createSidebar()
{
    auto *sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(200);
    auto *lay = new QVBoxLayout(sidebar);
    lay->setContentsMargins(0, 16, 0, 0);
    lay->setSpacing(0);

    struct BtnDef { QString icon; QString text; int page; };
    QVector<BtnDef> defs = {
        {"📚", "作业列表", 0},
        {"📝", "我的提交", 2},
    };

    for (const auto &d : defs) {
        auto *btn = new QPushButton(d.icon + "  " + d.text);
        btn->setObjectName("sidebarBtn");
        btn->setCheckable(true);
        connect(btn, &QPushButton::clicked, this, [=]() { switchPage(d.page); });
        lay->addWidget(btn);
        m_sidebarBtns.append(btn);
    }
    lay->addStretch();
    return sidebar;
}

// ==================== 页面 0：作业列表 ====================

QWidget* studentpanel::createPageHomeworkList()
{
    auto *wrapper = new QWidget;
    auto *wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);

    auto *header = new QLabel("作业列表");
    header->setObjectName("pageHeader");
    wrapperLayout->addWidget(header);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setObjectName("homeworkScroll");

    auto *container = new QWidget;
    m_homeworkListLayout = new QVBoxLayout(container);
    m_homeworkListLayout->setContentsMargins(24, 0, 24, 24);
    m_homeworkListLayout->setSpacing(12);
    m_homeworkListLayout->addStretch();

    scrollArea->setWidget(container);
    wrapperLayout->addWidget(scrollArea, 1);
    return wrapper;
}

// ==================== 页面 1：作业详情 ====================

QWidget* studentpanel::createPageDetail()
{
    auto *page = new QWidget;
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(24, 0, 24, 24);
    lay->setSpacing(16);

    auto *backBtn = new QPushButton("← 返回作业列表");
    backBtn->setObjectName("backBtn");
    connect(backBtn, &QPushButton::clicked, this, [=]() { switchPage(0); });
    lay->addWidget(backBtn);

    auto *card = new QFrame;
    card->setObjectName("homeworkInfoCard");
    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(24, 24, 24, 24);
    cardLay->setSpacing(12);

    m_detailTitle = new QLabel("");
    m_detailTitle->setObjectName("homeworkDetailTitle");
    m_detailDesc = new QLabel("");
    m_detailDesc->setObjectName("homeworkDetailDesc");
    m_detailDesc->setWordWrap(true);

    auto *infoRow = new QHBoxLayout;
    m_detailTeacher = new QLabel("");
    m_detailTeacher->setObjectName("infoLabel");
    m_detailDeadline = new QLabel("");
    m_detailDeadline->setObjectName("infoLabel");
    infoRow->addWidget(m_detailTeacher);
    infoRow->addWidget(m_detailDeadline);
    infoRow->addStretch();

    m_detailStatus = new QLabel("");
    m_detailStatus->setObjectName("infoLabel");

    cardLay->addWidget(m_detailTitle);
    cardLay->addWidget(m_detailDesc);
    cardLay->addLayout(infoRow);
    cardLay->addWidget(m_detailStatus);
    lay->addWidget(card);

    // 提交区域
    m_submitArea = new QWidget;
    auto *submitLay = new QVBoxLayout(m_submitArea);
    submitLay->setContentsMargins(0, 0, 0, 0);
    submitLay->setSpacing(12);

    auto *submitTitle = new QLabel("提交作业");
    submitTitle->setObjectName("sectionTitle");
    submitLay->addWidget(submitTitle);

    // 文字输入区域
    auto *contentLabel = new QLabel("文字内容（可选，与文件二选一或同时提交）");
    contentLabel->setObjectName("uploadNote");
    submitLay->addWidget(contentLabel);

    m_contentEdit = new QTextEdit;
    m_contentEdit->setPlaceholderText("在此输入文字内容，如代码、答案、笔记等...");
    m_contentEdit->setObjectName("contentEdit");
    m_contentEdit->setMinimumHeight(100);
    m_contentEdit->setMaximumHeight(200);
    submitLay->addWidget(m_contentEdit);

    auto *orLabel = new QLabel("—— 或上传文件 ——");
    orLabel->setObjectName("uploadNote");
    orLabel->setAlignment(Qt::AlignCenter);
    submitLay->addWidget(orLabel);

    auto *uploadBtn = new QPushButton("📎 点击选择文件");
    uploadBtn->setObjectName("uploadBtn");
    connect(uploadBtn, &QPushButton::clicked, this, [=]() {
        QString fp = QFileDialog::getOpenFileName(this, "选择作业文件");
        if (!fp.isEmpty()) {
            m_selectedFilePath = fp;
            QFileInfo fi(fp);
            uploadBtn->setText("📎 " + fi.fileName());
        }
    });
    submitLay->addWidget(uploadBtn);

    auto *noteLabel = new QLabel("支持提交源代码、文档、图片等文件");
    noteLabel->setObjectName("uploadNote");
    submitLay->addWidget(noteLabel);

    auto *submitBtn = new QPushButton("提交作业");
    submitBtn->setObjectName("primaryBtn");
    connect(submitBtn, &QPushButton::clicked, this, [=]() {
        if (!m_api || m_currentHomeworkId < 0) return;
        QString textContent = m_contentEdit->toPlainText().trimmed();
        if (m_selectedFilePath.isEmpty() && textContent.isEmpty()) {
            showToast("请输入文字内容或选择文件");
            return;
        }
        m_api->submitHomework(m_currentHomeworkId, m_selectedFilePath, textContent,
            [this](bool ok, const QJsonDocument &, const QString &msg) {
                if (ok) {
                    showToast("作业提交成功");
                    QTimer::singleShot(800, this, [=]() {
                        loadHomeworks();
                        switchPage(0);
                    });
                } else {
                    showToast(msg);
                }
            });
    });
    submitLay->addWidget(submitBtn);
    lay->addWidget(m_submitArea);

    // 已提交区域
    m_submittedArea = new QWidget;
    m_submittedArea->setVisible(false);
    auto *submittedLay = new QVBoxLayout(m_submittedArea);
    submittedLay->setContentsMargins(0, 0, 0, 0);
    submittedLay->setSpacing(8);

    auto *submittedTitle = new QLabel("我的提交");
    submittedTitle->setObjectName("sectionTitle");
    submittedLay->addWidget(submittedTitle);

    m_submittedFileName = new QLabel("");
    m_submittedFileName->setObjectName("infoLabel");
    submittedLay->addWidget(m_submittedFileName);

    m_submittedTime = new QLabel("");
    m_submittedTime->setObjectName("infoLabel");
    submittedLay->addWidget(m_submittedTime);

    m_submittedScore = new QLabel("");
    m_submittedScore->setObjectName("scoreLabel");
    submittedLay->addWidget(m_submittedScore);

    m_submittedComment = new QLabel("");
    m_submittedComment->setObjectName("infoLabel");
    m_submittedComment->setWordWrap(true);
    submittedLay->addWidget(m_submittedComment);

    lay->addWidget(m_submittedArea);
    lay->addStretch();

    scroll->setWidget(content);
    outer->addWidget(scroll);
    return page;
}

// ==================== 页面 2：我的提交 ====================

QWidget* studentpanel::createPageMySubmissions()
{
    auto *page = new QWidget;
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget;
    auto *lay = new QVBoxLayout(content);
    lay->setContentsMargins(24, 0, 24, 24);
    lay->setSpacing(16);

    auto *header = new QLabel("我的提交");
    header->setObjectName("pageHeader");
    lay->addWidget(header);

    m_submissionsTable = new QTableWidget;
    m_submissionsTable->setColumnCount(4);
    m_submissionsTable->setHorizontalHeaderLabels({"作业名称", "提交时间", "评分", "状态"});
    m_submissionsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_submissionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_submissionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_submissionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_submissionsTable->verticalHeader()->setVisible(false);
    m_submissionsTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_submissionsTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_submissionsTable->setObjectName("submissionsTable");
    m_submissionsTable->setMinimumHeight(200);
    lay->addWidget(m_submissionsTable);

    m_submissionDetailArea = new QWidget;
    m_submissionDetailArea->setVisible(false);
    auto *detailLay = new QVBoxLayout(m_submissionDetailArea);
    detailLay->setContentsMargins(0, 0, 0, 0);
    detailLay->setSpacing(8);

    auto *detailTitle = new QLabel("提交详情");
    detailTitle->setObjectName("sectionTitle");
    detailLay->addWidget(detailTitle);

    auto *hwLabel = new QLabel("");
    hwLabel->setObjectName("infoLabel");
    detailLay->addWidget(hwLabel);

    auto *fileLabel = new QLabel("");
    fileLabel->setObjectName("infoLabel");
    detailLay->addWidget(fileLabel);

    auto *scoreLabel = new QLabel("");
    scoreLabel->setObjectName("scoreLabel");
    detailLay->addWidget(scoreLabel);

    auto *commentLabel = new QLabel("");
    commentLabel->setObjectName("infoLabel");
    commentLabel->setWordWrap(true);
    detailLay->addWidget(commentLabel);

    lay->addWidget(m_submissionDetailArea);
    lay->addStretch();

    connect(m_submissionsTable, &QTableWidget::itemSelectionChanged, this, [=]() {
        auto *item = m_submissionsTable->currentItem();
        if (!item) return;
        int row = item->row();
        if (row >= 0 && row < m_submissions.size()) {
            QJsonObject sub = m_submissions[row].toObject();
            hwLabel->setText("作业：" + QString::number(sub["homework_id"].toInt()));
            fileLabel->setText("文件：" + sub["file_name"].toString());
            int sc = sub["score"].toInt(-1);
            scoreLabel->setText(sc >= 0 ? QString::number(sc) + " / 100" : "未批改");
            commentLabel->setText("评语：" + sub["comment"].toString());
            m_submissionDetailArea->setVisible(true);
        }
    });

    scroll->setWidget(content);
    outer->addWidget(scroll);
    return page;
}

// ==================== 空状态 ====================

QWidget* studentpanel::createEmptyState(const QString &msg)
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

// ==================== 作业卡片 ====================

QFrame* studentpanel::createHomeworkCard(int homeworkId, const QString &title,
                                          const QString &desc, const QString &teacher,
                                          const QString &deadline, const QString &status,
                                          bool expired)
{
    auto *card = new QFrame;
    card->setObjectName("homeworkCard");
    card->setCursor(Qt::PointingHandCursor);
    card->setProperty("clickable", true);
    card->setProperty("homeworkId", homeworkId);
    card->installEventFilter(this);
    if (expired) card->setProperty("expired", true);

    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(6);

    auto *topRow = new QHBoxLayout;
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("homeworkCardTitle");
    topRow->addWidget(titleLabel);
    topRow->addStretch();

    if (!status.isEmpty()) {
        auto *tag = new QLabel(status);
        if (status == "已提交") tag->setObjectName("tagSubmitted");
        else if (status == "已批改") tag->setObjectName("tagGraded");
        else tag->setObjectName("tagUnsubmitted");
        topRow->addWidget(tag);
    }
    lay->addLayout(topRow);

    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("homeworkCardDesc");
    lay->addWidget(descLabel);

    auto *bottomRow = new QHBoxLayout;
    auto *teacherLabel = new QLabel("教师：" + teacher + "  |  截止：" + deadline);
    teacherLabel->setObjectName("homeworkCardMeta");
    bottomRow->addWidget(teacherLabel);
    bottomRow->addStretch();

    auto *detailBtn = new QPushButton("查看详情 →");
    detailBtn->setObjectName("detailBtn");
    connect(detailBtn, &QPushButton::clicked, this, [=]() {
        showHomeworkDetail(homeworkId);
    });
    bottomRow->addWidget(detailBtn);
    lay->addLayout(bottomRow);

    return card;
}

// ==================== 数据加载 ====================

void studentpanel::loadHomeworks()
{
    if (!m_api) return;
    m_api->getHomeworks([this](bool ok, const QJsonDocument &doc, const QString &) {
        if (ok) {
            m_homeworks = doc.array();
            populateHomeworkList();
        }
    });
}

void studentpanel::populateHomeworkList()
{
    QLayoutItem *item;
    while ((item = m_homeworkListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_cardHomeworkIds.clear();

    if (m_homeworks.isEmpty()) {
        m_homeworkListLayout->addWidget(createEmptyState("暂无作业"));
        m_homeworkListLayout->addStretch();
        return;
    }

    m_api->getSubmissions(-1, [this](bool ok, const QJsonDocument &doc, const QString &) {
        QJsonArray subs;
        if (ok) subs = doc.array();

        QMap<int, QJsonObject> subMap;
        for (const auto &s : subs) {
            QJsonObject obj = s.toObject();
            subMap[obj["homework_id"].toInt()] = obj;
        }

        QLayoutItem *item;
        while ((item = m_homeworkListLayout->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        m_cardHomeworkIds.clear();

        for (const auto &hw : m_homeworks) {
            QJsonObject h = hw.toObject();
            int hid = h["id"].toInt();
            QString title = h["title"].toString();
            QString desc = h["description"].toString();
            QString teacher = h["teacher_name"].toString();
            QString dlStr = h["deadline"].toString();
            QString deadline = dlStr.isEmpty() ? "无限制"
                : QDateTime::fromString(dlStr, Qt::ISODate).toString("yyyy-MM-dd hh:mm");

            bool expired = false;
            if (!dlStr.isEmpty()) {
                expired = QDateTime::fromString(dlStr, Qt::ISODate) < QDateTime::currentDateTime();
            }

            QString status;
            if (subMap.contains(hid)) {
                QJsonObject sub = subMap[hid];
                status = sub["score"].isNull() ? "已提交" : "已批改";
            } else {
                status = "未提交";
            }

            auto *card = createHomeworkCard(hid, title, desc, teacher, deadline, status, expired);
            m_homeworkListLayout->addWidget(card);
            m_cardHomeworkIds.append(hid);
        }
        m_homeworkListLayout->addStretch();
    });
}

void studentpanel::showHomeworkDetail(int homeworkId)
{
    if (!m_api) return;
    m_currentHomeworkId = homeworkId;

    m_api->getHomework(homeworkId, [this](bool ok, const QJsonDocument &doc, const QString &) {
        if (!ok) return;
        QJsonObject h = doc.object();

        m_detailTitle->setText(h["title"].toString());
        m_detailDesc->setText(h["description"].toString());
        m_detailTeacher->setText("教师：" + h["teacher_name"].toString());
        QString dl = h["deadline"].toString();
        m_detailDeadline->setText("截止时间：" + (dl.isEmpty() ? "无限制"
            : QDateTime::fromString(dl, Qt::ISODate).toString("yyyy-MM-dd hh:mm")));

        m_api->getSubmissions(h["id"].toInt(),
            [this](bool ok, const QJsonDocument &doc, const QString &) {
                QJsonArray subs = ok ? doc.array() : QJsonArray();

                if (!subs.isEmpty()) {
                    QJsonObject sub = subs[0].toObject();
                    m_submitArea->setVisible(false);
                    m_submittedArea->setVisible(true);
                    m_submittedFileName->setText("提交文件：" + sub["file_name"].toString());
                    m_submittedTime->setText("提交时间："
                        + QDateTime::fromString(sub["submitted_at"].toString(), Qt::ISODate).toString("yyyy-MM-dd hh:mm"));
                    int sc = sub["score"].toInt(-1);
                    if (sc >= 0) {
                        m_submittedScore->setText(QString::number(sc) + " / 100");
                        m_submittedScore->setObjectName("scoreLabelGraded");
                    } else {
                        m_submittedScore->setText("待批改");
                        m_submittedScore->setObjectName("scoreLabelPending");
                    }
                    m_submittedScore->style()->unpolish(m_submittedScore);
                    m_submittedScore->style()->polish(m_submittedScore);
                    m_submittedComment->setText("评语：" + sub["comment"].toString());
                } else {
                    m_submitArea->setVisible(true);
                    m_submittedArea->setVisible(false);
                    m_selectedFilePath.clear();
                    if (m_contentEdit) m_contentEdit->clear();
                }
        });
    });

    switchPage(1);
}

void studentpanel::loadMySubmissions()
{
    if (!m_api) return;
    m_api->getSubmissions(-1, [this](bool ok, const QJsonDocument &doc, const QString &) {
        if (ok) {
            m_submissions = doc.array();
            populateMySubmissions();
        }
    });
}

void studentpanel::populateMySubmissions()
{
    m_submissionsTable->setRowCount(0);
    m_submissionDetailArea->setVisible(false);

    if (m_submissions.isEmpty()) {
        m_submissionsTable->setRowCount(1);
        auto *emptyItem = new QTableWidgetItem("暂无提交记录");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setFlags(Qt::NoItemFlags);
        m_submissionsTable->setItem(0, 0, emptyItem);
        return;
    }

    m_submissionsTable->setRowCount(m_submissions.size());
    for (int i = 0; i < m_submissions.size(); ++i) {
        QJsonObject sub = m_submissions[i].toObject();
        auto *nameItem = new QTableWidgetItem(QString::number(sub["homework_id"].toInt()));
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_submissionsTable->setItem(i, 0, nameItem);

        QString time = QDateTime::fromString(sub["submitted_at"].toString(), Qt::ISODate).toString("MM-dd hh:mm");
        auto *timeItem = new QTableWidgetItem(time);
        timeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_submissionsTable->setItem(i, 1, timeItem);

        int sc = sub["score"].toInt(-1);
        auto *scoreItem = new QTableWidgetItem(sc >= 0 ? QString::number(sc) : "—");
        scoreItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_submissionsTable->setItem(i, 2, scoreItem);

        auto *statusItem = new QTableWidgetItem(sc >= 0 ? "已批改" : "待批改");
        statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_submissionsTable->setItem(i, 3, statusItem);
    }
}

// ==================== 页面切换 ====================

void studentpanel::switchPage(int index)
{
    if (index == 2) loadMySubmissions();
    m_stacked->setCurrentIndex(index);
    updateSidebarActive(index);
}

void studentpanel::updateSidebarActive(int index)
{
    for (int i = 0; i < m_sidebarBtns.size(); ++i)
        m_sidebarBtns[i]->setChecked(i == index);
}

// ==================== Toast ====================

void studentpanel::showToast(const QString &msg)
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
