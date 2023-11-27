"""
This code is to configure and send an email alert.
"""
import smtplib
import logging
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from config import EMAIL_FROM, EMAIL_TO, EMAIL_PASSWORD, EMAIL_SUBJECT, EMAIL_MESSAGE, GREEN, RED, ENDC

# Configure logging
logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s] - %(message)s')
logger = logging.getLogger(__name__)

# Configuración para enviar correo electrónico
def configure_email_message():
    msg = MIMEMultipart()
    msg['From'] = EMAIL_FROM
    msg['To'] = EMAIL_TO
    msg['Subject'] = EMAIL_SUBJECT
    msg.attach(MIMEText(EMAIL_MESSAGE, 'plain'))
    return msg

# Función para enviar correo electrónico
def send_email(msg):
    try:
        # Iniciar una conexión SMTP y enviar el correo electrónico
        server = smtplib.SMTP('smtp.gmail.com: 587')
        server.starttls()
        server.login(msg['From'], EMAIL_PASSWORD)
        server.sendmail(msg['From'], msg['To'], msg.as_string())
        server.quit()
        logger.info(f"{GREEN}Correo electrónico de alerta enviado con éxito.{ENDC}")
    except Exception as e:
        logger.error(f"{RED}Error al enviar el correo electrónico: {e}{ENDC}")
