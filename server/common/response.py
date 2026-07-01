from flask import jsonify

def success(data=None, message="success"):
    """成功响应 - code 固定 200"""
    return jsonify({
        "code": 200,
        "message": message,
        "data": data
    })

def fail(code=400, message="error", data=None):
    """失败响应"""
    return jsonify({
        "code": code,
        "message": message,
        "data": data
    }), code
