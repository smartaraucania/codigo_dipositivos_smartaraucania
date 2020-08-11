import serial
import urllib3
import time

http = urllib3.PoolManager()
# microbit = serial.Serial('/dev/ttyACM0', baudrate=9600, timeout=1.0) #linux/mac
microbit = serial.Serial('COM9', baudrate=115200, timeout=3.0) #windows

while True:
    data = microbit.readline() # read a byte string
    dataDecode = data.decode() # decode byte string into Unicode 
    dataClean = dataDecode.rstrip()  # remove \n and \r
    dataSeparated = dataClean.split(",")
    # print(dataSeparated)

    #Serial data input format
    #[Color, Red, Green, Blue, Hex Color, Soil Moisture, Infrared]
    #['209', '57', '75', '73', '455B59', '1020', '1']
    
    bodyColor = "color|"+dataSeparated[0]+"|red|"+dataSeparated[1]+"|green|"+dataSeparated[2]+"|blue|"+dataSeparated[3]+"|hexColor|"+dataSeparated[4]
    bodySoilmoisture = "soilMoisture|"+dataSeparated[5]
    bodyInfrared =  "IRReception|"+dataSeparated[6]

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=color002%3ATCS34725&d='+ bodyColor, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(0.3)

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=soilmoisture002%3AU019&d='+ bodySoilmoisture, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(0.3)

    http.request('POST', 'https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=infrared001%3AIRM3638T&d='+ bodyInfrared, 
    headers={'Content-Type': 'text/plain'})
    time.sleep(2.4)

