from datetime import datetime
from extensions import db


class Homework(db.Model):
    """作业表"""
    __tablename__ = 'homeworks'

    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200), nullable=False)
    description = db.Column(db.Text, nullable=True)
    teacher_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    deadline = db.Column(db.DateTime, nullable=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    # 关联关系
    teacher = db.relationship('User', backref='published_homeworks')
    submissions = db.relationship('Submission', backref='homework', lazy='dynamic')

    def to_dict(self):
        return {
            "id": self.id,
            "title": self.title,
            "description": self.description,
            "teacher_id": self.teacher_id,
            "teacher_name": self.teacher.username if self.teacher else None,
            "deadline": self.deadline.isoformat() if self.deadline else None,
            "created_at": self.created_at.isoformat() if self.created_at else None,
        }
