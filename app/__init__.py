# libraries
from flask import Flask
import os
import logging

from .models import db

from .routes import main_bp, routes_bp, trips_bp
from .utils import dp_bp, generator_bp

def create_app():
    app = Flask(__name__)

    app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///app.db'
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
    app.config['SECRET_KEY'] = 'your_secret_key_here'  # Use a random secret key for sessions
    # connecting the app to the database from models/extensions.py
    db.init_app(app)

    # Set up logging
    logging.basicConfig(level=logging.DEBUG)

    # initializing app context and creating the tables of the database
    with app.app_context():
      db.create_all()

    # Register Blueprints
    app.register_blueprint(main_bp)
    app.register_blueprint(routes_bp)
    app.register_blueprint(trips_bp)
    app.register_blueprint(dp_bp)
    app.register_blueprint(generator_bp)

    return app