import os
from datetime import datetime
from flask import Blueprint, request, current_app
from flask_jwt_extended import get_jwt_identity, jwt_required
from werkzeug.utils import secure_filename
from extensions import db
from models.user import User
from models.homework import Homework
from models.submission import Submission
from common.response import success, fail

submission_bp = Blueprint("submission", __name__)


def _get_current_user():
    uid = int(get_jwt_identity())
    return User.query.get(uid)


@submission_bp.route("/api/submissions", methods=["GET"])
@jwt_required()
def list_submissions():
    """查看提交列表
    学生：查看自己的提交
    老师：查看某次作业的所有提交（需传 homework_id 参数）
    """
    user = _get_current_user()
    if not user:
        return fail(401, "用户不存在")

    homework_id = request.args.get("homework_id", type=int)

    if user.role == "teacher":
        if not homework_id:
            return fail(400, "教师查看提交需指定 homework_id")
        hw = Homework.query.get(homework_id)
        if not hw or hw.teacher_id != user.id:
            return fail(403, "无权查看此作业的提交")
        items = Submission.query.filter_by(homework_id=homework_id).order_by(Submission.submitted_at.desc()).all()
    else:
        query = Submission.query.filter_by(student_id=user.id)
        if homework_id:
            query = query.filter_by(homework_id=homework_id)
        items = query.order_by(Submission.submitted_at.desc()).all()

    return success([s.to_dict() for s in items])


@submission_bp.route("/api/submissions/<int:sid>", methods=["GET"])
@jwt_required()
def get_submission(sid):
    """查看单个提交详情"""
    user = _get_current_user()
    if not user:
        return fail(401, "用户不存在")

    s = Submission.query.get(sid)
    if not s:
        return fail(404, "提交不存在")

    if user.role == "student":
        if s.student_id != user.id:
            return fail(403, "无权查看此提交")
    elif user.role == "teacher":
        hw = Homework.query.get(s.homework_id)
        if not hw or hw.teacher_id != user.id:
            return fail(403, "无权查看此提交")

    return success(s.to_dict())


@submission_bp.route("/api/submissions", methods=["POST"])
@jwt_required()
def create_submission():
    """学生提交作业（文件或文本内容）"""
    user = _get_current_user()
    if not user or user.role != "student":
        return fail(403, "仅学生可提交作业")

    # 提取 homework_id（支持表单和 JSON）
    homework_id = None
    if request.is_json:
        homework_id = request.json.get("homework_id")
        if homework_id is not None:
            try:
                homework_id = int(homework_id)
            except (ValueError, TypeError):
                return fail(400, "homework_id 必须是整数")
    elif request.form:
        homework_id = request.form.get("homework_id", type=int)

    if not homework_id:
        return fail(400, "请指定 homework_id")

    hw = Homework.query.get(homework_id)
    if not hw:
        return fail(404, "作业不存在")

    # 检查是否已提交
    existing = Submission.query.filter_by(homework_id=homework_id, student_id=user.id).first()
    if existing:
        return fail(400, "你已提交过此作业，请勿重复提交")

    content = None
    file_path = None
    file_name = None

    # 文件上传
    if "file" in request.files:
        f = request.files["file"]
        if f.filename:
            file_name = secure_filename(f.filename) or "upload"
            # 按 homework_id 和 student_id 分目录保存
            upload_dir = os.path.join(current_app.config["UPLOAD_FOLDER"], f"hw_{homework_id}", f"stu_{user.id}")
            os.makedirs(upload_dir, exist_ok=True)
            file_path_rel = os.path.join(upload_dir, file_name)
            f.save(file_path_rel)
            # 存储相对路径
            file_path = os.path.relpath(file_path_rel, current_app.config["UPLOAD_FOLDER"])

    # 文本内容
    if request.is_json:
        content = request.json.get("content", "")

    if not content and not file_path:
        return fail(400, "请上传文件或填写内容")

    sub = Submission(
        homework_id=homework_id,
        student_id=user.id,
        file_path=file_path,
        file_name=file_name,
        content=content,
    )
    db.session.add(sub)
    db.session.commit()

    return success(sub.to_dict(), "提交成功")


@submission_bp.route("/api/submissions/<int:sid>/grade", methods=["PUT"])
@jwt_required()
def grade_submission(sid):
    """老师批改评分"""
    user = _get_current_user()
    if not user or user.role != "teacher":
        return fail(403, "仅教师可批改")

    sub = Submission.query.get(sid)
    if not sub:
        return fail(404, "提交不存在")

    # 验证是否是该老师布置的作业
    hw = Homework.query.get(sub.homework_id)
    if not hw or hw.teacher_id != user.id:
        return fail(403, "无权批改此提交")

    data = request.get_json(silent=True) or {}
    score = data.get("score")
    comment = data.get("comment", "")

    if score is None:
        return fail(400, "分数不能为空")
    if not isinstance(score, int) or score < 0 or score > 100:
        return fail(400, "分数必须是 0-100 的整数")

    sub.score = score
    sub.comment = comment
    sub.graded_at = datetime.utcnow()
    db.session.commit()

    return success(sub.to_dict(), "批改成功")
