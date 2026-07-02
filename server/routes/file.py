import os
from flask import Blueprint, send_from_directory, current_app
from flask_jwt_extended import jwt_required
from common.response import success, fail

file_bp = Blueprint("file", __name__)


@file_bp.route("/api/uploads/<path:filepath>", methods=["GET"])
@jwt_required()
def download_file(filepath):
    """下载上传的文件"""
    upload_folder = current_app.config["UPLOAD_FOLDER"]
    # 安全检查：防止路径穿越
    target = os.path.normpath(os.path.join(upload_folder, filepath))
    if not target.startswith(os.path.normpath(upload_folder)):
        return fail(403, "禁止访问")

    directory = os.path.dirname(target)
    filename = os.path.basename(target)

    if not os.path.isfile(target):
        return fail(404, "文件不存在")

    return send_from_directory(directory, filename, as_attachment=True)
