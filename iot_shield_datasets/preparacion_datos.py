import pandas as pd

input_csv_path = 'Raw data/csv/flood_publish.csv'
output_csv_path = 'flood_publish.csv'

# Carga el CSV
df = pd.read_csv(input_csv_path, on_bad_lines='skip')

# Rellena los valores faltantes con ceros
df = df.fillna(0)

# Agrega la columna 'target' con el valor 'legitimate'
df['target'] = 'flood_publish'

# Convierte la columna 'tcp.flags' a decimal y reemplaza su contenido
df['tcp.flags'] = df['tcp.flags'].apply(lambda x: int(x, 16))

# Elimina filas duplicadas
df = df.drop_duplicates()

# Guarda el DataFrame con los nuevos cambios
df.to_csv(output_csv_path, index=False, header=True)

print("Datos procesados y guardados en el archivo:", output_csv_path)
