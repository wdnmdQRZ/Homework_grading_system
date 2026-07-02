from datetime import datetime
from extensions import db


class Submission(db.Model):
    """作业提交表"""
    __tablename__ = 'submissions'

    id = db.Column(db.Integer, primary_key=True)
    homework_id = db.Column(db.Integer, db.ForeignKey('homeworks.id'), nullable=False)
    student_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    file_path = db.Column(db.String(500), nullable=True)
    file_name = db.Column(db.String(200), nullable=True)
    content = db.Column(db.Text, nullable=True)
    score = db.Column(db.Integer, nullable=True)
    comment = db.Column(db.Text, nullable=True)
    submitted_at = db.Column(db.DateTime, default=datetime.utcnow)
    graded_at = db.Column(db.DateTime, nullable=True)

    # 关联关系
    student = db.relationship('User', backref='submissions')

    def to_dict(self):
        return {
            "id": self.id,
            "homework_id": self.homework_id,
            "student_id": self.student_id,
            "student_name": self.student.username if self.student else None,
            "file_path": self.file_path,
            "file_name": self.file_name,
            "content": self.content,
            "score": self.score,
            "comment": self.comment,
            "submitted_at": self.submitted_at.isoformat() if self.submitted_at else None,
            "graded_at": self.graded_at.isoformat() if self.graded_at else None,
        }
