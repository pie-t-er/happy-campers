from .data_processing import dp_bp
from .route_generator import generator_bp

# Expose the blueprints to be easily imported
__all__ = ['dp_bp', 'generator_bp']