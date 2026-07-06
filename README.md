# Homework Grading System

**QT 客户端 + Flask 后端** 课设项目——在线作业提交与批改系统。

## 目录结构

```
E:\Homework_grading_system\
├── client/                          # QT 6 C++ 桌面客户端
│   ├── CMakeLists.txt               # CMake 构建配置（依赖 Qt6::Core/Widgets/Network）
│   ├── main.cpp                     # 程序入口
│   ├── mainwindow.cpp / .h / .ui    # 主窗口
│   ├── logindialog.cpp / .h / .ui   # 登录/注册对话框（QStackedWidget 容器）
│   ├── loginpage.cpp / .h / .ui     # 登录页
│   ├── registerpage.cpp / .h / .ui  # 注册页
│   ├── studentpanel.cpp / .h / .ui  # 学生面板
│   ├── teacherpanel.cpp / .h / .ui  # 教师面板
│   ├── style.qss                    # QT 样式表
│   └── src/
│       └── core/
│           ├── apiclient.h           # API 客户端封装（GET/POST/PUT/DELETE）
│           └── apiclient.cpp         # API 客户端实现
│
├── server/                          # Flask Python 后端
│   ├── app.py                       # 应用入口，工厂函数 create_app()
│   ├── config.py                    # 配置类（SQLite、JWT、上传路径等）
│   ├── extensions.py                # 第三方扩展实例（db, jwt）
│   ├── requirements.txt             # Python 依赖列表
│   ├── common/
│   │   ├── __init__.py
│   │   └── response.py              # 统一 JSON 响应（success / fail）
│   ├── models/
│   │   ├── __init__.py
│   │   ├── user.py                  # 用户模型（User 表）
│   │   ├── homework.py              # 作业模型（Homework 表）
│   │   └── submission.py            # 提交模型（Submission 表）
│   ├── routes/
│   │   ├── __init__.py
│   │   ├── hello.py                 # 测试路由 /api/hello
│   │   ├── auth.py                  # 认证路由（注册 / 登录）
│   │   ├── homework.py              # 作业 CRUD 路由
│   │   ├── submission.py            # 提交 / 批改路由
│   │   └── file.py                  # 文件下载路由
│   ├── services/                    # 业务逻辑层（待建）
│   ├── utils/                       # 工具函数（待建）
│   ├── uploads/                     # 作业文件上传目录
│   ├── tests/                       # 测试（待建）
│   └── venv/                        # Python 虚拟环境
│
├── .gitignore                       # Git 忽略规则
├── start_server.bat                 # 一键启动后端脚本
└── README.md                        # 本文件
```

---

## 快速启动

### 后端（Flask）

1. **激活虚拟环境并安装依赖**

```bash
cd server
venv\Scripts\activate
pip install -r requirements.txt
```

或直接双击 `start_server.bat`（自动激活 venv）。

2. **启动服务器**

```bash
python app.py
```

看到以下输出即启动成功：

```
 * Running on http://0.0.0.0:8080
```

3. **验证**

访问 http://localhost:8080/ 应返回：

```json
{"code": 200, "message": "HomeworkGrader Backend Running", "data": {"version": "1.0.0"}}
```

### 客户端（QT）

用 **Qt Creator** 打开 `client/CMakeLists.txt`，点锤子运行 CMake 后构建运行。

---

## API 接口文档

所有接口返回统一的 JSON 格式：

```json
// 成功
{"code": 200, "message": "success", "data": {...}}

// 失败
{"code": 4xx, "message": "错误描述", "data": null}
```

### 1. Hello（测试）

```
GET /api/hello
```

响应示例：

```json
{"code": 200, "message": "success", "data": {"greeting": "Hello, Homework Grading System!"}}
```

### 2. 注册

```
POST /api/auth/register
Content-Type: application/json

{
    "username": "student01",
    "password": "123456"
}
```

参数校验规则：
- 用户名不能为空，最长 80 字符
- 密码不能少于 6 位，不能包含空格
- 用户名不能重复

### 3. 登录

```
POST /api/auth/login
Content-Type: application/json

{
    "username": "student01",
    "password": "123456"
}
```

成功响应返回 JWT token 和用户信息：

```json
{
    "code": 200,
    "message": "登录成功",
    "data": {
        "token": "eyJhbGciOiJIUzI1NiIs...",
        "user": {"id": 1, "username": "student01", "role": "student"}
    }
}
```

后续请求在 Header 中携带 token：

