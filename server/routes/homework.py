from datetime import datetime
import os
import shutil
from flask import Blueprint, request, current_app
from flask_jwt_extended import get_jwt_identity, jwt_required
from extensions import db
from models.user import User
from models.homework import Homework
from models.submission import Submission
from common.response import success, fail

homework_bp = Blueprint("homework", __name__)


def _get_current_user():
    """获取当前登录用户"""
    uid = int(get_jwt_identity())
    return User.query.get(uid)


@homework_bp.route("/api/homeworks", methods=["GET"])
@jwt_required()
def list_homeworks():
    """获取作业列表（学生看全部，老师看自己发布的）"""
    user = _get_current_user()
    if not user:
        return fail(401, "用户不存在")

    if user.role == "teacher":
        items = Homework.query.filter_by(teacher_id=user.id).order_by(Homework.created_at.desc()).all()
    else:
        items = Homework.query.order_by(Homework.created_at.desc()).all()

    return success([h.to_dict() for h in items])


@homework_bp.route("/api/homeworks/<int:hid>", methods=["GET"])
@jwt_required()
def get_homework(hid):
    """获取单个作业详情"""
    hw = Homework.query.get(hid)
    if not hw:
        return fail(404, "作业不存在")
    return success(hw.to_dict())


@homework_bp.route("/api/homeworks", methods=["POST"])
@jwt_required()
def create_homework():
    """老师发布作业"""
    user = _get_current_user()
    if not user or user.role != "teacher":
        return fail(403, "仅教师可发布作业")

    data = request.get_json(silent=True) or {}
    title = data.get("title", "").strip()
    description = data.get("description", "")
    deadline = data.get("deadline")  # ISO 格式字符串

    if not title:
        return fail(400, "标题不能为空")

    deadline_dt = None
    if deadline:
        try:
            deadline_dt = datetime.fromisoformat(deadline)
        except ValueError:
            return fail(400, "截止时间格式错误")

    hw = Homework(
        title=title,
        description=description,
        teacher_id=user.id,
        deadline=deadline_dt,
    )
    db.session.add(hw)
    db.session.commit()

    return success(hw.to_dict(), "发布成功")


@homework_bp.route("/api/homeworks/<int:hid>", methods=["PUT"])
@jwt_required()
def update_homework(hid):
    """老师编辑作业"""
    user = _get_current_user()
    if not user or user.role != "teacher":
        return fail(403, "仅教师可编辑作业")

    hw = Homework.query.get(hid)
    if not hw:
        return fail(404, "作业不存在")
    if hw.teacher_id != user.id:
        return fail(403, "无权编辑此作业")

    data = request.get_json(silent=True) or {}
    if "title" in data:
        hw.title = data["title"].strip()
    if "description" in data:
        hw.description = data["description"]
    if "deadline" in data:
        try:
            hw.deadline = datetime.fromisoformat(data["deadline"]) if data["deadline"] else None
        except ValueError:
            return fail(400, "截止时间格式错误")

    db.session.commit()
    return success(hw.to_dict(), "更新成功")


@homework_bp.route("/api/homeworks/<int:hid>", methods=["DELETE"])
@jwt_required()
def delete_homework(hid):
    """老师删除作业"""
    user = _get_current_user()
    if not user or user.role != "teacher":
        return fail(403, "仅教师可删除作业")

    hw = Homework.query.get(hid)
    if not hw:
        return fail(404, "作业不存在")
    if hw.teacher_id != user.id:
        return fail(403, "无权删除此作业")

    # 清理磁盘上的上传文件
    hw_upload_dir = os.path.join(current_app.config["UPLOAD_FOLDER"], f"hw_{hid}")
    if os.path.isdir(hw_upload_dir):
        shutil.rmtree(hw_upload_dir, ignore_errors=True)

    # 先删除该作业的所有提交记录
    Submission.query.filter_by(homework_id=hid).delete()
    db.session.delete(hw)
    db.session.commit()

    return success(None, "删除成功")
