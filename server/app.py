from flask import Flask, jsonify
from flask_cors import CORS
from config import Config
from routes.hello import hello_bp

def create_app():
    """工厂函数创建 Flask 应用"""
    app = Flask(__name__)
    app.config.from_object(Config)

    # 启用 CORS（QT 客户端跨域调用）
    CORS(app, resources={r"/*": {"origins": "*"}})

    # 注册蓝图
    app.register_blueprint(hello_bp)

    @app.route("/")
    def index():
        return jsonify({
            "code": 200,
            "message": "HomeworkGrader Backend Running",
            "data": {"version": "1.0.0"}
        })

    return app

app = create_app()

if __name__ == "__main__":
    app.run(
        host=app.config["HOST"],
        port=app.config["PORT"],
        debug=app.config["DEBUG"]
    )
