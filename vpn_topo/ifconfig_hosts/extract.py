import re


def extract_info(filename):
    with open(filename, "r") as file:
        content = file.read()
        ip = re.search(r"inet (\d+\.\d+\.\d+\.\d+)", content).group(1)
        mac = re.search(r"ether ([\da-fA-F:]+)", content).group(1)
        return ip, mac


host_data = {}
for i in range(1, 8):  # Assuming h1 to h7
    try:
        ip, mac = extract_info(f"h{i}.txt")
        host_data[f"h{i}"] = {"ip": ip, "mac": mac}
    except FileNotFoundError:
        print(f"File for host h{i} not found. Skipping.")
    except AttributeError:
        print(f"Data extraction failed for h{i}. Check the file format.")

# Writing to a C header file 'host_info.h'
header_file_content = "#ifndef HOST_INFO_H\n#define HOST_INFO_H\n\n"
for host, data in host_data.items():
    header_file_content += f"#define {host.upper()}_IP \"{data['ip']}\"\n"
    header_file_content += f"#define {host.upper()}_MAC \"{data['mac']}\"\n"
header_file_content += "\n#endif // HOST_INFO_H\n"

# Save the header file
header_file_path = "../../src/host_info.h"
with open(header_file_path, "w") as file:
    file.write(header_file_content)
