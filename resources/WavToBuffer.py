import wave

BUFFER_SIZE = 64
START_AFTER = 128 # Do not include first x frames from sample (skip e.g. metadata etc)
TOTAL_OUTPUT_BYTES = 50000
TOTAL_LINES = TOTAL_OUTPUT_BYTES / BUFFER_SIZE
OUTPUT_PATH = '../jellED/data/test_music_buffer.txt'

music_file = wave.open('30secBeat.wav')

print("Sample width: " + str(music_file.getsampwidth()) + " channels: " + str(music_file.getnchannels()) + " sample frequency: " + str(music_file.getframerate()))
#frames_to_read = int(BUFFER_SIZE / (music_file.getsampwidth() + music_file.getnchannels()))
frames_to_read = 1

line_count = 0

with open(OUTPUT_PATH, 'wb') as outfile:
    while True:
        for i in range(START_AFTER):
            music_file.readframes(frames_to_read)
        frames = music_file.readframes(frames_to_read)
        if not frames or line_count > TOTAL_LINES:
            break
        line_count+=1
        outfile.write(frames)
        # + str.encode('\n')