import queue
import threading
from capture_data import capture_and_process_data
from mqtt_clients import configure_mqtt_client, device_handler
from config import IA_TOPIC_DATOS_1, IA_TOPIC_DATOS_2, IA_TOPIC_CLASSIFICATION_1, IA_TOPIC_CLASSIFICATION_2

# Funcion principal
def main():

    # Creación de la cola FIFO compartida
    q1 = queue.Queue()

    devices = {
        "device_1": {
            "publish": IA_TOPIC_DATOS_1,
            "subscribe": IA_TOPIC_CLASSIFICATION_1
        },
        "device_2": {
            "publish": IA_TOPIC_DATOS_2,
            "subscribe": IA_TOPIC_CLASSIFICATION_2
        },
        # Agrega más dispositivos aquí si es necesario
    }

    threads = []

    # Configura y arranca los hilos de generación de datos y manejo de dispositivos
    for device_name, topics in devices.items():
        device = configure_mqtt_client(device_name, topics["subscribe"])
        device_thread = threading.Thread(target=device_handler, args=(q1, device, topics["publish"]))
        device_thread.start()
        threads.append(device_thread)
        devices[device_name]["client"] = device  # Almacena la instancia del cliente MQTT

    # Arranca el hilo de generación de datos
    data_generator_thread = threading.Thread(target=capture_and_process_data, args=(q1,))
    data_generator_thread.start()
    threads.append(data_generator_thread)

    # Arranca el bucle de MQTT para recibir mensajes de todos los dispositivos
    for device_name, device_data in devices.items():
        device = device_data["client"]
        device.loop_start()

    # Espera a que todos los hilos terminen
    for thread in threads:
        thread.join()


if __name__ == "__main__":
    main()
    