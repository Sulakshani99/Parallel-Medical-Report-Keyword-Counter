import csv

input_csv = "data/Original_Dataset.csv"
output_txt = "data/medical_reports.txt"

with open(input_csv, 'r', encoding='utf-8') as csv_file, open(output_txt, 'w', encoding='utf-8') as txt_file:
    reader = csv.DictReader(csv_file)
    for row in reader:
        line = f"{row['Disease']} {row['Symptom_1']} {row['Symptom_2']} {row['Symptom_3']} {row['Symptom_4']} {row['Symptom_5']}"
        line = line.lower()
        txt_file.write(line.strip() + "\n")

print("Converted CSV to medical_reports.txt")
