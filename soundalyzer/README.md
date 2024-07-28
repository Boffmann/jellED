# Read Audio from ESP32

Run command

```
pio run -e az-delivery-devkit-v4 -t monitor > soundinput.txt
```

Edit `soundinput.txt` and remove the top lines that do not contribute to the sound.
You can then read the input using the soundalyzer
