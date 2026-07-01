from flask import Blueprint
from common.response import success

hello_bp = Blueprint("hello", __name__)


@hello_bp.route("/api/hello")
def hello():
    return success({"greeting": "Hello, Homework Grading System!"})
