"""
This code is to centralize configuration settings and constants.
"""
from decouple import config

# MQTT Configuration
BROKER_IP = "192.168.50.169"
BROKER_PORT = 1884
USERNAME = "user1_david"
PASSWORD = config("MQTT_PASSWORD")

# Email Configuration
EMAIL_FROM = "davidqc05tests@gmail.com"
EMAIL_TO = "davidqc05@gmail.com"
EMAIL_PASSWORD = config("EMAIL_PASSWORD")
EMAIL_SUBJECT = "¡Alerta de ataque!"
EMAIL_MESSAGE = "Se ha detectado tráfico inusual en la red. Por favor, tome medidas inmediatas para investigar y abordar esta situación."

# MQTT Topics
IA_TOPIC_DATOS_1 = "ia/rpi_1/datos"
IA_TOPIC_DATOS_2 = "ia/rpi_2/datos"
IA_TOPIC_CLASSIFICATION_1 = "ia/rpi_1/clasificacion"
IA_TOPIC_CLASSIFICATION_2 = "ia/rpi_2/clasificacion"

# ANSI Escape Codes for Colored Text
YELLOW = "\033[93m"
GREEN = "\033[92m"
RED = "\033[91m"
ENDC = "\033[0m"    # Reset to default color
