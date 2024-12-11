from .main import main_bp
from .routes import routes_bp
from .trips import trips_bp

# Expose the blueprints to be easily imported
__all__ = ['main_bp', 'routes_bp', 'trips_bp']