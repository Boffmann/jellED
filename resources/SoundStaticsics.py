import struct

filePath = "../jellED/data/test_music_buffer.txt"
samples = []

with open(filePath, 'rb') as f:
    byte = f.read(1)
    while byte:
        samples.append(int(int.from_bytes(byte, "little")))
        byte = f.read(1)

mean = sum(samples) / len(samples) 
variance = sum([((x - mean) ** 2) for x in samples]) / len(samples) 
res = variance ** 0.5
 
# Printing result 

print("Standard max of sample is : " + str(max(samples)))
print("Standard mean of sample is : " + str(mean))
print("Standard variance of sample is : " + str(variance)) 
print("Standard deviation of sample is : " + str(res)) 