```
Authorization: Bearer <token>
```

### 4. 作业列表

```
GET /api/homeworks
Authorization: Bearer <token>
```

老师查看自己发布的作业，学生查看全部作业。

### 5. 作业详情

```
GET /api/homeworks/<id>
Authorization: Bearer <token>
```

### 6. 发布作业（教师）

```
POST /api/homeworks
Authorization: Bearer <token>
Content-Type: application/json

{
    "title": "第一次作业",
    "description": "写一篇作文",
    "deadline": "2026-07-10T23:59:59"
}
```

### 7. 编辑作业（教师）

```
PUT /api/homeworks/<id>
Authorization: Bearer <token>
Content-Type: application/json

{"title": "修改后的标题"}
```

### 8. 删除作业（教师）

```
DELETE /api/homeworks/<id>
Authorization: Bearer <token>
```

会级联删除所有提交记录和磁盘文件。

### 9. 提交作业（学生）

```
POST /api/submissions
Authorization: Bearer <token>
Content-Type: application/json

{"homework_id": 1, "content": "我的答案"}
```

也支持文件上传（multipart/form-data，字段名 `file`）。

### 10. 提交列表

```
GET /api/submissions?homework_id=1
Authorization: Bearer <token>
```

学生查看自己的提交，教师查看指定作业的所有提交。

### 11. 提交详情

```
GET /api/submissions/<id>
Authorization: Bearer <token>
```

学生只能看自己的，教师只能看自己作业的。

### 12. 批改评分（教师）

```
PUT /api/submissions/<id>/grade
Authorization: Bearer <token>
Content-Type: application/json

{"score": 95, "comment": "写得很好"}
```

### 13. 文件下载

```
GET /api/uploads/<filepath>
Authorization: Bearer <token>
```

---

## 技术栈

| 层 | 技术 | 说明 |
|----|------|------|
| 客户端 | QT 6 (C++) | 跨平台桌面 GUI，模块：Core / Widgets / Network |
| 后端 | Flask 3.0 (Python) | Web 框架，工厂模式创建应用 |
| 数据库 | SQLite | 文件数据库，零配置 |
| ORM | Flask-SQLAlchemy | 数据库操作 |
| 认证 | Flask-JWT-Extended | JWT 用户认证 |
| 通信 | HTTP + JSON | 前后端通过 RESTful API 通信 |

---

## 数据库模型

### User（用户表）

| 字段 | 类型 | 说明 |
|------|------|------|
| id | Integer (PK) | 主键，自增 |
| username | String(80) | 用户名，唯一，非空 |
| password_hash | String(256) | 加密存储，不存明文 |
| role | String(20) | 角色：`student` / `teacher` |
| created_at | DateTime | 注册时间（UTC） |

### Homework（作业表）

| 字段 | 类型 | 说明 |
|------|------|------|
| id | Integer (PK) | 主键，自增 |
| title | String(200) | 作业标题，非空 |
| description | Text | 作业描述 |
| teacher_id | Integer (FK) | 关联 users.id |
| deadline | DateTime | 截止时间（可选） |
| created_at | DateTime | 发布时间（UTC） |

### Submission（提交表）

| 字段 | 类型 | 说明 |
|------|------|------|
| id | Integer (PK) | 主键，自增 |
| homework_id | Integer (FK) | 关联 homeworks.id |
| student_id | Integer (FK) | 关联 users.id |
| file_path | String(500) | 上传文件路径（可选） |
| file_name | String(200) | 原始文件名（可选） |
| content | Text | 文本内容（可选） |
| score | Integer | 分数 0-100（教师批改后填入） |
| comment | Text | 教师评语 |
| submitted_at | DateTime | 提交时间（UTC） |
| graded_at | DateTime | 批改时间（UTC） |

---

## 开发计划 / 待实现

- [x] 项目结构拆分（client/ + server/）
- [x] Flask 后端骨架（config、app 工厂）
- [x] 统一响应格式（response.py）
- [x] 用户模型（User）
- [x] 注册 / 登录接口
- [x] QT ApiClient 封装（HTTP 请求）
- [x] 登录/注册界面（QStackedWidget 分离页面）
- [x] 作业 CRUD 接口 + 模型
- [x] 提交作业功能（文本 + 文件上传）
- [x] 批改评分功能
- [x] 学生面板（作业列表、提交、查看成绩）
- [x] 教师面板（发布作业、查看提交、批改）