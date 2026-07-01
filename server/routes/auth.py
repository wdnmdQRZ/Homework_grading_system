from flask import Blueprint, request
from flask_jwt_extended import create_access_token
from extensions import db
from models.user import User
from common.response import success, fail

auth_bp = Blueprint("auth", __name__)
@auth_bp.route("/api/auth/register", methods=["POST"])
def register():
    data = request.get_json()
    username = data.get("username", "").strip()
    password = data.get("password", "")

    # 1. 参数校验
    if not username or not password:
        return fail(400, "用户名和密码不能为空")
    if len(username) > 80:
        return fail(400, "用户名过长")
    if len(password) < 6:
        return fail(400, "密码不能少于6位")
    if " " in password:
        return fail(400, "密码不能包含空格")

    # 2. 查重
    if User.query.filter_by(username=username).first():
        return fail(400, "用户名已存在")

    # 3. 创建用户
    user = User(username=username, role="student")
    user.set_password(password)
    db.session.add(user)
    db.session.commit()

    return success({"id": user.id, "username": user.username}, "注册成功")

@auth_bp.route("/api/auth/login", methods=["POST"])
def login():
    data = request.get_json()
    username = data.get("username", "").strip()
    password = data.get("password", "")

    # 1. 参数校验
    if not username or not password:
        return fail(400, "用户名和密码不能为空")

    # 2. 查用户
    user = User.query.filter_by(username=username).first()
    if not user:
        return fail(401, "用户名或密码错误")

    # 3. 验密码
    if not user.check_password(password):
        return fail(401, "用户名或密码错误")

    # 4. 签发 JWT
    token = create_access_token(identity=user.id)

    return success({
        "token": token,
        "user": {
            "id": user.id,
            "username": user.username,
            "role": user.role
        }
    }, "登录成功")