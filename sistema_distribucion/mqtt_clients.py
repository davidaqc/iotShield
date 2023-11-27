"""
This code configures and handles MQTT connections, manages incoming messages, and publish messages.
"""
import time
import logging
import subprocess
import paho.mqtt.client as mqtt
from config import BROKER_IP, BROKER_PORT, USERNAME, PASSWORD, GREEN, RED, YELLOW, ENDC
from email_handler import configure_email_message, send_email

# Initialization of variables and constants
email_msg = configure_email_message()
responses = {}
respuestas_ataque = []
KILL_MOSQUITTO_COMMAND = 'taskkill /F /IM mosquitto.exe'
MAX_ATTACK_RESPONSES = 10
MAX_RESPONSE_TIME = 10
RECONNECT_DELAY = 5
# Configure logging
logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s] - %(message)s')
logger = logging.getLogger(__name__)

# Establishes an MQTT connection to the broker.
def connect_mqtt(client):
    while True:
        try:
            client.connect(BROKER_IP, BROKER_PORT, 5)
            break  # Exit the loop if the connection is established successfully
        except ConnectionRefusedError as e:
            logger.error(f"{RED}Cannot connect to the MQTT server. Retrying in 5 seconds...{ENDC}")
            time.sleep(RECONNECT_DELAY)  # Wait for 5 seconds before trying again

# Configure an MQTT client with the specified client name and subscribe topic
def configure_mqtt_client(device_name, subscribe_topic):
    client = mqtt.Client(client_id=device_name, clean_session=True)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = lambda client, userdata, flags, rc: on_connect(client, userdata, flags, rc, subscribe_topic)
    client.on_message = on_message_device_handler
    client.on_disconnect = on_disconnect

    connect_mqtt(client)  # Initial connection

    return client

# Handle incoming messages from MQTT devices
def on_message_device_handler(client, userdata, message):
    response = message.payload.decode('utf-8')  # Decodificar los bytes a una cadena

    client_name = client._client_id.decode()  # Nombre del cliente

    # Almacena la respuesta en el diccionario con el nombre del cliente como clave
    responses[client_name] = response

    # Verificar si la respuesta es "ataque" y agregarla a la lista de respuestas
    if response == 'ataque':
        respuestas_ataque.append('ataque')
    else:
        respuestas_ataque.append('legitimate')

    # Mantener solo las últimas X respuestas en la lista
    if len(respuestas_ataque) > MAX_ATTACK_RESPONSES:
        respuestas_ataque.pop(0)

    # Verificar si las últimas X respuestas son todas "ataque"
    if len(respuestas_ataque) == MAX_ATTACK_RESPONSES and all(resp == 'ataque' for resp in respuestas_ataque):
        logger.warning(f"{YELLOW}¡Las últimas X respuestas son todas de ataque!{ENDC}")
        send_email(email_msg)
        # Terminar el proceso "mosquitto.exe" de manera forzada.
        try:
            subprocess.run(KILL_MOSQUITTO_COMMAND, shell=True, check=True)
            logger.warning(f"{YELLOW}The MQTT server has been stopped due to a detected attack.{ENDC}")
        except subprocess.CalledProcessError as e:
            logger.error(f"{RED}An error occurred while terminating the process: {e}{ENDC}")

# Handle the interaction with MQTT devices
def device_handler(q1, device, topic):
    while True:
        # Recibir un dato de la cola
        data = q1.get()
        
        client_name = device._client_id.decode()  # Nombre del cliente, opcional
        logger.info(f"Enviando dato al dispositivo {client_name}: {data}")

        # Enviar el dato al dispositivo
        device.publish(topic, data)

        # Esperar a que el dispositivo responda
        start_time = time.time()
        response_attribute = device._client_id.decode()  # Nombre del cliente
        response = None

        while response is None:
            if time.time() - start_time > MAX_RESPONSE_TIME and device.is_connected():
                # Si han pasado más de X segundos sin respuesta, volver a enviar el mismo dato
                logger.warning(f"{YELLOW}Reenviando dato al dispositivo {client_name}: {data}{ENDC}")
                device.publish(topic, data)
                start_time = time.time()
            else:
                # Verificar si hay una respuesta asociada al dispositivo
                if response_attribute in responses:
                    response = responses[response_attribute]

        logger.info(f"Respuesta del dispositivo {client_name}: {response}")

        # Eliminar la respuesta del diccionario
        del responses[response_attribute]

        # Eliminar el elemento de la cola después de procesarlo con éxito
        q1.task_done()

        # Imprimir la cantidad de datos restantes en la cola
        remaining_data = q1.qsize()
        logger.info(f"Datos restantes en la cola: {remaining_data}")

# Handle MQTT connection status
def on_connect(client, userdata, flags, rc, subscribe_topic):
    if rc == 0:
        logger.info(f"{GREEN}Cliente {client._client_id.decode()} conectado al broker MQTT con resultado, rc: {rc}{ENDC}")
        client.subscribe(subscribe_topic)
    else:
        logger.warning(f"{YELLOW}Cliente {client._client_id.decode()} no se pudo conectar al broker MQTT, rc: {rc}{ENDC}")

# Callback for MQTT client disconnection
def on_disconnect(client, userdata, rc):
    if rc != 0:
        logger.error(f"{RED}Unexpected MQTT disconnection. Reconnecting...{ENDC}")
        connect_mqtt(client)  # Reconnect when an unexpected disconnection occurs
