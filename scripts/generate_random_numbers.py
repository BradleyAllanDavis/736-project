import random
import sys


output_file_name = sys.argv[1]
num_keys = int(sys.argv[2])

with open(output_file_name, 'w') as out_file:
    final = random.sample(range(10000000), num_keys)
    for key in final:
        out_file.write(str(key) + '\n')
