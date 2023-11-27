"""
This code captures and processes data from the network and places it in a queue.
"""
import subprocess
import pandas as pd
import joblib

# Initialization of constants
SCALER = joblib.load('scaler.pkl')
FEATURE_COLUMNS = ['tcp.len', 'tcp.stream', "tcp.flags", "tcp.time_delta"]
TSHARK_COMMAND = 'tshark -i Wi-Fi -f "port 1883" -T fields -e tcp.len -e tcp.stream -e tcp.flags -e tcp.time_delta -l -E separator=,'
MIN_TIME_DELTA = 0.001
NEW_TIME_DELTA = 10

# Captura y procesa los datos de la red MQTT
def capture_and_process_data(q1):
    # Execute tshark in the background
    process = subprocess.Popen(TSHARK_COMMAND, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1, universal_newlines=True)

    # Process the captured data
    for line in process.stdout:
        fields = line.strip().split(',')

        # Conversion of hexadecimal fields (position 2)
        if fields[2].startswith("0x"):
            fields[2] = str(int(fields[2], 16))

        # Replace empty fields with "0"
        fields = [field if field else "0" for field in fields]

        data_temporal = pd.DataFrame([fields], columns=FEATURE_COLUMNS)

        # Apply modifications to tcp.time_delta
        data_temporal['tcp.time_delta'] = data_temporal['tcp.time_delta'].astype(float)
        data_temporal.loc[data_temporal['tcp.time_delta'] < MIN_TIME_DELTA, 'tcp.time_delta'] = NEW_TIME_DELTA

        new_data_standardized = SCALER.transform(data_temporal)

        new_data = ",".join(map(lambda x: str(round(x, 6)), new_data_standardized[0]))
        q1.put(new_data)

    process.wait()
